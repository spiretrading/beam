#ifndef BEAM_WIN32_HOOKS_HPP
#define BEAM_WIN32_HOOKS_HPP
#include <atomic>
#include <string_view>
#include <thread>
#include <windows.h>
#include <winternl.h>
#include "Beam/Routines/Routine.hpp"

namespace Beam::Routines::Details {
  struct ThreadIdNode {
    DWORD m_originalThreadId;
    DWORD m_targetThreadId;
    std::atomic<ThreadIdNode*> m_previous;
    std::atomic<ThreadIdNode*> m_next;
    static inline auto head = std::atomic<ThreadIdNode*>(nullptr);

    static void add(ThreadIdNode& node) {
      auto oldHead = static_cast<ThreadIdNode*>(nullptr);
      do {
        oldHead = head.load(std::memory_order_relaxed);
        node.m_next.store(oldHead, std::memory_order_relaxed);
        if(oldHead) {
          oldHead->m_previous.store(&node, std::memory_order_relaxed);
        }
      } while(!head.compare_exchange_weak(oldHead, &node,
          std::memory_order_release, std::memory_order_relaxed));
    }

    static void remove(ThreadIdNode& node) {
      auto previous = node.m_previous.load(std::memory_order_acquire);
      auto next = node.m_next.load(std::memory_order_acquire);
      if(previous) {
        previous->m_next.store(next, std::memory_order_release);
      } else {
        auto expected = &node;
        head.compare_exchange_strong(expected, next,
          std::memory_order_release, std::memory_order_relaxed);
      }
      if(next) {
        next->m_previous.store(previous, std::memory_order_release);
      }
    }

    static ThreadIdNode* find(DWORD originalThreadId) {
      auto current = head.load(std::memory_order_acquire);
      while(current) {
        if(current->m_originalThreadId == originalThreadId) {
          return current;
        }
        current = current->m_next.load(std::memory_order_acquire);
      }
      return nullptr;
    }
  };

  template <typename F>
  F Hook(std::string_view target_name, F hook) {
    auto kernel_module = GetModuleHandleW(L"ntdll.dll");
    if(!kernel_module) {
      return nullptr;
    }
    auto target = GetProcAddress(kernel_module, target_name.data());
    if(!target) {
      return nullptr;
    }
    const auto TRAMPOLINE_SIZE = std::size_t(16);
    auto trampoline = VirtualAlloc(
      nullptr, TRAMPOLINE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
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
    auto suspension = SuspendedRoutineQueue();
    auto result = NTSTATUS();
    auto mutex = std::mutex();
    auto thread = std::thread([&] {
      result = std::forward<F>(f)();
      {
        auto lock = std::lock_guard(mutex);
        ResumeFront(Store(suspension));
      }
    });
    thread.detach();
    {
      auto lock = std::unique_lock(mutex);
      Suspend(Store(suspension), lock);
    }
    CurrentRoutineGlobal<void>::isInsideRoutine = true;
    return result;
  }

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

  static auto OriginalNtAlertThreadByThreadId =
    static_cast<NTSTATUS (NTAPI*)(HANDLE)>(nullptr);

  inline NTSTATUS NTAPI MyNtAlertThreadByThreadId(HANDLE threadId) {
    if(auto node = ThreadIdNode::find(reinterpret_cast<DWORD>(threadId))) {
      return OriginalNtAlertThreadByThreadId(reinterpret_cast<HANDLE>(
        static_cast<std::uintptr_t>(node->m_targetThreadId)));
    } else {
      return OriginalNtAlertThreadByThreadId(threadId);
    }
  }

  static auto OriginalNtWaitForAlertByThreadId =
    static_cast<NTSTATUS (NTAPI*)(PVOID, PLARGE_INTEGER)>(nullptr);

  inline NTSTATUS NTAPI MyNtWaitForAlertByThreadId(
      PVOID address, PLARGE_INTEGER timeout) {
    auto threadId = GetCurrentThreadId();
    return CallNt([=] {
      auto targetId = GetCurrentThreadId();
      if(threadId == targetId) {
        return OriginalNtWaitForAlertByThreadId(address, timeout);
      }
      auto node = ThreadIdNode(threadId, targetId, nullptr);
      ThreadIdNode::add(node);
      auto result = OriginalNtWaitForAlertByThreadId(address, timeout);
      ThreadIdNode::remove(node);
      return result;
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

  inline void InstallHooks() {
    OriginalNtDelayExecution = Hook("NtDelayExecution", MyNtDelayExecution);
    if(!OriginalNtDelayExecution) {
      return;
    }
    OriginalNtWaitForSingleObject =
      Hook("NtWaitForSingleObject", MyNtWaitForSingleObject);
    if(!OriginalNtWaitForSingleObject) {
      return;
    }
    OriginalNtAlertThreadByThreadId =
      Hook("NtAlertThreadByThreadId", MyNtAlertThreadByThreadId);
    if(!OriginalNtAlertThreadByThreadId) {
      return;
    }
    OriginalNtWaitForAlertByThreadId =
      Hook("NtWaitForAlertByThreadId", MyNtWaitForAlertByThreadId);
    if(!OriginalNtWaitForAlertByThreadId) {
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
  }
}

#endif
