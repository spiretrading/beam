#ifndef BEAM_RESOURCE_POOL_HPP
#define BEAM_RESOURCE_POOL_HPP
#include <algorithm>
#include <condition_variable>
#include <deque>
#include <limits>
#include <memory>
#include <mutex>
#include <utility>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /**
   * Stores an object acquired from a ResourcePool.
   * @param <T> The type of object stored.
   * @param <B> The type of function used to construct the object.
   */
  template<typename T, typename B>
  class ScopedResource {
    public:

      /** The type of object stored. */
      using Type = T;

      /** The type of builder used to construct the object. */
      using Make = B;

      /**
       * Constructs a ScopedResource.
       * @param pool The ResourcePool the object belongs to.
       * @parma object The pooled object to manage.
       */
      ScopedResource(Ref<ResourcePool<Type, Make>> pool,
        std::unique_ptr<Type> object);

      /**
       * Acquires a ScopedResource.
       * @param object The ScopedResource to acquire.
       */
      ScopedResource(ScopedResource&& object);

      ~ScopedResource();

      /** Returns a reference to the object. */
      Type& operator *() const;

      /** Returns a pointer to the object. */
      Type* operator ->() const;

    private:
      ResourcePool<Type, Make>* m_pool;
      std::unique_ptr<Type> m_object;

      ScopedResource(const ScopedResource&) = delete;
      ScopedResource& operator =(const ScopedResource&) = delete;
  };

  /**
   * A resource pool.
   * @param <T> The type of object to pool.
   * @param <B> The type of function used to construct pooled objects.
   */
  template<typename T, typename B>
  class ResourcePool {
    public:

      /** The type of object to pool. */
      using Type = T;

      /** The type of function used to build objects. */
      using Make = B;

      /**
       * Constructs a ResourcePool.
       * @param timeout The amount of time to wait for an object before
       *        constructing a new one.
       * @param builder The function used to build objects.
       * @param minObjectCount The minimum number of objects to build.
       * @param maxObjectCount The maximum number of objects to build.
       */
      template<typename BF>
      ResourcePool(boost::posix_time::time_duration timeout,
        BF&& objectBuilder, std::size_t minObjectCount = 1,
        std::size_t maxObjectCount = std::numeric_limits<std::size_t>::max());

      /**
       * Destroys all existing objects and rebuilds the minimum number of
       * objects.
       */
      void Reset();

      /** Acquires the next resource. */
      ScopedResource<Type, Make> Acquire();

      /** Returns a resource if one is available without blocking. */
      boost::optional<ScopedResource<Type, Make>> TryAcquire();

    private:
      template<typename, typename> friend class ScopedResource;
      std::mutex m_mutex;
      boost::posix_time::time_duration m_timeout;
      Make m_builder;
      std::size_t m_minObjectCount;
      std::size_t m_maxObjectCount;
      std::size_t m_currentObjectCount;
      std::deque<std::unique_ptr<Type>> m_objects;
      std::condition_variable m_objectAvailableCondition;

      ResourcePool(const ResourcePool&) = delete;
      ResourcePool& operator =(const ResourcePool&) = delete;
      void Add(std::unique_ptr<Type> object);
  };

  template<typename T, typename B>
  template<typename BF>
  ResourcePool<T, B>::ResourcePool(boost::posix_time::time_duration timeout,
      BF&& builder, std::size_t minObjectCount, std::size_t maxObjectCount)
      : m_timeout(timeout),
        m_builder(std::forward<BF>(builder)),
        m_minObjectCount(std::max<std::size_t>(1, minObjectCount)),
        m_maxObjectCount(std::max(m_minObjectCount, maxObjectCount)),
        m_currentObjectCount(m_minObjectCount) {
    for(auto i = std::size_t(0); i < m_currentObjectCount; ++i) {
      m_objects.push_back(m_builder());
    }
  }

  template<typename T, typename B>
  void ResourcePool<T, B>::Reset() {
    auto lock = std::unique_lock(m_mutex);
    while(m_objects.size() != m_currentObjectCount) {
      m_objectAvailableCondition.wait(lock);
    }
    m_objects.clear();
    m_currentObjectCount = m_minObjectCount;
    for(auto i = std::size_t(0); i < m_currentObjectCount; ++i) {
      m_objects.push_back(m_builder());
    }
  }

  template<typename T, typename B>
  ScopedResource<typename ResourcePool<T, B>::Type,
      typename ResourcePool<T, B>::Make> ResourcePool<T, B>::Acquire() {
    auto lock = std::unique_lock(m_mutex);
    auto unconditionalWait = false;
    while(m_objects.empty()) {
      if(unconditionalWait || m_currentObjectCount >= m_maxObjectCount) {
        m_objectAvailableCondition.wait(lock);
      } else if(m_objectAvailableCondition.wait_for(
          std::chrono::microseconds(lock, m_timeout.total_microseconds())) ==
            std::cv_status::timeout) {
        if(m_objects.empty() && m_currentObjectCount < m_maxObjectCount) {
          try {
            auto scopedObject = ScopedResource(Ref(*this), m_builder());
            ++m_currentObjectCount;
            return scopedObject;
          } catch(const std::exception&) {
            unconditionalWait = true;
          }
        }
      }
    }
    auto object = ScopedResource(Ref(*this), std::move(m_objects.front()));
    m_objects.pop_front();
    return object;
  }

  template<typename T, typename B>
  boost::optional<ScopedResource<typename ResourcePool<T, B>::Type,
      typename ResourcePool<T, B>::Make>> ResourcePool<T, B>::TryAcquire() {
    auto lock = std::unique_lock(m_mutex);
    if(m_objects.empty()) {
      return boost::none;
    }
    auto object = ScopedResource(Ref(*this), std::move(m_objects.front()));
    m_objects.pop_front();
    return object;
  }

  template<typename T, typename B>
  void ResourcePool<T, B>::Add(std::unique_ptr<Type> object) {
    auto lock = std::lock_guard(m_mutex);
    m_objects.push_back(std::move(object));
    m_objectAvailableCondition.notify_one();
  }

  template<typename T, typename B>
  ScopedResource<T, B>::ScopedResource(Ref<ResourcePool<Type, Make>> pool,
    std::unique_ptr<Type> object)
    : m_pool(pool.Get()),
      m_object(std::move(object)) {}

  template<typename T, typename B>
  ScopedResource<T, B>::ScopedResource(ScopedResource<T, B>&& object)
    : m_pool(std::exchange(object.m_pool, nullptr)),
      m_object(std::move(object.m_object)) {}

  template<typename T, typename B>
  ScopedResource<T, B>::~ScopedResource() {
    if(m_pool) {
      m_pool->Add(std::move(m_object));
    }
  }

  template<typename T, typename B>
  typename ScopedResource<T, B>::Type&
      ScopedResource<T, B>::operator *() const {
    return *m_object;
  }

  template<typename T, typename B>
  typename ScopedResource<T, B>::Type*
      ScopedResource<T, B>::operator ->() const {
    return m_object.get();
  }
}

#endif
