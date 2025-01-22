#ifndef BEAM_WIN32_HOOKS_HPP
#define BEAM_WIN32_HOOKS_HPP
#include <array>
#include <string_view>
#include <thread>
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

  template <typename F>
  F Hook(std::string_view target_name, F hook, LPCWSTR module = L"ntdll.dll") {
    auto kernel_module = GetModuleHandleW(module);
    if(!kernel_module) {
      return nullptr;
    }
    auto target = GetProcAddress(kernel_module, target_name.data());
    if(!target) {
      return nullptr;
    }
    const auto TRAMPOLINE_SIZE = std::size_t(16);
    auto trampoline = VirtualAlloc(nullptr,
      TRAMPOLINE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if(!trampoline) {
      return nullptr;
    }
    const auto PROLOGUE_SIZE = std::size_t(5);
    std::memcpy(trampoline, target, PROLOGUE_SIZE);
    auto return_address =
      reinterpret_cast<std::uintptr_t>(target) + PROLOGUE_SIZE;
    auto trampoline_bytes = reinterpret_cast<BYTE*>(trampoline);
    const auto JMP = BYTE(0xE9);
    trampoline_bytes[PROLOGUE_SIZE] = JMP;
    const auto JMP_SIZE = std::size_t(5);
    *reinterpret_cast<std::int32_t*>(&trampoline_bytes[PROLOGUE_SIZE + 1]) =
      static_cast<std::int32_t>(return_address -
        (reinterpret_cast<std::uintptr_t>(trampoline) +
          PROLOGUE_SIZE + JMP_SIZE));
    auto old_protect = DWORD();
    if(!VirtualProtect(
        target, PROLOGUE_SIZE, PAGE_EXECUTE_READWRITE, &old_protect)) {
      VirtualFree(trampoline, 0, MEM_RELEASE);
      return nullptr;
    }
    auto hook_address = reinterpret_cast<std::uintptr_t>(hook);
    auto offset =
      hook_address - reinterpret_cast<std::uintptr_t>(target) - JMP_SIZE;
    BYTE jump_instruction[JMP_SIZE] = { JMP };
    std::memcpy(&jump_instruction[1], &offset, sizeof(std::int32_t));
    std::memcpy(target, jump_instruction, sizeof(jump_instruction));
    if(!VirtualProtect(target, PROLOGUE_SIZE, old_protect, &old_protect)) {
      VirtualFree(trampoline, 0, MEM_RELEASE);
      return nullptr;
    }
    return reinterpret_cast<F>(trampoline);
  }

  template<typename T>
  struct WaitEntry {
    Threading::SpinMutex m_mutex;
    SuspendedRoutineQueue<T> m_suspendedRoutines;
  };

  static auto waitEntries = std::array<WaitEntry<void*>, 256>();

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

  static auto OriginalRtlWaitOnAddress = static_cast<NTSTATUS (WINAPI*)(
    _In_ void*, _In_ void*, _In_ size_t, _In_opt_ PLARGE_INTEGER)>(nullptr);

  inline NTSTATUS WINAPI MyRtlWaitOnAddress(_In_ void* address,
      _In_ void* value, _In_ size_t size, _In_opt_ PLARGE_INTEGER timeout) {
    if(!Routine::IsInsideRoutine()) {
      return OriginalRtlWaitOnAddress(address, value, size, timeout);
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

  static auto OriginalRtlWakeAddressSingle =
    static_cast<void (WINAPI*)(_In_ void*)>(nullptr);

  inline void WINAPI MyRtlWakeAddressSingle(_In_ void* address) {
    OriginalRtlWakeAddressSingle(address);
    auto& waitEntry = GetWaitEntry(address);
    auto lock = std::unique_lock(waitEntry.m_mutex);
    if(!waitEntry.m_suspendedRoutines.empty()) {
      auto isInsideRoutine = Routine::IsInsideRoutine();
      Routine::IsInsideRoutine() = false;
      ResumeFirstMatch(Store(waitEntry.m_suspendedRoutines), address, lock);
      Routine::IsInsideRoutine() = isInsideRoutine;
    }
  }

  static auto OriginalRtlWakeAddressAll =
    static_cast<void (WINAPI*)(_In_ void*)>(nullptr);

  inline void WINAPI MyRtlWakeAddressAll(_In_ void* address) {
    OriginalRtlWakeAddressAll(address);
    auto& waitEntry = GetWaitEntry(address);
    auto lock = std::unique_lock(waitEntry.m_mutex);
    if(!waitEntry.m_suspendedRoutines.empty()) {
      auto isInsideRoutine = Routine::IsInsideRoutine();
      Routine::IsInsideRoutine() = false;
      ResumeAllMatches(Store(waitEntry.m_suspendedRoutines), address, lock);
      Routine::IsInsideRoutine() = isInsideRoutine;
    }
  }

  static auto OriginalNtDelayExecution =
    static_cast<NTSTATUS (NTAPI*)(BOOLEAN, PLARGE_INTEGER)>(nullptr);

  inline NTSTATUS NTAPI MyNtDelayExecution(
      BOOLEAN alertable, PLARGE_INTEGER timeout) {
    if(!Routine::IsInsideRoutine()) {
      return OriginalNtDelayExecution(alertable, timeout);
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

  static const auto LOCK_STATE_FREE = std::uintptr_t(0);
  static const auto LOCK_STATE_EXCLUSIVE = std::uintptr_t(1);
  static const auto LOCK_STATE_WAITING = std::uintptr_t(2);

  static auto OriginalRtlTryAcquireSRWLockExclusive =
    static_cast<BOOLEAN (WINAPI*)(RTL_SRWLOCK*)>(nullptr);

  inline BOOLEAN WINAPI MyRtlTryAcquireSRWLockExclusive(RTL_SRWLOCK* lock) {
    if(isHooking) {
      return OriginalRtlTryAcquireSRWLockExclusive(lock);
    }
    auto expected = LOCK_STATE_FREE;
    auto& flag = *reinterpret_cast<std::atomic<std::uintptr_t>*>(&lock->Ptr);
    if(flag.compare_exchange_strong(
        expected, LOCK_STATE_EXCLUSIVE, std::memory_order_acquire)) {
      return TRUE;
    }
    return FALSE;
  }

  static auto OriginalRtlAcquireSRWLockExclusive =
    static_cast<void (WINAPI*)(RTL_SRWLOCK*)>(nullptr);

  inline void WINAPI MyRtlAcquireSRWLockExclusive(RTL_SRWLOCK* lock) {
    if(isHooking) {
      return OriginalRtlAcquireSRWLockExclusive(lock);
    }
    auto expected = LOCK_STATE_FREE;
    auto& flag = *reinterpret_cast<std::atomic<std::uintptr_t>*>(&lock->Ptr);
    if(flag.compare_exchange_strong(
        expected, LOCK_STATE_EXCLUSIVE, std::memory_order_acquire)) {
      return;
    }
    while(true) {
      expected = flag.load(std::memory_order_relaxed);
      if(expected == LOCK_STATE_FREE) {
        if(flag.compare_exchange_strong(
            expected, LOCK_STATE_EXCLUSIVE, std::memory_order_acquire)) {
          return;
        }
      } else {
        if(flag.compare_exchange_strong(expected,
            expected | LOCK_STATE_WAITING, std::memory_order_acquire)) {
          MyRtlWaitOnAddress(&flag, &expected, sizeof(expected), nullptr);
        }
      }
    }
  }

  static auto OriginalRtlReleaseSRWLockExclusive =
    static_cast<void (NTAPI*)(RTL_SRWLOCK*)>(nullptr);

  inline void NTAPI MyRtlReleaseSRWLockExclusive(RTL_SRWLOCK* lock) {
    if(isHooking) {
      return OriginalRtlReleaseSRWLockExclusive(lock);
    }
    auto* flag = reinterpret_cast<std::atomic<std::uintptr_t>*>(&lock->Ptr);
    if(flag->exchange(
        LOCK_STATE_FREE, std::memory_order_release) & LOCK_STATE_WAITING) {
      MyRtlWakeAddressSingle(flag);
    }
  }

  static auto OriginalRtlSleepConditionVariableSRW =
    static_cast<NTSTATUS (WINAPI*)(RTL_CONDITION_VARIABLE*, RTL_SRWLOCK*,
      LARGE_INTEGER*, ULONG)>(nullptr);

  inline NTSTATUS WINAPI MyRtlSleepConditionVariableSRW(
      RTL_CONDITION_VARIABLE* variable, RTL_SRWLOCK* lock,
      LARGE_INTEGER* timeout, ULONG flags) {
    auto value = *reinterpret_cast<int*>(&variable->Ptr);
    if(flags & RTL_CONDITION_VARIABLE_LOCKMODE_SHARED) {
      ReleaseSRWLockShared(lock);
    } else {
      ReleaseSRWLockExclusive(lock);
    }
    auto status =
      MyRtlWaitOnAddress(&variable->Ptr, &value, sizeof(value), timeout);
    if(flags & RTL_CONDITION_VARIABLE_LOCKMODE_SHARED) {
      AcquireSRWLockShared(lock);
    } else {
      AcquireSRWLockExclusive(lock);
    }
    return status;
  }

  static auto OriginalRtlWakeConditionVariable =
    static_cast<void (WINAPI*)(RTL_CONDITION_VARIABLE*)>(nullptr);

  inline void WINAPI MyRtlWakeConditionVariable(
      RTL_CONDITION_VARIABLE* variable) {
    InterlockedIncrement(reinterpret_cast<LONG*>(&variable->Ptr));
    MyRtlWakeAddressSingle(variable);
  }

  static auto OriginalRtlWakeAllConditionVariable =
    static_cast<void (WINAPI*)(RTL_CONDITION_VARIABLE*)>(nullptr);

  inline void WINAPI MyRtlWakeAllConditionVariable(
      RTL_CONDITION_VARIABLE *variable) {
    InterlockedIncrement(reinterpret_cast<LONG*>(&variable->Ptr));
    MyRtlWakeAddressAll(variable);
  }

  inline bool InstallHooks() {
    isHooking = true;
    OriginalRtlWaitOnAddress = Hook("RtlWaitOnAddress", MyRtlWaitOnAddress);
    if(!OriginalRtlWaitOnAddress) {
      return false;
    }
    OriginalRtlWakeAddressSingle =
      Hook("RtlWakeAddressSingle", MyRtlWakeAddressSingle);
    if(!OriginalRtlWakeAddressSingle) {
      return false;
    }
    OriginalRtlWakeAddressAll = Hook("RtlWakeAddressAll", MyRtlWakeAddressAll);
    if(!OriginalRtlWakeAddressAll) {
      return false;
    }
    OriginalNtDelayExecution = Hook("NtDelayExecution", MyNtDelayExecution);
    if(!OriginalNtDelayExecution) {
      return false;
    }
    OriginalRtlTryAcquireSRWLockExclusive =
      Hook("RtlTryAcquireSRWLockExclusive", MyRtlTryAcquireSRWLockExclusive);
    if(!OriginalRtlTryAcquireSRWLockExclusive) {
      return false;
    }
    OriginalRtlReleaseSRWLockExclusive =
      Hook("RtlReleaseSRWLockExclusive", MyRtlReleaseSRWLockExclusive);
    if(!OriginalRtlReleaseSRWLockExclusive) {
      return false;
    }
    OriginalRtlSleepConditionVariableSRW =
      Hook("RtlSleepConditionVariableSRW", MyRtlSleepConditionVariableSRW);
    if(!OriginalRtlSleepConditionVariableSRW) {
      return false;
    }
    OriginalRtlWakeConditionVariable =
      Hook("RtlWakeConditionVariable", MyRtlWakeConditionVariable);
    if(!OriginalRtlWakeConditionVariable) {
      return false;
    }
    OriginalRtlWakeAllConditionVariable =
      Hook("RtlWakeAllConditionVariable", MyRtlWakeAllConditionVariable);
    if(!OriginalRtlWakeAllConditionVariable) {
      return false;
    }
    OriginalRtlAcquireSRWLockExclusive =
      Hook("RtlAcquireSRWLockExclusive", MyRtlAcquireSRWLockExclusive);
    if(!OriginalRtlAcquireSRWLockExclusive) {
      return false;
    }
    isHooking = false;
    return true;
  }

  struct HookInstaller {
    static inline auto installHooks = InstallHooks();
  };
}

#endif
