#ifndef BEAM_WIN32_HOOKS_HPP
#define BEAM_WIN32_HOOKS_HPP
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <windows.h>
#include <winternl.h>
#include <ntstatus.h>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/SuspendedRoutineQueue.hpp"
#include "Beam/Threading/SpinMutex.hpp"

namespace Beam::Routines::Details {
  inline DWORD ToDwordTimeout(PLARGE_INTEGER timeout) {
    if(timeout->QuadPart >= 0) {
      return 0;
    }
    auto relative100ns = -timeout->QuadPart;
    auto msTimeout = static_cast<DWORD>(relative100ns / 10000);
    return msTimeout;
  }

  inline auto isHooking = false;

  inline std::size_t GetModRMEncodingSize(const std::uint8_t* address) {
    auto modrm = address[0];
    auto mod = modrm >> 6;
    auto rm = modrm & 0x07;
    auto size = std::size_t(1);
    if(mod == 0 && rm == 5) {
      size += 4;
    } else if(mod == 3) {
      return size;
    } else if(rm == 4) {
      size += 1;
      auto sib = address[1];
      auto base = sib & 0x07;
      if(mod == 0 && base == 5) {
        size += 4;
      }
    }
    if(mod == 1) {
      size += 1;
    } else if(mod == 2) {
      size += 4;
    }
    return size;
  }

  inline std::size_t GetInstructionSize(const std::uint8_t* address) {
    auto op = address[0];
    if(op == 0x0F) {
      auto secondOp = address[1];
      if(secondOp >= 0x80 && secondOp <= 0x8F) {
        return 6;
      } else if(secondOp == 0xB1) {
        return GetModRMEncodingSize(address + 2) + 2;
      } else {
        throw std::runtime_error("Unsupported two-byte opcode");
      }
    } else if(op == 0x2D) { // SUB EAX, imm32 (0x2D)
      return 5;
    } else if(op == 0x31) { // XOR edx, edx (commonly 31 D2)
      return 2;
    } else if(op == 0x33) {
      return GetModRMEncodingSize(address + 1) + 1;
    } else if(op >= 0x40 && op <= 0x4F) { // REX prefix (0x40-0x4F)
      return GetInstructionSize(address + 1) + 1;
    } else if(op >= 0x50 && op <= 0x57) {
      return 1;
    } else if(op >= 0x58 && op <= 0x5F) {
      return 1;
    } else if(op >= 0x70 && op <= 0x7F) {
      return 2;
    } else if(op == 0x80) {
      return GetModRMEncodingSize(address + 1) + 2;
    } else if(op == 0x83) {
      return GetModRMEncodingSize(address + 1) + 2;
    } else if(op == 0x85) {
      return GetModRMEncodingSize(address + 1) + 1;
    } else if(op == 0x8B) {
      return GetModRMEncodingSize(address + 1) + 1;
    } else if(op == 0x89) {
      return GetModRMEncodingSize(address + 1) + 1;
    } else if (op == 0x8D) {
      return GetModRMEncodingSize(address + 1) + 1;
    } else if(op == 0x90) { // NOP
      return 1;
    } else if(op == 0xA8) {
      return 2;
    } else if(op >= 0xB0 && op <= 0xB7) {
      return 2;
    } else if(op >= 0xB8 && op <= 0xBF) { // MOV reg, imm32 (0xB8-0xBF)
      return 5;
    } else if(op == 0xC3 || op == 0xCC) { // RET or INT 3
      return 1;
    } else if(op == 0xE9) { // JMP rel32
      return 5;
    } else if(op == 0xEB) { // JMP short rel8
      return 2;
    } else if(op == 0xF0) { // LOCK prefix
      return 1 + GetInstructionSize(address + 1);
    } else if(op == 0xF6) {
      auto modrm = address[1];
      auto regField = (modrm >> 3) & 0x07;
      if(regField != 0) {
        throw std::runtime_error("Unsupported F6 instruction variant.");
      }
      return GetModRMEncodingSize(address + 1) + 2;
    }
    throw std::runtime_error("Unrecognized instruction.");
  }

  struct SavedInstructions {
    static constexpr auto MAX_SIZE = 29;
    std::array<std::uint8_t, MAX_SIZE> m_instructions;
    std::size_t m_size;
  };

