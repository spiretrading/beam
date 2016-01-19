#ifndef BEAM_SINGLETON_HPP
#define BEAM_SINGLETON_HPP
#include <boost/noncopyable.hpp>

namespace Beam {

  /*! \class Singleton
      \brief Implements the Singleton pattern.
   */
  template<typename T>
  class Singleton : private boost::noncopyable {
    public:

      //! Returns the singleton instance.
      static T& GetInstance();
  };

  template<typename T>
  T& Singleton<T>::GetInstance() {
    static T singleton;
    return singleton;
  }
}

#endif
