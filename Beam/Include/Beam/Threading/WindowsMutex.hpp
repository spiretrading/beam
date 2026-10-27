#ifndef BEAM_WINDOWS_MUTEX_HPP
#define BEAM_WINDOWS_MUTEX_HPP
#if defined(_MSC_VER) && defined(_DLL)
#include <array>
#include <atomic>
#include <cstdint>
#include <system_error>
#include <Windows.h>
#include <TlHelp32.h>
#include "Beam/Threading/NativeConditionVariable.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam {
  class Routine;
}

namespace Beam::Details {
  class BEAM_EXPORT_DLL WindowsMutex {
    public:

      /**
       * Enables routine-aware mutexes, untimed condition variable waits, and
       * one-time initialization in loaded modules using this MSVC CRT.
       * Must be called before dispatching routines.
       */
      static void initialize();

    private:
      static constexpr auto LOCKED = std::uintptr_t(1);
      static constexpr auto HAS_WAITERS = std::uintptr_t(2);
      struct Waiter {
        PSRWLOCK m_mutex;
        Routine* m_routine;
        DWORD m_thread;
        Waiter* m_next;
        NativeConditionVariable m_notification;
        std::atomic_bool m_is_notified;
        bool m_is_suspended;
      };
      struct alignas(64) Bucket {
        NativeMutex m_mutex = NativeMutex();
        Waiter* m_waiters = nullptr;
      };
      struct ConditionWaiter {
        PCONDITION_VARIABLE m_condition;
        Routine* m_routine;
        ConditionWaiter* m_next;
        NativeConditionVariable m_notification;
        bool m_is_notified;
      };
      struct ConditionBucket {
        NativeMutex m_mutex = NativeMutex();
        ConditionWaiter* m_waiters = nullptr;
      };
      struct OnceWaiter {
        PINIT_ONCE m_once;
        Routine* m_routine;
        OnceWaiter* m_next;
        NativeConditionVariable m_notification;
        bool m_is_notified;
      };
      struct OnceBucket {
        NativeMutex m_mutex = NativeMutex();
        OnceWaiter* m_waiters = nullptr;
      };
      struct State {
        decltype(&_Mtx_lock) m_lock = reinterpret_cast<decltype(&_Mtx_lock)>(
          GetProcAddress(get_runtime(), "_Mtx_lock"));
        decltype(&_Mtx_trylock) m_try_lock =
          reinterpret_cast<decltype(&_Mtx_trylock)>(
            GetProcAddress(get_runtime(), "_Mtx_trylock"));
        decltype(&_Mtx_unlock) m_unlock =
          reinterpret_cast<decltype(&_Mtx_unlock)>(
            GetProcAddress(get_runtime(), "_Mtx_unlock"));
        decltype(&_Cnd_wait) m_wait = reinterpret_cast<decltype(&_Cnd_wait)>(
          GetProcAddress(get_runtime(), "_Cnd_wait"));
        decltype(&SleepConditionVariableSRW) m_sleep =
          reinterpret_cast<decltype(&SleepConditionVariableSRW)>(
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"),
              "SleepConditionVariableSRW"));
        decltype(&WakeConditionVariable) m_notify_one =
          reinterpret_cast<decltype(&WakeConditionVariable)>(
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"),
              "WakeConditionVariable"));
        decltype(&WakeAllConditionVariable) m_notify_all =
          reinterpret_cast<decltype(&WakeAllConditionVariable)>(
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"),
              "WakeAllConditionVariable"));
        decltype(&InitOnceBeginInitialize) m_begin =
          reinterpret_cast<decltype(&InitOnceBeginInitialize)>(
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"),
              "InitOnceBeginInitialize"));
        decltype(&InitOnceComplete) m_complete =
          reinterpret_cast<decltype(&InitOnceComplete)>(
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"),
              "InitOnceComplete"));
        std::array<Bucket, 64> m_buckets;
        std::array<ConditionBucket, 64> m_conditions;
        std::array<OnceBucket, 64> m_initializers;
      };
      class Guard {
        public:
          Guard() noexcept;
          ~Guard();

        private:
          bool m_previous;
      };
      static HMODULE get_runtime();
      static State& get_state();
      static bool& is_entered();
      static Routine* get_routine();
      static PSRWLOCK get_lock(_Mtx_t mutex);
      static Bucket& get_bucket(PSRWLOCK mutex);
      static ConditionBucket& get_bucket(PCONDITION_VARIABLE condition);
      static OnceBucket& get_bucket(PINIT_ONCE once);
      static std::uintptr_t enter(_Mtx_t mutex);
      static void leave(_Mtx_t mutex, std::uintptr_t state);
      static Routine* notify(Bucket& bucket, PSRWLOCK mutex);
      static bool is_recursive_owner(_Mtx_t mutex, Routine& routine);
      static _Thrd_result acquire(_Mtx_t mutex, Routine& routine);
      static _Thrd_result acquire(
        _Mtx_t mutex, Routine* routine, std::uintptr_t& state, bool selected);
      static _Thrd_result lock(_Mtx_t mutex, Routine* routine);
      static _Thrd_result __cdecl lock(_Mtx_t mutex) noexcept;
      static _Thrd_result __cdecl try_lock(_Mtx_t mutex) noexcept;
      static _Thrd_result __cdecl unlock(_Mtx_t mutex) noexcept;
      static _Thrd_result __cdecl wait(_Cnd_t condition, _Mtx_t mutex) noexcept;
      static BOOL WINAPI sleep(PCONDITION_VARIABLE condition, PSRWLOCK mutex,
        DWORD timeout, ULONG flags) noexcept;
      static BOOL wait(PCONDITION_VARIABLE condition, PSRWLOCK mutex,
        DWORD timeout, ULONG flags, _Mtx_t owner) noexcept;
      static void notify(PCONDITION_VARIABLE condition, bool all) noexcept;
      static void WINAPI notify_one(PCONDITION_VARIABLE condition) noexcept;
      static void WINAPI notify_all(PCONDITION_VARIABLE condition) noexcept;
      static BOOL WINAPI begin(PINIT_ONCE once, DWORD flags, PBOOL pending,
        PVOID* context) noexcept;
      static BOOL WINAPI complete(
        PINIT_ONCE once, DWORD flags, PVOID context) noexcept;
      static void patch(HMODULE module);
  };
}

