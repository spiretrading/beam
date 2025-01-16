#ifndef BEAM_WIN32_HOOKS_HPP
#define BEAM_WIN32_HOOKS_HPP
#include <atomic>
#include <string_view>
#include <thread>
#include <windows.h>
#include <winternl.h>
#include "Beam/Routines/Routine.hpp"

namespace Beam::Routines::Details {
  class SpinMutex {
    public:
      SpinMutex() noexcept = default;

      void lock() noexcept {
        while(m_flag.test_and_set(std::memory_order_acquire)) {}
      }

      void unlock() noexcept {
        m_flag.clear(std::memory_order_release);
      }

      bool try_lock() noexcept {
        return !m_flag.test_and_set(std::memory_order_acquire);
      }

    private:
      std::atomic_flag m_flag;

      SpinMutex(const SpinMutex&) = delete;
      SpinMutex& operator =(const SpinMutex&) = delete;
  };

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

  template<typename F>
  NTSTATUS CallNt(F&& f) {
    if(CurrentRoutineGlobal<void>::activeRoutineCount == 0 ||
        !CurrentRoutineGlobal<void>::isInsideRoutine) {
      return std::forward<F>(f)();
    }
    CurrentRoutineGlobal<void>::isInsideRoutine = false;
    auto result = NTSTATUS();
    {
      auto suspension = SuspendedRoutineQueue();
      auto mutex = SpinMutex();
      auto lock = std::unique_lock(mutex);
      auto thread = std::thread([&] {
        result = std::forward<F>(f)();
        {
          auto lock = std::lock_guard(mutex);
          ResumeFront(Store(suspension));
        }
      });
      thread.detach();
      Suspend(Store(suspension), lock);
    }
    CurrentRoutineGlobal<void>::isInsideRoutine = true;
    return result;
  }

  static auto RtlWaitOnAddress = static_cast<NTSTATUS (WINAPI*)(
    _In_ void*, _In_ void*, _In_ size_t, _In_opt_ PLARGE_INTEGER)>(nullptr);
  static auto RtlWakeAddressSingle =
    static_cast<void (WINAPI*)(_In_ void*)>(nullptr);
  static auto RtlWakeAddressAll =
    static_cast<void (WINAPI*)(_In_ void*)>(nullptr);

  static auto OriginalNtDelayExecution =
    static_cast<NTSTATUS (NTAPI*)(BOOLEAN, PLARGE_INTEGER)>(nullptr);

  inline NTSTATUS NTAPI MyNtDelayExecution(
      BOOLEAN alertable, PLARGE_INTEGER timeout) {
    return CallNt([=] {
      return OriginalNtDelayExecution(alertable, timeout);
    });
  }

  static auto OriginalNtWaitForSingleObject =
    static_cast<NTSTATUS (NTAPI*)(HANDLE, BOOLEAN, PLARGE_INTEGER)>(nullptr);

  inline NTSTATUS NTAPI MyNtWaitForSingleObject(
      HANDLE handle, BOOLEAN alertable, PLARGE_INTEGER timeout) {
    return CallNt([=] {
      return OriginalNtWaitForSingleObject(handle, alertable, timeout);
    });
  }

  static auto OriginalNtWaitForKeyedEvent =
    static_cast<NTSTATUS (NTAPI*)(HANDLE, PVOID, BOOLEAN, PLARGE_INTEGER)>(
      nullptr);

  inline NTSTATUS NTAPI MyNtWaitForKeyedEvent(HANDLE eventHandle, PVOID key,
      BOOLEAN alertable, PLARGE_INTEGER timeout) {
    return CallNt([=] {
      return OriginalNtWaitForKeyedEvent(eventHandle, key, alertable, timeout);
    });
  }

  static auto OriginalNtWriteFile =
    static_cast<NTSTATUS (NTAPI*)(HANDLE, HANDLE, PIO_APC_ROUTINE, PVOID,
      PIO_STATUS_BLOCK, PVOID, ULONG, PLARGE_INTEGER, PULONG)>(nullptr);

  inline NTSTATUS NTAPI MyNtWriteFile(HANDLE file_handle, HANDLE event,
      PIO_APC_ROUTINE  apc_routine, PVOID apc_context,
      PIO_STATUS_BLOCK io_status_block, PVOID buffer, ULONG length,
      PLARGE_INTEGER byte_offset, PULONG key) {
    return CallNt([=] {
      return OriginalNtWriteFile(file_handle, event, apc_routine, apc_context,
        io_status_block, buffer, length, byte_offset, key);
    });
  }

