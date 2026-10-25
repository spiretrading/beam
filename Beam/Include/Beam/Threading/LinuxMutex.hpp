#ifndef BEAM_LINUX_MUTEX_HPP
#define BEAM_LINUX_MUTEX_HPP
#if defined(__linux__) && defined(__x86_64__)
#include <features.h>
#if defined(__GLIBC__)
#include <array>
#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <system_error>
#include <dlfcn.h>
#include <elf.h>
#include <link.h>
#include <poll.h>
#include <pthread.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include "Beam/Utilities/DllExport.hpp"

namespace Beam {
  class Routine;
}

namespace Beam::Details {
  class BEAM_EXPORT_DLL LinuxMutex {
    public:

      /**
       * Enables routine-aware standard synchronization in loaded ELF modules
       * using x86-64 glibc 2.30+ and libstdc++. Requires dynamically linked
       * glibc and must precede routine dispatch.
       */
      static void initialize();

    private:
      static constexpr auto LOCKED = std::uintptr_t(1);
      static constexpr auto HAS_WAITERS = std::uintptr_t(2);
      struct Waiter {
        void* m_key;
        Routine* m_routine;
        Waiter* m_next;
        pthread_cond_t m_notification;
        std::atomic_bool m_is_notified;
        bool m_is_suspended;
        pid_t m_thread;
        int m_result;
        clockid_t m_clock;
        timespec m_deadline;
        bool m_is_timed;

        Waiter(void* key, Routine* routine);
        ~Waiter();
      };
      struct ActiveOnce {
        pthread_once_t* m_once;
        ActiveOnce* m_next;
      };
      struct alignas(64) Bucket {
        pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;
        Waiter* m_waiters = nullptr;
        ActiveOnce* m_initializers = nullptr;
      };
      template<typename T>
      static T load(const char* name);
      struct State {
        decltype(&pthread_mutex_lock) m_lock =
          load<decltype(m_lock)>("pthread_mutex_lock");
        decltype(&pthread_mutex_trylock) m_try_lock =
          load<decltype(m_try_lock)>("pthread_mutex_trylock");
        decltype(&pthread_mutex_unlock) m_unlock =
          load<decltype(m_unlock)>("pthread_mutex_unlock");
        decltype(&pthread_cond_wait) m_wait =
          load<decltype(m_wait)>("pthread_cond_wait");
        decltype(&pthread_cond_timedwait) m_timed_wait =
          load<decltype(m_timed_wait)>("pthread_cond_timedwait");
        decltype(&pthread_cond_clockwait) m_clock_wait =
          load<decltype(m_clock_wait)>("pthread_cond_clockwait");
        decltype(&pthread_cond_signal) m_notify_one =
          load<decltype(m_notify_one)>("pthread_cond_signal");
        decltype(&pthread_cond_broadcast) m_notify_all =
          load<decltype(m_notify_all)>("pthread_cond_broadcast");
        decltype(&pthread_once) m_once =
          load<decltype(m_once)>("pthread_once");
        std::array<Bucket, 64> m_mutexes;
        std::array<Bucket, 64> m_conditions;
        std::array<Bucket, 64> m_initializers;
        int m_event = -1;
        std::array<int, 2> m_timers = {-1, -1};
      };
      class Guard {
        public:
          Guard() noexcept;
          ~Guard();

