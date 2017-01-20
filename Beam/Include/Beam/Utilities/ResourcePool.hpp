#ifndef BEAM_RESOURCEPOOL_HPP
#define BEAM_RESOURCEPOOL_HPP
#include <algorithm>
#include <deque>
#include <functional>
#include <limits>
#include <memory>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/locks.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/TimedConditionVariable.hpp"
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class ScopedResource
      \brief Stores an object acquired from a ResourcePool.
      \tparam T The type of object stored.
   */
  template<typename T>
  class ScopedResource : private boost::noncopyable {
    public:

      //! The type of object stored.
      using Type = T;

      //! Constructs a ScopedResource.
      ScopedResource(RefType<ResourcePool<Type>> pool,
        std::unique_ptr<Type> object);

      //! Acquires a ScopedResource.
      /*!
        \param object The ScopedResource to acquire.
      */
      ScopedResource(ScopedResource&& object);

      ~ScopedResource();

      //! Returns a reference to the object.
      Type& operator *() const;

      //! Returns a pointer to the object.
      Type* operator ->() const;

    private:
      ResourcePool<T>* m_pool;
      std::unique_ptr<Type> m_object;
  };

  /*! \class ResourcePool
      \brief A resource pool.
      \tparam T The type of object to pool.
   */
  template<typename T>
  class ResourcePool : private boost::noncopyable {
    public:

      //! The type of object to pool.
      using Type = T;

      //! The type of function used to build objects.
      using ObjectBuilder = std::function<std::unique_ptr<Type> ()>;

      //! Constructs a ResourcePool.
      /*!
        \param timeout The amount of time to wait for an object before
               constructing a new one.
        \param objectBuilder The function used to build objects.
        \param timerThreadPool The thread pool used for timed operations.
        \param minObjectCount The minimum number of objects to build.
        \param maxObjectCount The maximum number of objects to build.
      */
      ResourcePool(const boost::posix_time::time_duration& timeout,
        const ObjectBuilder& objectBuilder,
        RefType<Threading::TimerThreadPool> timerThreadPool,
        std::size_t minObjectCount = 1,
        std::size_t maxObjectCount = std::numeric_limits<std::size_t>::max());

      //! Resets the pool.
      void Reset();

      //! Acquires the next resource.
      ScopedResource<Type> Acquire();

    private:
      template<typename> friend class ScopedResource;
      Threading::Mutex m_mutex;
      boost::posix_time::time_duration m_timeout;
      ObjectBuilder m_objectBuilder;
      std::size_t m_minObjectCount;
      std::size_t m_maxObjectCount;
      std::size_t m_currentObjectCount;
      std::deque<std::unique_ptr<Type>> m_objects;
      Threading::TimedConditionVariable m_objectAvailableCondition;

      void Add(std::unique_ptr<Type> object);
  };

  template<typename T>
  ResourcePool<T>::ResourcePool(const boost::posix_time::time_duration& timeout,
      const ObjectBuilder& objectBuilder,
      RefType<Threading::TimerThreadPool> timerThreadPool,
      std::size_t minObjectCount, std::size_t maxObjectCount)
      : m_timeout{timeout},
        m_objectBuilder{objectBuilder},
        m_minObjectCount{std::max<std::size_t>(1, minObjectCount)},
        m_maxObjectCount{std::max(m_minObjectCount, maxObjectCount)},
        m_currentObjectCount{m_minObjectCount},
        m_objectAvailableCondition{Ref(timerThreadPool)} {
    for(std::size_t i = 0; i < m_currentObjectCount; ++i) {
      m_objects.push_back(m_objectBuilder());
    }
  }

  template<typename T>
  void ResourcePool<T>::Reset() {
    boost::unique_lock<Threading::Mutex> lock{m_mutex};
    while(m_objects.size() != m_currentObjectCount) {
      m_objectAvailableCondition.wait(lock);
    }
    m_objects.clear();
    m_currentObjectCount = m_minObjectCount;
    for(std::size_t i = 0; i < m_currentObjectCount; ++i) {
      m_objects.push_back(m_objectBuilder());
    }
  }

  template<typename T>
  ScopedResource<typename ResourcePool<T>::Type> ResourcePool<T>::Acquire() {
    boost::unique_lock<Threading::Mutex> lock{m_mutex};
    auto unconditionalWait = false;
    while(m_objects.empty()) {
      if(unconditionalWait || m_currentObjectCount >= m_maxObjectCount) {
        m_objectAvailableCondition.wait(lock);
      } else {
        try {
          m_objectAvailableCondition.timed_wait(m_timeout, lock);
        } catch(const Threading::TimeoutException&) {
          if(m_objects.empty() && m_currentObjectCount < m_maxObjectCount) {
            try {
              auto object = m_objectBuilder();
              ScopedResource<Type> scopedObject{Ref(*this), std::move(object)};
              ++m_currentObjectCount;
              return scopedObject;
            } catch(const std::exception&) {
              unconditionalWait = true;
            }
          }
        }
      }
    }
    ScopedResource<Type> object{Ref(*this), std::move(m_objects.front())};
    m_objects.pop_front();
    return object;
  }

  template<typename T>
  void ResourcePool<T>::Add(std::unique_ptr<Type> object) {
    boost::unique_lock<Threading::Mutex> lock{m_mutex};
    m_objects.push_back(std::move(object));
    m_objectAvailableCondition.notify_one();
  }

  template<typename T>
  ScopedResource<T>::ScopedResource(RefType<ResourcePool<Type>> pool,
      std::unique_ptr<Type> object)
      : m_pool{pool.Get()},
        m_object{std::move(object)} {}

  template<typename T>
  ScopedResource<T>::ScopedResource(ScopedResource<T>&& object)
      : m_pool{std::move(object.m_pool)},
        m_object{std::move(object.m_object)} {}

  template<typename T>
  ScopedResource<T>::~ScopedResource() {
    if(m_object != nullptr) {
      m_pool->Add(std::move(m_object));
    }
  }

  template<typename T>
  typename ScopedResource<T>::Type& ScopedResource<T>::operator *() const {
    return *m_object;
  }

  template<typename T>
  typename ScopedResource<T>::Type* ScopedResource<T>::operator ->() const {
    return m_object.get();
  }
}

#endif