  static auto OriginalRtlSleepConditionVariableSRW =
    static_cast<NTSTATUS (WINAPI*)(RTL_CONDITION_VARIABLE*, RTL_SRWLOCK*,
      LARGE_INTEGER*, ULONG)>(nullptr);

  inline NTSTATUS WINAPI MyRtlSleepConditionVariableSRW(
      RTL_CONDITION_VARIABLE* variable, RTL_SRWLOCK* lock,
      LARGE_INTEGER* timeout, ULONG flags) {
    if(InterlockedCompareExchangePointer(reinterpret_cast<void**>(
        &variable->Ptr), nullptr, nullptr) == nullptr) {
    }
    auto value = *reinterpret_cast<int*>(&variable->Ptr);
    if(flags & RTL_CONDITION_VARIABLE_LOCKMODE_SHARED) {
      ReleaseSRWLockShared(lock);
    } else {
      ReleaseSRWLockExclusive(lock);
    }
    auto status =
      RtlWaitOnAddress(&variable->Ptr, &value, sizeof(value), timeout);
    if(flags & RTL_CONDITION_VARIABLE_LOCKMODE_SHARED) {
      AcquireSRWLockShared(lock);
    } else {
      AcquireSRWLockExclusive(lock);
    }
    return status;
  }

  static auto OriginalRtlWakeConditionVariable =
    static_cast<void (WINAPI*)(RTL_CONDITION_VARIABLE*)>(nullptr);

  void WINAPI MyRtlWakeConditionVariable(RTL_CONDITION_VARIABLE* variable) {
    InterlockedIncrement(reinterpret_cast<LONG*>(&variable->Ptr));
    RtlWakeAddressSingle(variable);
  }

  static auto OriginalRtlWakeAllConditionVariable =
    static_cast<void (WINAPI*)(RTL_CONDITION_VARIABLE*)>(nullptr);

  void WINAPI MyRtlWakeAllConditionVariable(RTL_CONDITION_VARIABLE *variable) {
    InterlockedIncrement(reinterpret_cast<LONG*>(&variable->Ptr));
    RtlWakeAddressAll(variable);
  }

  inline void InstallHooks() {
    RtlWaitOnAddress = reinterpret_cast<NTSTATUS (WINAPI*)(
      _In_ void*, _In_ void*, _In_ size_t, _In_opt_ PLARGE_INTEGER)>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlWaitOnAddress"));
    RtlWakeAddressSingle = reinterpret_cast<void (WINAPI*)(_In_ void*)>(
      GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlWakeAddressSingle"));
    RtlWakeAddressAll = reinterpret_cast<void (WINAPI*)(_In_ void*)>(
      GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlWakeAddressAll"));
    OriginalNtDelayExecution = Hook("NtDelayExecution", MyNtDelayExecution);
    if(!OriginalNtDelayExecution) {
      return;
    }
    OriginalNtWaitForSingleObject =
      Hook("NtWaitForSingleObject", MyNtWaitForSingleObject);
    if(!OriginalNtWaitForSingleObject) {
      return;
    }
    OriginalNtWaitForKeyedEvent =
      Hook("NtWaitForKeyedEvent", MyNtWaitForKeyedEvent);
    if(!OriginalNtWaitForKeyedEvent) {
      return;
    }
    OriginalNtWriteFile = Hook("NtWriteFile", MyNtWriteFile);
    if(!OriginalNtWriteFile) {
      return;
    }
    OriginalRtlSleepConditionVariableSRW =
      Hook("RtlSleepConditionVariableSRW", MyRtlSleepConditionVariableSRW);
    if(!OriginalRtlSleepConditionVariableSRW) {
      return;
    }
    OriginalRtlWakeConditionVariable =
      Hook("RtlWakeConditionVariable", MyRtlWakeConditionVariable);
    if(!OriginalRtlWakeConditionVariable) {
      return;
    }
    OriginalRtlWakeAllConditionVariable =
      Hook("RtlWakeAllConditionVariable", MyRtlWakeAllConditionVariable);
    if(!OriginalRtlWakeAllConditionVariable) {
      return;
    }
  }
}

#endif
