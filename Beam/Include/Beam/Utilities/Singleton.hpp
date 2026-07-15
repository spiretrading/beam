#ifndef BEAM_SINGLETON_HPP
#define BEAM_SINGLETON_HPP
#include "Beam/Utilities/DllExport.hpp"

namespace Beam {

  /** Implements the Singleton pattern. */
  template<typename T>
  class BEAM_EXPORT_DLL Singleton {
    public:
      using Type = T;

      /** Returns the singleton instance. */
      static Type& get();

    protected:
      Singleton() = default;

    private:
      Singleton(const Singleton&) = delete;
      Singleton& operator =(const Singleton&) = delete;
  };

#ifndef BEAM_USE_DLL
  template<typename T>
  typename Singleton<T>::Type& Singleton<T>::get() {
    static auto singleton = Type();
    return singleton;
  }
#endif
}

#endif
