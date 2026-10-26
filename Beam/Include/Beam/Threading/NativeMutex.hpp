#ifndef BEAM_NATIVE_MUTEX_HPP
#define BEAM_NATIVE_MUTEX_HPP
#include <cerrno>
#include <system_error>
#ifdef _WIN32
  #include <Windows.h>
#else
  #include <pthread.h>
  #if defined(__linux__) && defined(__GLIBC__) && defined(__x86_64__)
    #include <dlfcn.h>
  #endif
#endif

namespace Beam {

  /**
   * A non-recursive mutex that blocks the calling OS thread on contention.
   * Bypasses Beam's synchronization hooks, including when used by a routine.
   */
  class NativeMutex {
    public:

      /** Constructs an unlocked mutex. */
      #ifdef _WIN32
        NativeMutex() noexcept;
      #else
        NativeMutex();
      #endif

      ~NativeMutex();

      /** Acquires the mutex, blocking the calling OS thread if necessary. */
      void lock();

      /** Returns whether the mutex was acquired without blocking. */
      bool try_lock();

      /** Releases the mutex owned by the calling OS thread. */
      void unlock();

    private:
      friend class NativeConditionVariable;
      #ifdef _WIN32
        SRWLOCK m_mutex;
      #else
        struct Functions {
          #if defined(__linux__) && defined(__GLIBC__) && defined(__x86_64__)
            decltype(&pthread_mutex_lock) m_lock =
              load<decltype(m_lock)>("pthread_mutex_lock");
            decltype(&pthread_mutex_trylock) m_try_lock =
              load<decltype(m_try_lock)>("pthread_mutex_trylock");
            decltype(&pthread_mutex_unlock) m_unlock =
              load<decltype(m_unlock)>("pthread_mutex_unlock");
          #else
            decltype(&pthread_mutex_lock) m_lock = &pthread_mutex_lock;
            decltype(&pthread_mutex_trylock) m_try_lock =
              &pthread_mutex_trylock;
            decltype(&pthread_mutex_unlock) m_unlock = &pthread_mutex_unlock;
          #endif
        };
        pthread_mutex_t m_mutex;

        static const Functions& get_functions();
      #endif

      template<typename T>
      static T load(const char* name);
      NativeMutex(const NativeMutex&) = delete;
      NativeMutex& operator =(const NativeMutex&) = delete;
  };

  #ifdef _WIN32
    inline NativeMutex::NativeMutex() noexcept
      : m_mutex() {}
  #else
    inline NativeMutex::NativeMutex()
        : m_mutex() {
      get_functions();
      auto result = pthread_mutex_init(&m_mutex, nullptr);
      if(result != 0) {
        throw std::system_error(result, std::generic_category());
      }
    }
  #endif

  inline NativeMutex::~NativeMutex() {
    #ifndef _WIN32
      pthread_mutex_destroy(&m_mutex);
    #endif
  }

  inline void NativeMutex::lock() {
    #ifdef _WIN32
      AcquireSRWLockExclusive(&m_mutex);
    #else
      auto result = get_functions().m_lock(&m_mutex);
      if(result != 0) {
        throw std::system_error(result, std::generic_category());
      }
    #endif
  }

  inline bool NativeMutex::try_lock() {
    #ifdef _WIN32
      return TryAcquireSRWLockExclusive(&m_mutex) != 0;
    #else
      auto result = get_functions().m_try_lock(&m_mutex);
      if(result == EBUSY) {
        return false;
      }
      if(result != 0) {
        throw std::system_error(result, std::generic_category());
      }
      return true;
    #endif
  }

  inline void NativeMutex::unlock() {
    #ifdef _WIN32
      ReleaseSRWLockExclusive(&m_mutex);
    #else
      auto result = get_functions().m_unlock(&m_mutex);
      if(result != 0) {
        throw std::system_error(result, std::generic_category());
      }
    #endif
  }

  #ifndef _WIN32
    inline const NativeMutex::Functions& NativeMutex::get_functions() {
      static const auto functions = Functions();
      return functions;
    }
  #endif

  #if defined(_WIN32) || (defined(__linux__) && defined(__GLIBC__) && \
      defined(__x86_64__))
    template<typename T>
    T NativeMutex::load(const char* name) {
      #ifdef _WIN32
        auto function = reinterpret_cast<T>(
          GetProcAddress(GetModuleHandleW(L"kernel32.dll"), name));
        if(!function) {
          throw std::system_error(GetLastError(), std::system_category(), name);
        }
        return function;
      #else
        static auto library = dlopen("libc.so.6", RTLD_NOW | RTLD_LOCAL);
        if(!library) {
          throw std::system_error(ENOENT, std::generic_category(), "libc.so.6");
        }
        auto function = reinterpret_cast<T>(dlsym(library, name));
        if(!function) {
          throw std::system_error(ENOSYS, std::generic_category(), name);
        }
        return function;
      #endif
    }
  #endif
}

#endif
