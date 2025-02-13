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

  std::size_t GetInstructionSize(const std::uint8_t* address) {
    auto op = address[0];
    switch(op) {
      case 0x90: // NOP
        return 1;
      case 0xC3: // RET
        return 1;
      case 0xCC: // INT 3 (breakpoint)
        return 1;
      case 0xE9: // jmp rel32
        return 5;
      case 0xEB: // jmp short rel8
        return 2;

      // MOV reg, imm (opcode 0xB8-0xBF)
      case 0xB8: case 0xB9: case 0xBA: case 0xBB:
      case 0xBC: case 0xBD: case 0xBE: case 0xBF:

        // MOV r32, imm32 in x64 typically takes 5 bytes.
        return 5;

      // Handle REX-prefixed opcodes minimally.
      default:

        // Check for REX prefix (0x40-0x4F)
        if (op >= 0x40 && op <= 0x4F) {

          // For simplicity, assume the next byte is an opcode we can handle.
          auto nextOp = address[1];
          switch (nextOp) {
            case 0x90: // REX NOP variant
              return 2;

            // Add additional cases as needed.
            default:
              throw std::runtime_error(
                "Unrecognized instruction after REX prefix");
          }
        }
        throw std::runtime_error("Unrecognized instruction");
    }
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
      auto instructionSize = GetInstructionSize(f + accumulated);
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
    auto jumpLocation =
      static_cast<std::uint8_t*>(trampoline) + instructions.m_size;
    auto jumpTarget =
      reinterpret_cast<std::uintptr_t>(target) + instructions.m_size;
    jumpLocation[0] = 0x48;
    jumpLocation[1] = 0xB8;
    *reinterpret_cast<uint64_t*>(jumpLocation + 2) = jumpTarget;
    jumpLocation[10] = 0xFF;
    jumpLocation[11] = 0xE0;
    return reinterpret_cast<F>(trampoline);
  }

  template <typename F>
  F Hook(std::string_view target_name, F hook, LPCWSTR module = L"ntdll.dll") {
      auto kernel_module = GetModuleHandleW(module);
      if (!kernel_module) {
          return nullptr;
      }
      auto target = reinterpret_cast<uint8_t*>(GetProcAddress(kernel_module, target_name.data()));
      if (!target) {
          return nullptr;
      }

      // x86-64: 14 bytes needed for a `mov rax, hook; jmp rax`
      constexpr size_t HOOK_SIZE = 14;
      constexpr size_t TRAMPOLINE_SIZE = 32;

      // Allocate trampoline
      auto trampoline = reinterpret_cast<uint8_t*>(VirtualAlloc(nullptr, TRAMPOLINE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
      if (!trampoline) {
          return nullptr;
      }

      // Copy original bytes to trampoline
      std::memcpy(trampoline, target, HOOK_SIZE);

      // Append jump from trampoline back to original function
      uint8_t trampoline_jump[] = {
          0x48, 0xB8, // mov rax, <original function>
          0, 0, 0, 0, 0, 0, 0, 0, // placeholder for function address
          0xFF, 0xE0  // jmp rax
      };
      *reinterpret_cast<uintptr_t*>(&trampoline_jump[2]) = reinterpret_cast<uintptr_t>(target + HOOK_SIZE);
      std::memcpy(trampoline + HOOK_SIZE, trampoline_jump, sizeof(trampoline_jump));

      // Prepare hook patch: `mov rax, hook; jmp rax`
      uint8_t hook_patch[] = {
          0x48, 0xB8, // mov rax, <hook function>
          0, 0, 0, 0, 0, 0, 0, 0, // placeholder for hook address
          0xFF, 0xE0  // jmp rax
      };
      *reinterpret_cast<uintptr_t*>(&hook_patch[2]) = reinterpret_cast<uintptr_t>(hook);

      // Modify memory protection to allow writing
      DWORD old_protect;
      if (!VirtualProtect(target, HOOK_SIZE, PAGE_EXECUTE_READWRITE, &old_protect)) {
          VirtualFree(trampoline, 0, MEM_RELEASE);
          return nullptr;
      }

      // Write the hook
      std::memcpy(target, hook_patch, sizeof(hook_patch));

      // Restore memory protection
      VirtualProtect(target, HOOK_SIZE, old_protect, &old_protect);

      return reinterpret_cast<F>(trampoline);
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
    auto& flag = *reinterpret_cast<std::atomic<LONG>*>(&lock->Ptr);
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
    auto& flag = *reinterpret_cast<std::atomic<LONG>*>(&lock->Ptr);
    HookedRtlWakeAddressAll(&flag);
  }

  inline auto NativeRtlSleepConditionVariableSRW =
    static_cast<NTSTATUS (NTAPI*)(_Inout_ PRTL_CONDITION_VARIABLE,
      _Inout_ PRTL_SRWLOCK, _In_opt_ PLARGE_INTEGER, _In_ ULONG)>(nullptr);

  inline NTSTATUS WINAPI HookedRtlSleepConditionVariableSRW(
      _Inout_ PRTL_CONDITION_VARIABLE variable, _Inout_ PRTL_SRWLOCK lock,
      _In_opt_ PLARGE_INTEGER timeout, _In_ ULONG flags) {
    auto value = reinterpret_cast<std::atomic<LONG>*>(&variable->Ptr)->load();
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
    auto& flag = *reinterpret_cast<std::atomic<LONG>*>(&variable->Ptr);
    ++flag;
    HookedRtlWakeAddressSingle(variable);
  }

  inline auto NativeRtlWakeAllConditionVariable =
    static_cast<void (NTAPI*)(_Inout_ PRTL_CONDITION_VARIABLE)>(nullptr);

  inline void WINAPI HookedRtlWakeAllConditionVariable(
      _Inout_ PRTL_CONDITION_VARIABLE variable) {
    auto& flag = *reinterpret_cast<std::atomic<LONG>*>(&variable->Ptr);
    ++flag;
    HookedRtlWakeAddressAll(variable);
  }

  inline bool InstallHooks() {
    isHooking = true;
    Routine::TLS_SLOT = TlsAlloc();
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
