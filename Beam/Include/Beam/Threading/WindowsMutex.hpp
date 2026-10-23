#ifndef BEAM_WINDOWS_MUTEX_HPP
#define BEAM_WINDOWS_MUTEX_HPP
#if defined(_MSC_VER) && defined(_DLL)
#include <array>
#include <cstdint>
#include <mutex>
#include <system_error>
#include <Windows.h>
#include <TlHelp32.h>
#include "Beam/Utilities/DllExport.hpp"

namespace Beam {
  class Routine;
}

namespace Beam::Details {
  class BEAM_EXPORT_DLL WindowsMutex {
    public:

      /**
       * Enables routine-aware mutexes and untimed condition variable waits
       * in loaded modules using this MSVC CRT.
       * Must be called before dispatching routines.
       */
      static void initialize();

    private:
      struct Waiter {
        PSRWLOCK m_mutex;
        Routine* m_routine;
        Waiter* m_next;
      };
      struct Bucket {
        SRWLOCK m_mutex = SRWLOCK_INIT;
        Waiter* m_waiters = nullptr;
      };
      struct ConditionWaiter {
        PCONDITION_VARIABLE m_condition;
        Routine* m_routine;
        ConditionWaiter* m_next;
        CONDITION_VARIABLE m_notification;
        bool m_is_notified;
      };
      struct ConditionBucket {
        SRWLOCK m_mutex = SRWLOCK_INIT;
        ConditionWaiter* m_waiters = nullptr;
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
        std::array<Bucket, 64> m_buckets;
        std::array<ConditionBucket, 64> m_conditions;
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
      static Routine* pop(Bucket& bucket, PSRWLOCK mutex);
      static _Thrd_result acquire(_Mtx_t mutex, Routine& routine);
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
          !state.m_notify_all) {
        throw std::system_error(ERROR_PROC_NOT_FOUND, std::system_category());
      }
      auto module = HMODULE();
      if(!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
          GET_MODULE_HANDLE_EX_FLAG_PIN,
          reinterpret_cast<LPCSTR>(&lock), &module)) {
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

  inline Routine* WindowsMutex::pop(Bucket& bucket, PSRWLOCK mutex) {
    auto waiter = &bucket.m_waiters;
    while(*waiter) {
      if((*waiter)->m_mutex == mutex) {
        auto routine = (*waiter)->m_routine;
        *waiter = (*waiter)->m_next;
        return routine;
      }
      waiter = &(*waiter)->m_next;
    }
    return nullptr;
  }

  inline _Thrd_result WindowsMutex::acquire(
      _Mtx_t mutex, Routine& routine) {
    auto owner = mutex->_Critical_section._Unused;
    if(owner && owner != &routine) {
      return _Thrd_result::_Busy;
    }
    auto result = get_state().m_try_lock(mutex);
    if(result == _Thrd_result::_Success) {
      mutex->_Critical_section._Unused = &routine;
    }
    return result;
  }

  inline _Thrd_result __cdecl WindowsMutex::lock(_Mtx_t mutex) noexcept {
    if(is_entered()) {
      return get_state().m_lock(mutex);
    }
    auto routine = get_routine();
    if(!routine) {
      return get_state().m_lock(mutex);
    }
    auto guard = Guard();
    auto& bucket = get_bucket(get_lock(mutex));
    while(true) {
      AcquireSRWLockExclusive(&bucket.m_mutex);
      auto result = acquire(mutex, *routine);
      if(result != _Thrd_result::_Busy) {
        ReleaseSRWLockExclusive(&bucket.m_mutex);
        return result;
      }
      auto waiter = Waiter(get_lock(mutex), routine, bucket.m_waiters);
      bucket.m_waiters = &waiter;
      routine->pending_suspend();
      ReleaseSRWLockExclusive(&bucket.m_mutex);
      is_entered() = false;
      routine->suspend();
      is_entered() = true;
    }
  }

  inline _Thrd_result __cdecl WindowsMutex::try_lock(_Mtx_t mutex) noexcept {
    if(is_entered()) {
      return get_state().m_try_lock(mutex);
    }
    auto routine = get_routine();
    if(!routine) {
      return get_state().m_try_lock(mutex);
    }
    auto guard = Guard();
    auto& bucket = get_bucket(get_lock(mutex));
    AcquireSRWLockExclusive(&bucket.m_mutex);
    auto result = acquire(mutex, *routine);
    ReleaseSRWLockExclusive(&bucket.m_mutex);
    return result;
  }

  inline _Thrd_result __cdecl WindowsMutex::unlock(_Mtx_t mutex) noexcept {
    auto guard = Guard();
    auto& bucket = get_bucket(get_lock(mutex));
    auto routine = static_cast<Routine*>(nullptr);
    AcquireSRWLockExclusive(&bucket.m_mutex);
    auto is_released = mutex->_Count == 1;
    if(is_released) {
      mutex->_Critical_section._Unused = nullptr;
    }
    auto result = get_state().m_unlock(mutex);
    if(is_released) {
      routine = pop(bucket, get_lock(mutex));
    }
    ReleaseSRWLockExclusive(&bucket.m_mutex);
    if(routine) {
      routine->resume();
    }
    return result;
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
    auto& state = get_state();
    auto& bucket = get_bucket(condition);
    auto waiter = ConditionWaiter(
      condition, routine, nullptr, CONDITION_VARIABLE(), false);
    AcquireSRWLockExclusive(&bucket.m_mutex);
    if(routine) {
      routine->pending_suspend();
    }
    waiter.m_next = bucket.m_waiters;
    bucket.m_waiters = &waiter;
    ReleaseSRWLockExclusive(&bucket.m_mutex);
    auto& mutex_bucket = get_bucket(mutex);
    AcquireSRWLockExclusive(&mutex_bucket.m_mutex);
    if(owner) {
      owner->_Critical_section._Unused = nullptr;
      owner->_Thread_id = -1;
      --owner->_Count;
    }
    if((flags & CONDITION_VARIABLE_LOCKMODE_SHARED) != 0) {
      ReleaseSRWLockShared(mutex);
    } else {
      ReleaseSRWLockExclusive(mutex);
    }
    auto next = pop(mutex_bucket, mutex);
    ReleaseSRWLockExclusive(&mutex_bucket.m_mutex);
    if(next) {
      next->resume();
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
    auto started = GetTickCount64();
    auto remaining = timeout;
    auto error = DWORD(ERROR_SUCCESS);
    AcquireSRWLockExclusive(&bucket.m_mutex);
    while(!waiter.m_is_notified) {
      if(!state.m_sleep(
          &waiter.m_notification, &bucket.m_mutex, remaining, 0)) {
        error = GetLastError();
        if(!waiter.m_is_notified) {
          break;
        }
      }
      if(waiter.m_is_notified) {
        break;
      }
      if(timeout != INFINITE) {
        auto elapsed = GetTickCount64() - started;
        if(elapsed >= timeout) {
          error = ERROR_TIMEOUT;
          break;
        }
        remaining = timeout - static_cast<DWORD>(elapsed);
      }
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
    ReleaseSRWLockExclusive(&bucket.m_mutex);
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
    AcquireSRWLockExclusive(&bucket.m_mutex);
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
        state.m_notify_one(&waiter->m_notification);
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
    ReleaseSRWLockExclusive(&bucket.m_mutex);
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
          replacement = reinterpret_cast<void*>(&lock);
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
