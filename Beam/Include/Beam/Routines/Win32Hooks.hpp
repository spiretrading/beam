#ifndef BEAM_WIN32_HOOKS_HPP
#define BEAM_WIN32_HOOKS_HPP
#include <array>
#include <string_view>
#include <thread>
#include <windows.h>
#include <winternl.h>
#include "Beam/Routines/Routine.hpp"
#include "Beam/Threading/SpinMutex.hpp"

namespace Beam::Routines::Details {
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
    if(!Routine::m_isInsideRoutine) {
      return std::forward<F>(f)();
    }
    Routine::m_isInsideRoutine = false;
    auto result = NTSTATUS();
    {
      auto suspension = SuspendedRoutineQueue();
      auto mutex = Threading::SpinMutex();
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
    Routine::m_isInsideRoutine = true;
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

  struct WaitEntry {
    Threading::SpinMutex m_mutex;
    SuspendedRoutineQueue m_suspendedRoutines;
  };

  inline WaitEntry& GetWaitEntry(void* address) {
    static auto entries = std::array<WaitEntry, 256>();
    return entries[(reinterpret_cast<std::uintptr_t>(address) >> 4) %
      entries.size()];
  }

  inline NTSTATUS WaitOn(void* address, int value, LARGE_INTEGER* timeout) {
    auto& waitEntry = GetWaitEntry(address);
    auto lock = std::unique_lock(waitEntry.m_mutex);
    if(*reinterpret_cast<int*>(address) != value) {
      return 0;
    }
    Suspend(Store(waitEntry.m_suspendedRoutines), lock);
    return 0;
  }

  inline void WakeAll(void* address) {
    auto& waitEntry = GetWaitEntry(address);
    auto lock = std::lock_guard(waitEntry.m_mutex);
    if(!waitEntry.m_suspendedRoutines.empty()) {
      Resume(Store(waitEntry.m_suspendedRoutines));
    }
  }

  inline void WakeSingle(void* address) {
    WakeAll(address);
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
    auto status = [&] {
      if(!Routine::m_isInsideRoutine) {
        return RtlWaitOnAddress(&variable->Ptr, &value, sizeof(value), timeout);
      } else {
        return WaitOn(&variable->Ptr, value, timeout);
      }
    }();
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
    WakeSingle(variable);
    RtlWakeAddressSingle(variable);
  }

  static auto OriginalRtlWakeAllConditionVariable =
    static_cast<void (WINAPI*)(RTL_CONDITION_VARIABLE*)>(nullptr);

  inline void WINAPI MyRtlWakeAllConditionVariable(
      RTL_CONDITION_VARIABLE *variable) {
    InterlockedIncrement(reinterpret_cast<LONG*>(&variable->Ptr));
    WakeAll(variable);
    RtlWakeAddressAll(variable);
  }

  inline bool InstallHooks() {
    #pragma warning(push)
    #pragma warning(disable:4191)
    RtlWaitOnAddress = reinterpret_cast<NTSTATUS (WINAPI*)(
      _In_ void*, _In_ void*, _In_ size_t, _In_opt_ PLARGE_INTEGER)>(
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlWaitOnAddress"));
    RtlWakeAddressSingle = reinterpret_cast<void (WINAPI*)(_In_ void*)>(
      GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlWakeAddressSingle"));
    RtlWakeAddressAll = reinterpret_cast<void (WINAPI*)(_In_ void*)>(
      GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlWakeAddressAll"));
    #pragma warning(pop)
    OriginalNtDelayExecution = Hook("NtDelayExecution", MyNtDelayExecution);
    if(!OriginalNtDelayExecution) {
      return false;
    }
    OriginalNtWaitForSingleObject =
      Hook("NtWaitForSingleObject", MyNtWaitForSingleObject);
    if(!OriginalNtWaitForSingleObject) {
      return false;
    }
    OriginalNtWaitForKeyedEvent =
      Hook("NtWaitForKeyedEvent", MyNtWaitForKeyedEvent);
    if(!OriginalNtWaitForKeyedEvent) {
      return false;
    }
    OriginalNtWriteFile = Hook("NtWriteFile", MyNtWriteFile);
    if(!OriginalNtWriteFile) {
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
    return true;
  }

  struct HookInstaller {
    static inline auto installHooks = InstallHooks();
  };
}

#endif