        private:
          bool m_previous;
      };
      static State& get_state();
      static bool& is_entered();
      static pid_t get_thread_id();
      static Routine* get_routine();
      static bool is_supported(pthread_mutex_t* mutex);
      static Bucket& get_bucket(std::array<Bucket, 64>& buckets, void* key);
      static std::uintptr_t enter(pthread_mutex_t* mutex);
      static void leave(pthread_mutex_t* mutex, std::uintptr_t state);
      static void suspend(Waiter& waiter, Bucket& bucket);
      static Routine* notify(Bucket& bucket, void* key);
      static int acquire(pthread_mutex_t* mutex, Routine& routine);
      static int acquire(pthread_mutex_t* mutex, Routine* routine,
        std::uintptr_t& state, bool selected);
      static int lock(pthread_mutex_t* mutex, Routine* routine);
      static int lock(pthread_mutex_t* mutex) noexcept;
      static int try_lock(pthread_mutex_t* mutex) noexcept;
      static int unlock(pthread_mutex_t* mutex) noexcept;
      static int wait(pthread_cond_t* condition, pthread_mutex_t* mutex);
      static int timed_wait(pthread_cond_t* condition, pthread_mutex_t* mutex,
        const timespec* deadline);
      static int clock_wait(pthread_cond_t* condition, pthread_mutex_t* mutex,
        clockid_t clock, const timespec* deadline);
      static int wait(pthread_cond_t* condition, pthread_mutex_t* mutex,
        clockid_t clock, const timespec* deadline);
      static int notify(pthread_cond_t* condition, bool all) noexcept;
      static int notify_one(pthread_cond_t* condition) noexcept;
      static int notify_all(pthread_cond_t* condition) noexcept;
      static int once(pthread_once_t* flag, void (*initialize)());
      static void complete(ActiveOnce& active, Bucket& bucket);
      static void* run_timers(void*);
      static bool precedes(const timespec& left, const timespec& right);
      static void patch(const dl_phdr_info& module);
      static void patch(const dl_phdr_info& module, void** address,
        void* replacement);
  };
}

#include "Beam/Routines/Routine.hpp"

namespace Beam::Details {
  inline void LinuxMutex::initialize() {
    static auto initialized = [] {
      auto guard = Guard();
      auto is_dynamic = false;
      dl_iterate_phdr([] (auto module, auto, auto context) {
        for(auto i = 0; i != module->dlpi_phnum; ++i) {
          if(module->dlpi_phdr[i].p_type == PT_INTERP) {
            *static_cast<bool*>(context) = true;
          }
        }
        return 1;
      }, &is_dynamic);
      if(!is_dynamic) {
        throw std::system_error(ENOTSUP, std::generic_category(),
          "Beam synchronization requires dynamically linked glibc.");
      }
      get_state();
      auto error = std::exception_ptr();
      dl_iterate_phdr([] (auto module, auto, auto context) {
        try {
          patch(*module);
          return 0;
        } catch(...) {
          *static_cast<std::exception_ptr*>(context) =
            std::current_exception();
          return 1;
        }
      }, &error);
      if(error) {
        std::rethrow_exception(error);
      }
      return true;
    }();
    static_cast<void>(initialized);
  }

  inline LinuxMutex::Waiter::Waiter(void* key, Routine* routine)
    : m_key(key),
      m_routine(routine),
      m_next(nullptr),
      m_notification(),
      m_is_notified(false),
      m_is_suspended(false),
      m_thread(get_thread_id()),
      m_result(0),
      m_clock(CLOCK_REALTIME),
      m_deadline(),
      m_is_timed(false) {}

  inline LinuxMutex::Waiter::~Waiter() {
    pthread_cond_destroy(&m_notification);
  }

  template<typename T>
  inline T LinuxMutex::load(const char* name) {
    static auto library = dlopen("libc.so.6", RTLD_NOW | RTLD_LOCAL);
    if(!library) {
      throw std::system_error(ENOENT, std::generic_category(), "libc.so.6");
    }
    auto function = reinterpret_cast<T>(dlsym(library, name));
    if(!function) {
      throw std::system_error(ENOSYS, std::generic_category(), name);
    }
    return function;
  }

  inline LinuxMutex::Guard::Guard() noexcept
      : m_previous(is_entered()) {
    is_entered() = true;
  }

  inline LinuxMutex::Guard::~Guard() {
    is_entered() = m_previous;
  }

  inline LinuxMutex::State& LinuxMutex::get_state() {
    static auto state = new State();
    return *state;
  }

  inline bool& LinuxMutex::is_entered() {
    static thread_local auto entered = false;
    return entered;
  }

  inline pid_t LinuxMutex::get_thread_id() {
    static thread_local auto id = gettid();
    return id;
  }

  inline Routine* LinuxMutex::get_routine() {
    if(is_entered() || !CurrentRoutineGlobal::is_scheduled()) {
      return nullptr;
    }
    auto routine = CurrentRoutineGlobal::get();
    if(!routine || routine->get_state() != Routine::State::RUNNING) {
      return nullptr;
    }
    return routine;
  }