#include "Beam/Routines/Routine.hpp"

namespace Beam::Details {

  inline void WindowsMutex::initialize() {
    static auto initialized = [] {
      auto& state = get_state();
      if(!state.m_lock || !state.m_try_lock || !state.m_unlock ||
          !state.m_wait || !state.m_sleep || !state.m_notify_one ||
          !state.m_notify_all || !state.m_begin || !state.m_complete) {
        throw std::system_error(ERROR_PROC_NOT_FOUND, std::system_category());
      }
      auto module = HMODULE();
      if(!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
          GET_MODULE_HANDLE_EX_FLAG_PIN,
          reinterpret_cast<LPCSTR>(
            static_cast<decltype(&_Mtx_lock)>(&lock)), &module)) {
        throw std::system_error(GetLastError(), std::system_category());
      }
      auto snapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE, GetCurrentProcessId());
      if(snapshot == INVALID_HANDLE_VALUE) {
        throw std::system_error(GetLastError(), std::system_category());
      }
      try {
        auto entry = MODULEENTRY32();
        entry.dwSize = sizeof(entry);
        if(!Module32First(snapshot, &entry)) {
          throw std::system_error(GetLastError(), std::system_category());
        }
        do {
          auto module = HMODULE();
          if(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
              reinterpret_cast<LPCSTR>(entry.hModule), &module)) {
            try {
              patch(module);
            } catch(...) {
              FreeLibrary(module);
              throw;
            }
            FreeLibrary(module);
          }
        } while(Module32Next(snapshot, &entry));
        if(GetLastError() != ERROR_NO_MORE_FILES) {
          throw std::system_error(GetLastError(), std::system_category());
        }
      } catch(...) {
        CloseHandle(snapshot);
        throw;
      }
      CloseHandle(snapshot);
      return true;
    }();
    static_cast<void>(initialized);
  }

  inline WindowsMutex::Guard::Guard() noexcept
      : m_previous(is_entered()) {
    is_entered() = true;
  }

  inline WindowsMutex::Guard::~Guard() {
    is_entered() = m_previous;
  }

  inline HMODULE WindowsMutex::get_runtime() {
    #ifdef _DEBUG
      return GetModuleHandleW(L"msvcp140d.dll");
    #else
      return GetModuleHandleW(L"msvcp140.dll");
    #endif
  }

  inline WindowsMutex::State& WindowsMutex::get_state() {
    static auto state = State();
    return state;
  }

  inline bool& WindowsMutex::is_entered() {
    static thread_local auto entered = false;
    return entered;
  }

  inline Routine* WindowsMutex::get_routine() {
    if(!CurrentRoutineGlobal::is_scheduled()) {
      return nullptr;
    }
    auto routine = CurrentRoutineGlobal::get();
    if(!routine || routine->get_state() != Routine::State::RUNNING) {
      return nullptr;
    }
    return routine;
  }

  inline PSRWLOCK WindowsMutex::get_lock(_Mtx_t mutex) {
    return reinterpret_cast<PSRWLOCK>(
      &mutex->_Critical_section._M_srw_lock);
  }

  inline WindowsMutex::Bucket& WindowsMutex::get_bucket(PSRWLOCK mutex) {
    auto& buckets = get_state().m_buckets;
    return buckets[(reinterpret_cast<std::uintptr_t>(mutex) >> 3) %
      buckets.size()];
  }

  inline WindowsMutex::ConditionBucket& WindowsMutex::get_bucket(
      PCONDITION_VARIABLE condition) {
    auto& buckets = get_state().m_conditions;
    return buckets[(reinterpret_cast<std::uintptr_t>(condition) >> 3) %
      buckets.size()];
  }

  inline WindowsMutex::OnceBucket& WindowsMutex::get_bucket(PINIT_ONCE once) {
    auto& buckets = get_state().m_initializers;
    return buckets[(reinterpret_cast<std::uintptr_t>(once) >> 3) %
      buckets.size()];
  }

  inline std::uintptr_t WindowsMutex::enter(_Mtx_t mutex) {
    static_assert(std::atomic_ref<void*>::is_always_lock_free);
    static_assert(alignof(Routine) > (LOCKED | HAS_WAITERS));
    auto state = std::atomic_ref(mutex->_Critical_section._Unused);
    auto value = state.load(std::memory_order_relaxed);
    while(true) {
      auto bits = reinterpret_cast<std::uintptr_t>(value);
      if((bits & LOCKED) == 0 && state.compare_exchange_weak(value,
          reinterpret_cast<void*>(bits | LOCKED), std::memory_order_acquire,
          std::memory_order_relaxed)) {
        return bits;
      }
      YieldProcessor();
      value = state.load(std::memory_order_relaxed);
    }
  }

  inline void WindowsMutex::leave(_Mtx_t mutex, std::uintptr_t state) {
    std::atomic_ref(mutex->_Critical_section._Unused).store(
      reinterpret_cast<void*>(state), std::memory_order_release);
  }

  inline Routine* WindowsMutex::notify(Bucket& bucket, PSRWLOCK mutex) {
    for(auto waiter = bucket.m_waiters; waiter; waiter = waiter->m_next) {
      if(waiter->m_mutex == mutex) {
        if(!waiter->m_is_notified.load(std::memory_order_relaxed)) {
          waiter->m_is_notified.store(true, std::memory_order_release);
          if(waiter->m_is_suspended) {
            if(waiter->m_routine) {
              return waiter->m_routine;
            }
            waiter->m_notification.notify_one();
          }
        }
        break;
      }
    }
    return nullptr;
  }

  inline bool WindowsMutex::is_recursive_owner(
      _Mtx_t mutex, Routine& routine) {
    if((mutex->_Type & _Mtx_recursive) == 0) {
      return false;
    }
    auto state = reinterpret_cast<std::uintptr_t>(
      std::atomic_ref(mutex->_Critical_section._Unused).load(
        std::memory_order_relaxed));
    return (state & ~(LOCKED | HAS_WAITERS)) ==
      reinterpret_cast<std::uintptr_t>(&routine);
  }

  inline _Thrd_result WindowsMutex::acquire(_Mtx_t mutex, Routine& routine) {
    if(is_recursive_owner(mutex, routine)) {
      ++mutex->_Count;
      return _Thrd_result::_Success;
    }
    auto owner = reinterpret_cast<std::uintptr_t>(&routine);
    auto expected = static_cast<void*>(nullptr);
    if(!std::atomic_ref(mutex->_Critical_section._Unused).
        compare_exchange_strong(expected,
          reinterpret_cast<void*>(owner | LOCKED),
          std::memory_order_acquire, std::memory_order_relaxed)) {
      return _Thrd_result::_Busy;
    }
    if(!TryAcquireSRWLockExclusive(get_lock(mutex))) {
      leave(mutex, 0);
      return _Thrd_result::_Busy;
    }
    mutex->_Thread_id = static_cast<long>(GetCurrentThreadId());
    mutex->_Count = 1;
    leave(mutex, owner);
    return _Thrd_result::_Success;
  }

  inline _Thrd_result WindowsMutex::acquire(
      _Mtx_t mutex, Routine* routine, std::uintptr_t& state, bool selected) {
    auto owner = reinterpret_cast<Routine*>(state & ~HAS_WAITERS);
    if(owner && owner != routine) {
      return _Thrd_result::_Busy;
    }
    if((state & HAS_WAITERS) != 0 && !selected) {
      if(owner != routine || mutex->_Count == 0 ||
          mutex->_Thread_id != static_cast<long>(GetCurrentThreadId())) {
        return _Thrd_result::_Busy;
      }
    }
    auto result = get_state().m_try_lock(mutex);
    if(result == _Thrd_result::_Success) {
      state = reinterpret_cast<std::uintptr_t>(routine) |
        (state & HAS_WAITERS);
    }
    return result;
  }

  inline _Thrd_result WindowsMutex::lock(_Mtx_t mutex, Routine* routine) {
    auto waiter = Waiter(get_lock(mutex), routine, GetCurrentThreadId(),
      nullptr, NativeConditionVariable(), false, false);
    auto guard = Guard();
    auto& bucket = get_bucket(get_lock(mutex));
    auto state = enter(mutex);
    auto bucket_lock = std::unique_lock(bucket.m_mutex);
    auto result = acquire(mutex, routine, state, false);
    if(result != _Thrd_result::_Busy) {
      leave(mutex, state);
      return result;
    }
    auto entry = &bucket.m_waiters;
    auto should_spin = mutex->_Thread_id != static_cast<long>(waiter.m_thread);
    while(*entry) {
      if((*entry)->m_mutex == get_lock(mutex) &&
          (*entry)->m_thread == waiter.m_thread) {
        should_spin = false;
      }
      entry = &(*entry)->m_next;
    }
    *entry = &waiter;
    state |= HAS_WAITERS;
    while(true) {
      waiter.m_is_notified.store(false, std::memory_order_relaxed);
      waiter.m_is_suspended = false;
      leave(mutex, state);
      auto should_suspend = true;
      if(should_spin) {
        bucket_lock.unlock();
        for(auto i = 0; i != 4096; ++i) {
          if(waiter.m_is_notified.load(std::memory_order_acquire)) {
            break;
          }
          YieldProcessor();
        }
        should_suspend = !waiter.m_is_notified.load(std::memory_order_acquire);
        if(should_suspend) {
          bucket_lock.lock();
          should_suspend =
            !waiter.m_is_notified.load(std::memory_order_relaxed);
          if(!should_suspend) {
            bucket_lock.unlock();
          }
        }
      }
      if(should_suspend) {
        waiter.m_is_suspended = true;
        if(routine) {
          routine->pending_suspend();
          bucket_lock.unlock();
          is_entered() = false;
          routine->suspend();
          is_entered() = true;
        } else {
          waiter.m_notification.wait(bucket_lock, [&] {
            return waiter.m_is_notified.load(std::memory_order_relaxed);
          });
          bucket_lock.unlock();
        }
      }
      state = enter(mutex);
      bucket_lock.lock();
      result = acquire(mutex, routine, state, true);
      if(result == _Thrd_result::_Busy) {
        continue;
      }
      entry = &bucket.m_waiters;
      auto has_waiters = false;
      while(*entry) {
        if(*entry == &waiter) {
          *entry = waiter.m_next;
        } else {
          has_waiters |= (*entry)->m_mutex == get_lock(mutex);
          entry = &(*entry)->m_next;
        }
      }
      if(!has_waiters) {
        state &= ~HAS_WAITERS;
      }
      leave(mutex, state);
      return result;
    }
  }

  inline _Thrd_result __cdecl WindowsMutex::lock(_Mtx_t mutex) noexcept {
    auto routine = static_cast<Routine*>(nullptr);
    if(!is_entered()) {
      routine = get_routine();
    }
    if(!routine) {
      return get_state().m_lock(mutex);
    }
    auto result = acquire(mutex, *routine);
    if(result != _Thrd_result::_Busy) {
      return result;
    }
    return lock(mutex, routine);
  }

  inline _Thrd_result __cdecl WindowsMutex::try_lock(_Mtx_t mutex) noexcept {
    auto routine = static_cast<Routine*>(nullptr);
    if(!is_entered()) {
      routine = get_routine();
    }
    if(!routine) {
      return get_state().m_try_lock(mutex);
    }
    return acquire(mutex, *routine);
  }

  inline _Thrd_result __cdecl WindowsMutex::unlock(_Mtx_t mutex) noexcept {
    if(mutex->_Count > 1) {
      --mutex->_Count;
      return _Thrd_result::_Success;
    }
    auto state = enter(mutex);
    mutex->_Thread_id = -1;
    mutex->_Count = 0;
    ReleaseSRWLockExclusive(get_lock(mutex));
    state &= HAS_WAITERS;
    if(state == 0) {
      leave(mutex, state);
      return _Thrd_result::_Success;
    }
    auto guard = Guard();
    auto& bucket = get_bucket(get_lock(mutex));
    auto bucket_lock = std::unique_lock(bucket.m_mutex);
    auto routine = notify(bucket, get_lock(mutex));
    leave(mutex, state);
    bucket_lock.unlock();
    if(routine) {
      routine->resume();
    }
    return _Thrd_result::_Success;
  }

  inline _Thrd_result __cdecl WindowsMutex::wait(
      _Cnd_t condition, _Mtx_t mutex) noexcept {
    if(is_entered() || !get_routine()) {
      return get_state().m_wait(condition, mutex);
    }
    wait(reinterpret_cast<PCONDITION_VARIABLE>(&condition->_Stl_cv._Win_cv),
      get_lock(mutex), INFINITE, 0, mutex);
    return _Thrd_result::_Success;
  }

  inline BOOL WINAPI WindowsMutex::sleep(PCONDITION_VARIABLE condition,
      PSRWLOCK mutex, DWORD timeout, ULONG flags) noexcept {
    return wait(condition, mutex, timeout, flags, nullptr);
  }

  inline BOOL WindowsMutex::wait(PCONDITION_VARIABLE condition,
      PSRWLOCK mutex, DWORD timeout, ULONG flags, _Mtx_t owner) noexcept {
    auto routine = static_cast<Routine*>(nullptr);
    if(owner) {
      routine = get_routine();
    }
    auto guard = Guard();
    auto& bucket = get_bucket(condition);
    auto waiter = ConditionWaiter(
      condition, routine, nullptr, NativeConditionVariable(), false);
    auto bucket_lock = std::unique_lock(bucket.m_mutex);
    if(routine) {
      routine->pending_suspend();
    }
    waiter.m_next = bucket.m_waiters;
    bucket.m_waiters = &waiter;
    bucket_lock.unlock();
    if(owner) {
      unlock(owner);
    } else {
      auto& mutex_bucket = get_bucket(mutex);
      auto mutex_lock = std::unique_lock(mutex_bucket.m_mutex);
      if((flags & CONDITION_VARIABLE_LOCKMODE_SHARED) != 0) {
        ReleaseSRWLockShared(mutex);
      } else {
        ReleaseSRWLockExclusive(mutex);
      }
      auto next = notify(mutex_bucket, mutex);
      mutex_lock.unlock();
      if(next) {
        next->resume();
      }
    }
    if(routine) {
      is_entered() = false;
      routine->suspend();
      auto result = lock(owner);
      if(result != _Thrd_result::_Success) {
        std::terminate();
      }
      is_entered() = true;
      return TRUE;
    }
    auto deadline = std::chrono::steady_clock::now() +
      std::chrono::milliseconds(timeout);
    auto error = DWORD(ERROR_SUCCESS);
    bucket_lock.lock();
    try {
      if(timeout == INFINITE) {
        waiter.m_notification.wait(bucket_lock, [&] {
          return waiter.m_is_notified;
        });
      } else if(!waiter.m_notification.wait_until(bucket_lock, deadline, [&] {
          return waiter.m_is_notified;
        })) {
        error = ERROR_TIMEOUT;
      }
    } catch(const std::system_error& exception) {
      error = exception.code().value();
    }
    if(!waiter.m_is_notified) {
      auto entry = &bucket.m_waiters;
      while(*entry != &waiter) {
        entry = &(*entry)->m_next;
      }
      *entry = waiter.m_next;
    } else {
      error = ERROR_SUCCESS;
    }
    bucket_lock.unlock();
    if((flags & CONDITION_VARIABLE_LOCKMODE_SHARED) != 0) {
      AcquireSRWLockShared(mutex);
    } else {
      AcquireSRWLockExclusive(mutex);
    }
    if(error != ERROR_SUCCESS) {
      SetLastError(error);
      return FALSE;
    }
    return TRUE;
  }

  inline void WindowsMutex::notify(
      PCONDITION_VARIABLE condition, bool all) noexcept {
    auto guard = Guard();
    auto& state = get_state();
    auto& bucket = get_bucket(condition);
    auto routines = static_cast<ConditionWaiter*>(nullptr);
    auto bucket_lock = std::unique_lock(bucket.m_mutex);
    auto entry = &bucket.m_waiters;
    while(*entry) {
      auto waiter = *entry;
      if(waiter->m_condition != condition) {
        entry = &waiter->m_next;
        continue;
      }
      *entry = waiter->m_next;
      waiter->m_is_notified = true;
      if(waiter->m_routine) {
        waiter->m_next = routines;
        routines = waiter;
      } else {
        waiter->m_notification.notify_one();
      }
      if(!all) {
        break;
      }
    }
    if(all) {
      state.m_notify_all(condition);
    } else {
      state.m_notify_one(condition);
    }
    bucket_lock.unlock();
    while(routines) {
      auto routine = routines->m_routine;
      routines = routines->m_next;
      routine->resume();
    }
  }

  inline void WINAPI WindowsMutex::notify_one(
      PCONDITION_VARIABLE condition) noexcept {
    notify(condition, false);
  }

  inline void WINAPI WindowsMutex::notify_all(
      PCONDITION_VARIABLE condition) noexcept {
    notify(condition, true);
  }

  inline BOOL WINAPI WindowsMutex::begin(
      PINIT_ONCE once, DWORD flags, PBOOL pending, PVOID* context) noexcept {
    auto& state = get_state();
    if(flags != 0 || is_entered()) {
      return state.m_begin(once, flags, pending, context);
    }
    if(state.m_begin(once, INIT_ONCE_CHECK_ONLY, pending, context)) {
      return TRUE;
    }
    auto routine = get_routine();
    auto guard = Guard();
    auto& bucket = get_bucket(once);
    auto bucket_lock = std::unique_lock(bucket.m_mutex);
    while(true) {
      if(!InterlockedCompareExchangePointer(&once->Ptr, nullptr, nullptr) ||
          state.m_begin(once, INIT_ONCE_CHECK_ONLY, pending, context) ||
          state.m_begin(once, INIT_ONCE_ASYNC, pending, context)) {
        return state.m_begin(once, flags, pending, context);
      }
      auto waiter = OnceWaiter(
        once, routine, bucket.m_waiters, NativeConditionVariable(), false);
      bucket.m_waiters = &waiter;
      if(routine) {
        routine->pending_suspend();
        bucket_lock.unlock();
        is_entered() = false;
        routine->suspend();
        is_entered() = true;
        bucket_lock.lock();
      } else {
        waiter.m_notification.wait(bucket_lock, [&] {
          return waiter.m_is_notified;
        });
      }
    }
  }

  inline BOOL WINAPI WindowsMutex::complete(
      PINIT_ONCE once, DWORD flags, PVOID context) noexcept {
    auto guard = Guard();
    auto& state = get_state();
    auto& bucket = get_bucket(once);
    auto routines = static_cast<OnceWaiter*>(nullptr);
    auto bucket_lock = std::unique_lock(bucket.m_mutex);
    auto result = state.m_complete(once, flags, context);
    auto error = GetLastError();
    if(result) {
      auto entry = &bucket.m_waiters;
      while(*entry) {
        auto waiter = *entry;
        if(waiter->m_once != once) {
          entry = &waiter->m_next;
          continue;
        }
        *entry = waiter->m_next;
        waiter->m_is_notified = true;
        if(waiter->m_routine) {
          waiter->m_next = routines;
          routines = waiter;
        } else {
          waiter->m_notification.notify_one();
        }
      }
    }
    bucket_lock.unlock();
    while(routines) {
      auto routine = routines->m_routine;
      routines = routines->m_next;
      routine->resume();
    }
    if(!result) {
      SetLastError(error);
    }
    return result;
  }

  inline void WindowsMutex::patch(HMODULE module) {
    auto base = reinterpret_cast<unsigned char*>(module);
    auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    auto image = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    auto& directory = image->OptionalHeader.
      DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if(directory.VirtualAddress == 0) {
      return;
    }
    auto& state = get_state();
    auto imports = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
      base + directory.VirtualAddress);
    for(auto descriptor = imports; descriptor->Name != 0; ++descriptor) {
      auto thunk = reinterpret_cast<IMAGE_THUNK_DATA*>(
        base + descriptor->FirstThunk);
      for(; thunk->u1.Function != 0; ++thunk) {
        auto target = reinterpret_cast<void*>(thunk->u1.Function);
        auto replacement = static_cast<void*>(nullptr);
        if(target == reinterpret_cast<void*>(state.m_lock)) {
          replacement = reinterpret_cast<void*>(
            static_cast<decltype(&_Mtx_lock)>(&lock));
        } else if(target == reinterpret_cast<void*>(state.m_try_lock)) {
          replacement = reinterpret_cast<void*>(&try_lock);
        } else if(target == reinterpret_cast<void*>(state.m_unlock)) {
          replacement = reinterpret_cast<void*>(&unlock);
        } else if(target == reinterpret_cast<void*>(state.m_wait)) {
          replacement = reinterpret_cast<void*>(
            static_cast<decltype(&_Cnd_wait)>(&wait));
        } else if(target == reinterpret_cast<void*>(state.m_sleep)) {
          replacement = reinterpret_cast<void*>(&sleep);
        } else if(target == reinterpret_cast<void*>(state.m_notify_one)) {
          replacement = reinterpret_cast<void*>(&notify_one);
        } else if(target == reinterpret_cast<void*>(state.m_notify_all)) {
          replacement = reinterpret_cast<void*>(&notify_all);
        } else if(target == reinterpret_cast<void*>(state.m_begin)) {
          replacement = reinterpret_cast<void*>(&begin);
        } else if(target == reinterpret_cast<void*>(state.m_complete)) {
          replacement = reinterpret_cast<void*>(&complete);
        }
        if(replacement) {
          auto protection = DWORD();
          if(!VirtualProtect(&thunk->u1.Function, sizeof(void*),
              PAGE_READWRITE, &protection)) {
            throw std::system_error(GetLastError(), std::system_category());
          }
          InterlockedExchangePointer(
            reinterpret_cast<void* volatile*>(&thunk->u1.Function),
            replacement);
          if(!VirtualProtect(&thunk->u1.Function, sizeof(void*),
              protection, &protection)) {
            throw std::system_error(GetLastError(), std::system_category());
          }
        }
      }
    }
  }
}
#endif
#endif