  template<typename F>
  SavedInstructions SaveInstructions(F target) {
    static const auto REDIRECT_SIZE = 14;
    auto instructions = SavedInstructions();
    auto f = reinterpret_cast<const std::uint8_t*>(target);
    auto accumulatedSize = std::size_t(0);
    while(accumulatedSize < REDIRECT_SIZE) {
      if(accumulatedSize >= SavedInstructions::MAX_SIZE) {
        throw std::runtime_error("Exceeded backup array size");
      }
      auto instructionSize = GetInstructionSize(f + accumulatedSize);
      if(accumulatedSize + instructionSize > SavedInstructions::MAX_SIZE) {
        throw std::runtime_error(
          "Instruction overshoots backup array capacity");
      }
      for(auto i = std::size_t(0); i < instructionSize; ++i) {
        instructions.m_instructions[accumulatedSize + i] =
          *(f + accumulatedSize + i);
      }
      accumulatedSize += instructionSize;
    }
    instructions.m_size = accumulatedSize;
    return instructions;
  }

  template<typename F>
  F MakeTrampoline(F target) {
    auto instructions = SaveInstructions(target);
    static const auto JUMP_SIZE = std::size_t(12);
    auto trampolineSize = instructions.m_size + JUMP_SIZE;
    auto trampoline = VirtualAlloc(nullptr, trampolineSize,
      MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if(!trampoline) {
      throw std::runtime_error("Unable to allocate memory for trampoline.");
    }
    std::memcpy(
      trampoline, instructions.m_instructions.data(), instructions.m_size);
    auto delta = reinterpret_cast<std::intptr_t>(target) -
      reinterpret_cast<std::intptr_t>(trampoline);
    auto offset = std::size_t(0);
    while(offset < instructions.m_size) {
      auto instruction = static_cast<std::uint8_t*>(trampoline) + offset;
      if(*instruction == 0x8B) {
        auto modrm = instruction[1];
        auto mod = modrm >> 6;
        auto rm = modrm & 0x07;
        if(mod == 0 && rm == 5) {
          if(delta >= std::numeric_limits<std::int32_t>::min() &&
              delta <= std::numeric_limits<std::int32_t>::max()) {
            auto displacement =
              reinterpret_cast<std::int32_t*>(instruction + 2);
            *displacement += static_cast<std::int32_t>(delta);
          } else {
            instruction[0] = 0xB8;

            /** TODO: Fix this hardcoded value. */
            *reinterpret_cast<std::uint32_t*>(instruction + 1) = 10240;
            instruction[5] = 0x90;
          }
        }
      }
      offset += GetInstructionSize(instruction);
    }
    auto jumpLocation =
      static_cast<std::uint8_t*>(trampoline) + instructions.m_size;
    auto jumpTarget =
      reinterpret_cast<std::uintptr_t>(target) + instructions.m_size;
    jumpLocation[0] = 0x48;
    jumpLocation[1] = 0xB8;
    *reinterpret_cast<std::uint64_t*>(jumpLocation + 2) = jumpTarget;
    jumpLocation[10] = 0xFF;
    jumpLocation[11] = 0xE0;
    return reinterpret_cast<F>(trampoline);
  }

  template<typename F>
  void PatchTarget(F target, F hook) {
    static constexpr auto PATCH_SIZE = std::size_t(12);
    auto targetBytes = reinterpret_cast<std::uint8_t*>(target);
    auto oldProtect = DWORD();
    if(!VirtualProtect(
        targetBytes, PATCH_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect)) {
      throw std::runtime_error("VirtualProtect failed");
    }
    targetBytes[0] = 0x48;
    targetBytes[1] = 0xB8;
    *reinterpret_cast<std::uint64_t*>(targetBytes + 2) =
      reinterpret_cast<std::uint64_t>(hook);
    targetBytes[10] = 0xFF;
    targetBytes[11] = 0xE0;
    FlushInstructionCache(GetCurrentProcess(), targetBytes, PATCH_SIZE);
    if(!VirtualProtect(targetBytes, PATCH_SIZE, oldProtect, &oldProtect)) {
      throw std::runtime_error("VirtualProtect restore failed");
    }
  }

  template <typename F>
  F Hook(std::string_view target_name, F hook, LPCWSTR module = L"ntdll.dll") {
    auto kernelModule = GetModuleHandleW(module);
    if(!kernelModule) {
      return nullptr;
    }
    auto target = reinterpret_cast<F>(reinterpret_cast<std::uint8_t*>(
      GetProcAddress(kernelModule, target_name.data())));
    if(!target) {
      return nullptr;
    }
    auto trampoline = MakeTrampoline(target);
    PatchTarget(target, hook);
    return trampoline;
  }

  template<typename T>
  struct WaitEntry {
    Threading::SpinMutex m_mutex;
    SuspendedRoutineQueue<T> m_suspendedRoutines;
  };

  inline auto waitEntries = std::array<WaitEntry<void*>, 256>();

  inline WaitEntry<void*>& GetWaitEntry(void* address) {
    return waitEntries[(reinterpret_cast<std::uintptr_t>(address) >> 4) %
      waitEntries.size()];
  }

  template<typename T>
  VOID CALLBACK OnTimerExpired(PVOID parameter, BOOLEAN timerOrWaitFired) {
    auto& waitEntry = *static_cast<WaitEntry<T>*>(parameter);
    auto lock = std::unique_lock(waitEntry.m_mutex);
    Resume(Store(waitEntry.m_suspendedRoutines));
  }

  inline auto NativeRtlWaitOnAddress = static_cast<NTSTATUS (NTAPI*)(
    _In_ void*, _In_ void*, _In_ size_t, _In_opt_ PLARGE_INTEGER)>(nullptr);

  inline NTSTATUS NTAPI HookedRtlWaitOnAddress(_In_ void* address,
      _In_ void* value, _In_ size_t size, _In_opt_ PLARGE_INTEGER timeout) {
    if(!Routine::IsInsideRoutine()) {
      return NativeRtlWaitOnAddress(address, value, size, timeout);
    }
    auto& waitEntry = GetWaitEntry(address);
    auto lock = std::unique_lock(waitEntry.m_mutex);
    if(std::memcmp(address, value, size) != 0) {
      return STATUS_SUCCESS;
    }
    if(!timeout || timeout->QuadPart == INFINITE) {
      auto isInsideRoutine = Routine::IsInsideRoutine();
      Routine::IsInsideRoutine() = false;
      Suspend(Store(waitEntry.m_suspendedRoutines), address, lock);
      Routine::IsInsideRoutine() = isInsideRoutine;
      return STATUS_SUCCESS;
    }
    auto timer = HANDLE();
    auto timeoutMs = ToDwordTimeout(timeout);
    auto timerResult = CreateTimerQueueTimer(&timer, nullptr,
      OnTimerExpired<void*>, &waitEntry, timeoutMs, 0, WT_EXECUTEONLYONCE);
    auto isInsideRoutine = Routine::IsInsideRoutine();
    Routine::IsInsideRoutine() = false;
    Suspend(Store(waitEntry.m_suspendedRoutines), address, lock);
    if(timer) {
      DeleteTimerQueueTimer(nullptr, timer, nullptr);
    }
    Routine::IsInsideRoutine() = isInsideRoutine;
    if(std::memcmp(address, value, size) != 0) {
      return STATUS_SUCCESS;
    }
    return STATUS_TIMEOUT;
  }

  inline auto NativeRtlWakeAddressSingle =
    static_cast<void (NTAPI*)(_In_ void*)>(nullptr);

  inline void NTAPI HookedRtlWakeAddressSingle(_In_ void* address) {
    NativeRtlWakeAddressSingle(address);
    auto& waitEntry = GetWaitEntry(address);
    auto lock = std::unique_lock(waitEntry.m_mutex);
    if(!waitEntry.m_suspendedRoutines.empty()) {
      ResumeFirstMatch(Store(waitEntry.m_suspendedRoutines), address, lock);
    }
  }

  inline auto NativeRtlWakeAddressAll =
    static_cast<void (NTAPI*)(_In_ void*)>(nullptr);

  inline void NTAPI HookedRtlWakeAddressAll(_In_ void* address) {
    NativeRtlWakeAddressAll(address);
    auto& waitEntry = GetWaitEntry(address);
    auto lock = std::unique_lock(waitEntry.m_mutex);
    if(!waitEntry.m_suspendedRoutines.empty()) {
      ResumeAllMatches(Store(waitEntry.m_suspendedRoutines), address, lock);
    }
  }

  inline auto NativeNtDelayExecution =
    static_cast<NTSTATUS (NTAPI*)(IN BOOLEAN, IN PLARGE_INTEGER)>(nullptr);

  inline NTSTATUS NTAPI HookedNtDelayExecution(
      IN BOOLEAN alertable, IN PLARGE_INTEGER timeout) {
    if(!Routine::IsInsideRoutine()) {
      return NativeNtDelayExecution(alertable, timeout);
    }
    auto isInsideRoutine = Routine::IsInsideRoutine();
    Routine::IsInsideRoutine() = false;
    if(!timeout || timeout->QuadPart == INFINITE) {
      auto waitEntry = WaitEntry<void>();
      auto lock = std::unique_lock(waitEntry.m_mutex);
      Suspend(Store(waitEntry.m_suspendedRoutines), lock);
    } else {
      auto timer = HANDLE();
      auto timeoutMs = ToDwordTimeout(timeout);
      auto waitEntry = WaitEntry<void>();
      {
        auto lock = std::unique_lock(waitEntry.m_mutex);
        auto timerResult = CreateTimerQueueTimer(&timer, nullptr,
          OnTimerExpired<void>, &waitEntry, timeoutMs, 0, WT_EXECUTEONLYONCE);
        if(!timer) {
          Routine::IsInsideRoutine() = isInsideRoutine;
          return timerResult;
        }
        Suspend(Store(waitEntry.m_suspendedRoutines), lock);
        if(timer) {
          DeleteTimerQueueTimer(nullptr, timer, nullptr);
        }
      }
    }
    Routine::IsInsideRoutine() = isInsideRoutine;
    return 0;
  }

  inline auto RtlTryAcquireSRWLockExclusive =
    static_cast<BOOLEAN (NTAPI*)(_Inout_ PRTL_SRWLOCK)>(nullptr);

  inline auto NativeRtlAcquireSRWLockExclusive =
    static_cast<void (NTAPI*)(_Inout_ PRTL_SRWLOCK)>(nullptr);

  inline void NTAPI HookedRtlAcquireSRWLockExclusive(
      _Inout_ PRTL_SRWLOCK lock) {
    if(isHooking) {
      return NativeRtlAcquireSRWLockExclusive(lock);
    }
    auto& flag = *reinterpret_cast<std::atomic_uintptr_t*>(&lock->Ptr);
    auto expected = flag.load();
    while(!RtlTryAcquireSRWLockExclusive(lock)) {
      HookedRtlWaitOnAddress(&flag, &expected, sizeof(expected), nullptr);
      expected = flag.load();
    }
  }

  inline auto NativeRtlReleaseSRWLockExclusive =
    static_cast<void (NTAPI*)(_Inout_ PRTL_SRWLOCK)>(nullptr);

  inline void NTAPI HookedRtlReleaseSRWLockExclusive(
      _Inout_ PRTL_SRWLOCK lock) {
    if(isHooking) {
      return NativeRtlReleaseSRWLockExclusive(lock);
    }
    NativeRtlReleaseSRWLockExclusive(lock);
    auto& flag = *reinterpret_cast<std::atomic_uintptr_t*>(&lock->Ptr);
    HookedRtlWakeAddressAll(&flag);
  }

  inline auto NativeRtlSleepConditionVariableSRW =
    static_cast<NTSTATUS (NTAPI*)(_Inout_ PRTL_CONDITION_VARIABLE,
      _Inout_ PRTL_SRWLOCK, _In_opt_ PLARGE_INTEGER, _In_ ULONG)>(nullptr);

  inline NTSTATUS WINAPI HookedRtlSleepConditionVariableSRW(
      _Inout_ PRTL_CONDITION_VARIABLE variable, _Inout_ PRTL_SRWLOCK lock,
      _In_opt_ PLARGE_INTEGER timeout, _In_ ULONG flags) {
    auto value =
      reinterpret_cast<std::atomic_uintptr_t*>(&variable->Ptr)->load();
    if(flags & RTL_CONDITION_VARIABLE_LOCKMODE_SHARED) {
      ReleaseSRWLockShared(lock);
    } else {
      ReleaseSRWLockExclusive(lock);
    }
    auto status =
      HookedRtlWaitOnAddress(&variable->Ptr, &value, sizeof(value), timeout);
    if(flags & RTL_CONDITION_VARIABLE_LOCKMODE_SHARED) {
      AcquireSRWLockShared(lock);
    } else {
      AcquireSRWLockExclusive(lock);
    }
    return status;
  }

  inline auto NativeRtlWakeConditionVariable =
    static_cast<void (NTAPI*)(_Inout_ PRTL_CONDITION_VARIABLE)>(nullptr);

  inline void NTAPI HookedRtlWakeConditionVariable(
      _Inout_ PRTL_CONDITION_VARIABLE variable) {
    auto& flag = *reinterpret_cast<std::atomic_uintptr_t*>(&variable->Ptr);
    ++flag;
    HookedRtlWakeAddressSingle(variable);
  }

  inline auto NativeRtlWakeAllConditionVariable =
    static_cast<void (NTAPI*)(_Inout_ PRTL_CONDITION_VARIABLE)>(nullptr);

  inline void WINAPI HookedRtlWakeAllConditionVariable(
      _Inout_ PRTL_CONDITION_VARIABLE variable) {
    auto& flag = *reinterpret_cast<std::atomic_uintptr_t*>(&variable->Ptr);
    ++flag;
    HookedRtlWakeAddressAll(variable);
  }

  inline bool InstallHooks() {
    isHooking = true;
    NativeRtlWaitOnAddress = Hook("RtlWaitOnAddress", HookedRtlWaitOnAddress);
    if(!NativeRtlWaitOnAddress) {
      return false;
    }
    NativeRtlWakeAddressSingle =
      Hook("RtlWakeAddressSingle", HookedRtlWakeAddressSingle);
    if(!NativeRtlWakeAddressSingle) {
      return false;
    }
    NativeRtlWakeAddressAll =
      Hook("RtlWakeAddressAll", HookedRtlWakeAddressAll);
    if(!NativeRtlWakeAddressAll) {
      return false;
    }
    NativeNtDelayExecution = Hook("NtDelayExecution", HookedNtDelayExecution);
    if(!NativeNtDelayExecution) {
      return false;
    }
    {
      auto kernel_module = GetModuleHandleW(L"ntdll.dll");
      if(!kernel_module) {
        return false;
      }
      auto target =
        GetProcAddress(kernel_module, "RtlTryAcquireSRWLockExclusive");
      if(!target) {
        return false;
      }
      #pragma warning(disable: 4191)
      RtlTryAcquireSRWLockExclusive =
        reinterpret_cast<BOOLEAN (NTAPI*)(_Inout_ PRTL_SRWLOCK)>(target);
      #pragma warning(default: 4191)
    }
    NativeRtlReleaseSRWLockExclusive =
      Hook("RtlReleaseSRWLockExclusive", HookedRtlReleaseSRWLockExclusive);
    if(!NativeRtlReleaseSRWLockExclusive) {
      return false;
    }
    NativeRtlSleepConditionVariableSRW =
      Hook("RtlSleepConditionVariableSRW", HookedRtlSleepConditionVariableSRW);
    if(!NativeRtlSleepConditionVariableSRW) {
      return false;
    }
    NativeRtlWakeConditionVariable =
      Hook("RtlWakeConditionVariable", HookedRtlWakeConditionVariable);
    if(!NativeRtlWakeConditionVariable) {
      return false;
    }
    NativeRtlWakeAllConditionVariable =
      Hook("RtlWakeAllConditionVariable", HookedRtlWakeAllConditionVariable);
    if(!NativeRtlWakeAllConditionVariable) {
      return false;
    }
    NativeRtlAcquireSRWLockExclusive =
      Hook("RtlAcquireSRWLockExclusive", HookedRtlAcquireSRWLockExclusive);
    if(!NativeRtlAcquireSRWLockExclusive) {
      return false;
    }
    isHooking = false;
    return true;
  }

  inline auto installHooks = InstallHooks();
}

#endif