  inline bool LinuxMutex::is_supported(pthread_mutex_t* mutex) {
    return mutex->__data.__kind == PTHREAD_MUTEX_NORMAL ||
      mutex->__data.__kind == PTHREAD_MUTEX_RECURSIVE;
  }

  inline LinuxMutex::Bucket& LinuxMutex::get_bucket(
      std::array<Bucket, 64>& buckets, void* key) {
    return buckets[(reinterpret_cast<std::uintptr_t>(key) >> 3) %
      buckets.size()];
  }

  inline std::uintptr_t LinuxMutex::enter(pthread_mutex_t* mutex) {
    static_assert(std::atomic_ref<__pthread_list_t*>::is_always_lock_free);
    static_assert(alignof(Routine) > (LOCKED | HAS_WAITERS));
    auto state = std::atomic_ref(mutex->__data.__list.__prev);
    auto value = state.load(std::memory_order_relaxed);
    while(true) {
      auto bits = reinterpret_cast<std::uintptr_t>(value);
      if((bits & LOCKED) == 0 && state.compare_exchange_weak(value,
          reinterpret_cast<__pthread_list_t*>(bits | LOCKED),
          std::memory_order_acquire, std::memory_order_relaxed)) {
        return bits;
      }
      __builtin_ia32_pause();
      value = state.load(std::memory_order_relaxed);
    }
  }

  inline void LinuxMutex::leave(pthread_mutex_t* mutex, std::uintptr_t state) {
    std::atomic_ref(mutex->__data.__list.__prev).store(
      reinterpret_cast<__pthread_list_t*>(state), std::memory_order_release);
  }

  inline void LinuxMutex::suspend(Waiter& waiter, Bucket& bucket) {
    auto& state = get_state();
    waiter.m_is_suspended = true;
    if(waiter.m_routine) {
      waiter.m_routine->pending_suspend();
      state.m_unlock(&bucket.m_mutex);
      is_entered() = false;
      waiter.m_routine->suspend();
      is_entered() = true;
      state.m_lock(&bucket.m_mutex);
    } else {
      while(!waiter.m_is_notified.load(std::memory_order_relaxed)) {
        state.m_wait(&waiter.m_notification, &bucket.m_mutex);
      }
    }
  }

  inline Routine* LinuxMutex::notify(Bucket& bucket, void* key) {
    for(auto waiter = bucket.m_waiters; waiter; waiter = waiter->m_next) {
      if(waiter->m_key == key) {
        if(!waiter->m_is_notified.load(std::memory_order_relaxed)) {
          waiter->m_is_notified.store(true, std::memory_order_release);
          if(waiter->m_is_suspended) {
            if(waiter->m_routine) {
              return waiter->m_routine;
            }
            get_state().m_notify_one(&waiter->m_notification);
          }
        }
        break;
      }
    }
    return nullptr;
  }

  inline int LinuxMutex::acquire(pthread_mutex_t* mutex, Routine& routine) {
    auto state = std::atomic_ref(mutex->__data.__list.__prev);
    auto owner = reinterpret_cast<std::uintptr_t>(&routine);
    if(mutex->__data.__kind == PTHREAD_MUTEX_RECURSIVE &&
        (reinterpret_cast<std::uintptr_t>(
          state.load(std::memory_order_relaxed)) &
          ~(LOCKED | HAS_WAITERS)) == owner) {
      return get_state().m_try_lock(mutex);
    }
    auto expected = static_cast<__pthread_list_t*>(nullptr);
    if(!state.compare_exchange_strong(expected,
        reinterpret_cast<__pthread_list_t*>(owner | LOCKED),
        std::memory_order_acquire, std::memory_order_relaxed)) {
      return EBUSY;
    }
    auto result = get_state().m_try_lock(mutex);
    if(result != 0) {
      leave(mutex, 0);
      return result;
    }
    leave(mutex, owner);
    return 0;
  }

  inline int LinuxMutex::acquire(pthread_mutex_t* mutex, Routine* routine,
      std::uintptr_t& state, bool selected) {
    auto owner = reinterpret_cast<Routine*>(state & ~HAS_WAITERS);
    if(owner && owner != routine) {
      return EBUSY;
    }
    if((state & HAS_WAITERS) != 0 && !selected) {
      if(owner != routine ||
          mutex->__data.__kind != PTHREAD_MUTEX_RECURSIVE ||
          mutex->__data.__owner != get_thread_id()) {
        return EBUSY;
      }
    }
    auto result = get_state().m_try_lock(mutex);
    if(result == 0) {
      state = reinterpret_cast<std::uintptr_t>(routine) |
        (state & HAS_WAITERS);
    }
    return result;
  }

  inline int LinuxMutex::lock(pthread_mutex_t* mutex, Routine* routine) {
    auto waiter = Waiter(mutex, routine);
    auto guard = Guard();
    auto& runtime = get_state();
    auto& bucket = get_bucket(runtime.m_mutexes, mutex);
    auto state = enter(mutex);
    runtime.m_lock(&bucket.m_mutex);
    auto result = acquire(mutex, routine, state, false);
    if(result != EBUSY) {
      leave(mutex, state);
      runtime.m_unlock(&bucket.m_mutex);
      return result;
    }
    auto entry = &bucket.m_waiters;
    auto should_spin = mutex->__data.__owner != waiter.m_thread;
    while(*entry) {
      if((*entry)->m_key == mutex &&
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
      if(should_spin) {
        runtime.m_unlock(&bucket.m_mutex);
        for(auto i = 0; i != 4096; ++i) {
          if(waiter.m_is_notified.load(std::memory_order_acquire)) {
            break;
          }
          __builtin_ia32_pause();
        }
        runtime.m_lock(&bucket.m_mutex);
      }
      if(!waiter.m_is_notified.load(std::memory_order_relaxed)) {
        suspend(waiter, bucket);
      }
      runtime.m_unlock(&bucket.m_mutex);
      state = enter(mutex);
      runtime.m_lock(&bucket.m_mutex);
      result = acquire(mutex, routine, state, true);
      if(result == EBUSY) {
        continue;
      }
      entry = &bucket.m_waiters;
      auto has_waiters = false;
      while(*entry) {
        if(*entry == &waiter) {
          *entry = waiter.m_next;
        } else {
          has_waiters |= (*entry)->m_key == mutex;
          entry = &(*entry)->m_next;
        }
      }
      if(!has_waiters) {
        state &= ~HAS_WAITERS;
      }
      leave(mutex, state);
      runtime.m_unlock(&bucket.m_mutex);
      return result;
    }
  }

  inline int LinuxMutex::lock(pthread_mutex_t* mutex) noexcept {
    auto& runtime = get_state();
    if(!is_supported(mutex)) {
      return runtime.m_lock(mutex);
    }
    auto routine = get_routine();
    if(!routine) {
      return runtime.m_lock(mutex);
    }
    auto result = acquire(mutex, *routine);
    if(result != EBUSY) {
      return result;
    }
    return lock(mutex, routine);
  }

  inline int LinuxMutex::try_lock(pthread_mutex_t* mutex) noexcept {
    auto& runtime = get_state();
    if(!is_supported(mutex)) {
      return runtime.m_try_lock(mutex);
    }
    auto routine = get_routine();
    if(!routine) {
      return runtime.m_try_lock(mutex);
    }
    return acquire(mutex, *routine);
  }

  inline int LinuxMutex::unlock(pthread_mutex_t* mutex) noexcept {
    auto& runtime = get_state();
    if(!is_supported(mutex) || mutex->__data.__count > 1) {
      return runtime.m_unlock(mutex);
    }
    auto state = enter(mutex);
    auto result = runtime.m_unlock(mutex);
    if(result != 0) {
      leave(mutex, state);
      return result;
    }
    state &= HAS_WAITERS;
    if(state == 0) {
      leave(mutex, state);
      return 0;
    }
    auto guard = Guard();
    auto& bucket = get_bucket(runtime.m_mutexes, mutex);
    runtime.m_lock(&bucket.m_mutex);
    auto routine = notify(bucket, mutex);
    leave(mutex, state);
    runtime.m_unlock(&bucket.m_mutex);
    if(routine) {
      routine->resume();
    }
    return 0;
  }

  inline int LinuxMutex::wait(
      pthread_cond_t* condition, pthread_mutex_t* mutex) {
    if(!is_supported(mutex) || (condition->__data.__wrefs & 1) != 0) {
      return get_state().m_wait(condition, mutex);
    }
    return wait(condition, mutex, CLOCK_REALTIME, nullptr);
  }

  inline int LinuxMutex::timed_wait(pthread_cond_t* condition,
      pthread_mutex_t* mutex, const timespec* deadline) {
    if(!is_supported(mutex) || (condition->__data.__wrefs & 1) != 0) {
      return get_state().m_timed_wait(condition, mutex, deadline);
    }
    auto clock = clockid_t(CLOCK_REALTIME);
    if((condition->__data.__wrefs & 2) != 0) {
      clock = CLOCK_MONOTONIC;
    }
    return wait(condition, mutex, clock, deadline);
  }

  inline int LinuxMutex::clock_wait(pthread_cond_t* condition,
      pthread_mutex_t* mutex, clockid_t clock, const timespec* deadline) {
    if(!is_supported(mutex) || (condition->__data.__wrefs & 1) != 0) {
      return get_state().m_clock_wait(condition, mutex, clock, deadline);
    }
    return wait(condition, mutex, clock, deadline);
  }

  inline int LinuxMutex::wait(pthread_cond_t* condition,
      pthread_mutex_t* mutex, clockid_t clock, const timespec* deadline) {
    if(clock != CLOCK_REALTIME && clock != CLOCK_MONOTONIC) {
      return EINVAL;
    }
    if(deadline && (deadline->tv_nsec < 0 || deadline->tv_nsec >= 1000000000)) {
      return EINVAL;
    }
    auto routine = get_routine();
    auto guard = Guard();
    auto& runtime = get_state();
    if(routine && deadline) {
      static auto started = [] {
        auto& state = get_state();
        state.m_event = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
        state.m_timers[0] = timerfd_create(
          CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK);
        state.m_timers[1] = timerfd_create(
          CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
        if(state.m_event == -1 || state.m_timers[0] == -1 ||
            state.m_timers[1] == -1) {
          std::terminate();
        }
        auto thread = pthread_t();
        if(pthread_create(&thread, nullptr, &run_timers, nullptr) != 0) {
          std::terminate();
        }
        pthread_detach(thread);
        return true;
      }();
      static_cast<void>(started);
    }
    auto& bucket = get_bucket(runtime.m_conditions, condition);
    auto waiter = Waiter(condition, routine);
    waiter.m_clock = clock;
    if(deadline) {
      waiter.m_deadline = *deadline;
      waiter.m_is_timed = true;
    }
    runtime.m_lock(&bucket.m_mutex);
    waiter.m_next = bucket.m_waiters;
    bucket.m_waiters = &waiter;
    if(routine) {
      routine->pending_suspend();
    }
    runtime.m_unlock(&bucket.m_mutex);
    unlock(mutex);
    if(routine) {
      if(deadline) {
        auto value = std::uint64_t(1);
        if(write(runtime.m_event, &value, sizeof(value)) == -1 &&
            errno != EAGAIN) {
          std::terminate();
        }
      }
      is_entered() = false;
      routine->suspend();
      lock(mutex);
      is_entered() = true;
      return waiter.m_result;
    }
    runtime.m_lock(&bucket.m_mutex);
    while(!waiter.m_is_notified.load(std::memory_order_relaxed)) {
      auto result = 0;
      if(deadline) {
        result = runtime.m_clock_wait(
          &waiter.m_notification, &bucket.m_mutex, clock, deadline);
      } else {
        result = runtime.m_wait(&waiter.m_notification, &bucket.m_mutex);
      }
      if(result != 0 &&
          !waiter.m_is_notified.load(std::memory_order_relaxed)) {
        auto entry = &bucket.m_waiters;
        while(*entry != &waiter) {
          entry = &(*entry)->m_next;
        }
        *entry = waiter.m_next;
        waiter.m_result = result;
        break;
      }
    }
    runtime.m_unlock(&bucket.m_mutex);
    lock(mutex);
    return waiter.m_result;
  }

  inline int LinuxMutex::notify(pthread_cond_t* condition, bool all) noexcept {
    auto guard = Guard();
    auto& runtime = get_state();
    auto& bucket = get_bucket(runtime.m_conditions, condition);
    auto routines = static_cast<Waiter*>(nullptr);
    runtime.m_lock(&bucket.m_mutex);
    auto entry = &bucket.m_waiters;
    while(*entry) {
      auto waiter = *entry;
      if(waiter->m_key != condition) {
        entry = &waiter->m_next;
        continue;
      }
      *entry = waiter->m_next;
      waiter->m_is_notified.store(true, std::memory_order_relaxed);
      if(waiter->m_routine) {
        waiter->m_next = routines;
        routines = waiter;
      } else {
        runtime.m_notify_one(&waiter->m_notification);
      }
      if(!all) {
        break;
      }
    }
    auto result = 0;
    if(all) {
      result = runtime.m_notify_all(condition);
    } else {
      result = runtime.m_notify_one(condition);
    }
    runtime.m_unlock(&bucket.m_mutex);
    while(routines) {
      auto routine = routines->m_routine;
      routines = routines->m_next;
      routine->resume();
    }
    return result;
  }

  inline int LinuxMutex::notify_one(pthread_cond_t* condition) noexcept {
    return notify(condition, false);
  }

  inline int LinuxMutex::notify_all(pthread_cond_t* condition) noexcept {
    return notify(condition, true);
  }

  inline int LinuxMutex::once(pthread_once_t* flag, void (*initialize)()) {
    auto& runtime = get_state();
    if(is_entered()) {
      return runtime.m_once(flag, initialize);
    }
    if((std::atomic_ref(*flag).load(std::memory_order_acquire) & 2) != 0) {
      return 0;
    }
    auto callable = std::__once_callable;
    auto call = std::__once_call;
    auto routine = get_routine();
    auto guard = Guard();
    auto& bucket = get_bucket(runtime.m_initializers, flag);
    runtime.m_lock(&bucket.m_mutex);
    while(true) {
      auto active = bucket.m_initializers;
      while(active && active->m_once != flag) {
        active = active->m_next;
      }
      if(!active) {
        break;
      }
      auto waiter = Waiter(flag, routine);
      waiter.m_next = bucket.m_waiters;
      bucket.m_waiters = &waiter;
      suspend(waiter, bucket);
    }
    auto active = ActiveOnce(flag, bucket.m_initializers);
    bucket.m_initializers = &active;
    runtime.m_unlock(&bucket.m_mutex);
    std::__once_callable = callable;
    std::__once_call = call;
    is_entered() = false;
    try {
      auto result = runtime.m_once(flag, initialize);
      is_entered() = true;
      complete(active, bucket);
      return result;
    } catch(...) {
      is_entered() = true;
      complete(active, bucket);
      throw;
    }
  }

  inline void LinuxMutex::complete(ActiveOnce& active, Bucket& bucket) {
    auto& runtime = get_state();
    auto routines = static_cast<Waiter*>(nullptr);
    runtime.m_lock(&bucket.m_mutex);
    auto initializer = &bucket.m_initializers;
    while(*initializer != &active) {
      initializer = &(*initializer)->m_next;
    }
    *initializer = active.m_next;
    auto entry = &bucket.m_waiters;
    while(*entry) {
      auto waiter = *entry;
      if(waiter->m_key != active.m_once) {
        entry = &waiter->m_next;
        continue;
      }
      *entry = waiter->m_next;
      waiter->m_is_notified.store(true, std::memory_order_relaxed);
      if(waiter->m_routine) {
        waiter->m_next = routines;
        routines = waiter;
      } else {
        runtime.m_notify_one(&waiter->m_notification);
      }
    }
    runtime.m_unlock(&bucket.m_mutex);
    while(routines) {
      auto routine = routines->m_routine;
      routines = routines->m_next;
      routine->resume();
    }
  }

  inline bool LinuxMutex::precedes(
      const timespec& left, const timespec& right) {
    return left.tv_sec < right.tv_sec ||
      (left.tv_sec == right.tv_sec && left.tv_nsec < right.tv_nsec);
  }

  inline void* LinuxMutex::run_timers(void*) {
    auto guard = Guard();
    auto& runtime = get_state();
    auto descriptors = std::array<pollfd, 3>();
    descriptors[0] = pollfd(runtime.m_event, POLLIN, 0);
    descriptors[1] = pollfd(runtime.m_timers[0], POLLIN, 0);
    descriptors[2] = pollfd(runtime.m_timers[1], POLLIN, 0);
    while(true) {
      auto deadlines = std::array<itimerspec, 2>();
      auto pending = std::array<bool, 2>();
      for(auto& bucket : runtime.m_conditions) {
        auto routines = static_cast<Waiter*>(nullptr);
        runtime.m_lock(&bucket.m_mutex);
        auto entry = &bucket.m_waiters;
        while(*entry) {
          auto waiter = *entry;
          if(!waiter->m_routine || !waiter->m_is_timed) {
            entry = &waiter->m_next;
            continue;
          }
          auto now = timespec();
          clock_gettime(waiter->m_clock, &now);
          if(!precedes(now, waiter->m_deadline)) {
            *entry = waiter->m_next;
            waiter->m_result = ETIMEDOUT;
            waiter->m_next = routines;
            routines = waiter;
            continue;
          }
          auto index = std::size_t(0);
          if(waiter->m_clock == CLOCK_MONOTONIC) {
            index = 1;
          }
          if(!pending[index] ||
              precedes(waiter->m_deadline, deadlines[index].it_value)) {
            deadlines[index].it_value = waiter->m_deadline;
            pending[index] = true;
          }
          entry = &waiter->m_next;
        }
        runtime.m_unlock(&bucket.m_mutex);
        while(routines) {
          auto routine = routines->m_routine;
          routines = routines->m_next;
          routine->resume();
        }
      }
      for(auto i = std::size_t(0); i != deadlines.size(); ++i) {
        if(timerfd_settime(runtime.m_timers[i], TFD_TIMER_ABSTIME,
            &deadlines[i], nullptr) != 0) {
          std::terminate();
        }
      }
      auto result = poll(descriptors.data(), descriptors.size(), -1);
      if(result < 0 && errno != EINTR) {
        std::terminate();
      }
      for(auto& descriptor : descriptors) {
        if((descriptor.revents & POLLIN) != 0) {
          auto value = std::uint64_t();
          static_cast<void>(read(descriptor.fd, &value, sizeof(value)));
        }
      }
    }
  }

  inline void LinuxMutex::patch(const dl_phdr_info& module) {
    auto dynamic = static_cast<const ElfW(Dyn)*>(nullptr);
    auto implementation = reinterpret_cast<std::uintptr_t>(&initialize);
    for(auto i = 0; i != module.dlpi_phnum; ++i) {
      auto& segment = module.dlpi_phdr[i];
      if(segment.p_type == PT_DYNAMIC) {
        dynamic = reinterpret_cast<const ElfW(Dyn)*>(
          module.dlpi_addr + segment.p_vaddr);
      }
      if(segment.p_type == PT_LOAD && *module.dlpi_name &&
          implementation >= module.dlpi_addr + segment.p_vaddr &&
          implementation < module.dlpi_addr + segment.p_vaddr +
            segment.p_memsz) {
        if(!dlopen(module.dlpi_name, RTLD_NOW | RTLD_NOLOAD | RTLD_NODELETE)) {
          throw std::system_error(ENOENT, std::generic_category());
        }
      }
    }
    if(!dynamic || std::strstr(module.dlpi_name, "linux-vdso")) {
      return;
    }
    auto symbols = static_cast<const ElfW(Sym)*>(nullptr);
    auto strings = static_cast<const char*>(nullptr);
    auto relocations = static_cast<const ElfW(Rela)*>(nullptr);
    auto jumps = static_cast<const ElfW(Rela)*>(nullptr);
    auto size = std::size_t(0);
    auto jump_size = std::size_t(0);
    for(auto entry = dynamic; entry->d_tag != DT_NULL; ++entry) {
      switch(entry->d_tag) {
        case DT_SYMTAB:
          symbols = reinterpret_cast<const ElfW(Sym)*>(entry->d_un.d_ptr);
          break;
        case DT_STRTAB:
          strings = reinterpret_cast<const char*>(entry->d_un.d_ptr);
          break;
        case DT_RELA:
          relocations = reinterpret_cast<const ElfW(Rela)*>(entry->d_un.d_ptr);
          break;
        case DT_RELASZ:
          size = entry->d_un.d_val / sizeof(ElfW(Rela));
          break;
        case DT_JMPREL:
          jumps = reinterpret_cast<const ElfW(Rela)*>(entry->d_un.d_ptr);
          break;
        case DT_PLTRELSZ:
          jump_size = entry->d_un.d_val / sizeof(ElfW(Rela));
          break;
      }
    }
    if(!symbols || !strings) {
      return;
    }
    auto apply = [&] (const auto& relocation) {
      auto type = ELF64_R_TYPE(relocation.r_info);
      if(type != R_X86_64_JUMP_SLOT && type != R_X86_64_GLOB_DAT &&
          type != R_X86_64_64) {
        return;
      }
      if(relocation.r_addend != 0) {
        return;
      }
      auto name = strings + symbols[ELF64_R_SYM(relocation.r_info)].st_name;
      auto replacement = static_cast<void*>(nullptr);
      if(std::strcmp(name, "pthread_mutex_lock") == 0) {
        replacement = reinterpret_cast<void*>(
          static_cast<decltype(&pthread_mutex_lock)>(&lock));
      } else if(std::strcmp(name, "pthread_mutex_trylock") == 0) {
        replacement = reinterpret_cast<void*>(&try_lock);
      } else if(std::strcmp(name, "pthread_mutex_unlock") == 0) {
        replacement = reinterpret_cast<void*>(&unlock);
      } else if(std::strcmp(name, "pthread_cond_wait") == 0) {
        replacement = reinterpret_cast<void*>(
          static_cast<decltype(&pthread_cond_wait)>(&wait));
      } else if(std::strcmp(name, "pthread_cond_timedwait") == 0) {
        replacement = reinterpret_cast<void*>(&timed_wait);
      } else if(std::strcmp(name, "pthread_cond_clockwait") == 0) {
        replacement = reinterpret_cast<void*>(&clock_wait);
      } else if(std::strcmp(name, "pthread_cond_signal") == 0) {
        replacement = reinterpret_cast<void*>(&notify_one);
      } else if(std::strcmp(name, "pthread_cond_broadcast") == 0) {
        replacement = reinterpret_cast<void*>(&notify_all);
      } else if(std::strcmp(name, "pthread_once") == 0) {
        replacement = reinterpret_cast<void*>(&once);
      }
      if(replacement) {
        patch(module, reinterpret_cast<void**>(
          module.dlpi_addr + relocation.r_offset), replacement);
      }
    };
    for(auto i = std::size_t(0); relocations && i != size; ++i) {
      apply(relocations[i]);
    }
    for(auto i = std::size_t(0); jumps && i != jump_size; ++i) {
      apply(jumps[i]);
    }
  }

  inline void LinuxMutex::patch(const dl_phdr_info& module, void** address,
      void* replacement) {
    auto size = static_cast<std::uintptr_t>(sysconf(_SC_PAGESIZE));
    auto value = reinterpret_cast<std::uintptr_t>(address);
    auto page = value & ~(size - 1);
    auto protection = 0;
    for(auto i = 0; i != module.dlpi_phnum; ++i) {
      auto& segment = module.dlpi_phdr[i];
      auto start = module.dlpi_addr + segment.p_vaddr;
      if(segment.p_type == PT_LOAD && value >= start &&
          value < start + segment.p_memsz) {
        if((segment.p_flags & PF_R) != 0) {
          protection |= PROT_READ;
        }
        if((segment.p_flags & PF_W) != 0) {
          protection |= PROT_WRITE;
        }
        if((segment.p_flags & PF_X) != 0) {
          protection |= PROT_EXEC;
        }
      }
    }
    for(auto i = 0; i != module.dlpi_phnum; ++i) {
      auto& segment = module.dlpi_phdr[i];
      if(segment.p_type == PT_GNU_RELRO) {
        auto start = (module.dlpi_addr + segment.p_vaddr) & ~(size - 1);
        auto end = (module.dlpi_addr + segment.p_vaddr + segment.p_memsz) &
          ~(size - 1);
        if(page >= start && page < end) {
          protection &= ~PROT_WRITE;
        }
      }
    }
    if(mprotect(reinterpret_cast<void*>(page), size,
        protection | PROT_WRITE) != 0) {
      throw std::system_error(errno, std::generic_category());
    }
    std::atomic_ref(*address).store(replacement, std::memory_order_release);
    if(mprotect(reinterpret_cast<void*>(page), size, protection) != 0) {
      throw std::system_error(errno, std::generic_category());
    }
  }
}
#endif
#endif
#endif
