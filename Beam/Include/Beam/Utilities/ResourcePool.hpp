#ifndef BEAM_RESOURCE_POOL_HPP
#define BEAM_RESOURCE_POOL_HPP
#include <algorithm>
#include <concepts>
#include <deque>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/locks.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/TimedConditionVariable.hpp"

namespace Beam {
  template<typename T, typename B> requires std::invocable<B> &&
    std::convertible_to<std::invoke_result_t<B>, std::unique_ptr<T>>
  class ResourcePool;

  /**
   * Stores an object acquired from a ResourcePool.
   * @tparam T The type of object stored.
   * @tparam B The type of function used to construct the object.
   */
  template<typename T, typename B> requires std::invocable<B> &&
    std::convertible_to<std::invoke_result_t<B>, std::unique_ptr<T>>
  class ScopedResource {
    public:

      /** The type of object stored. */
      using Type = T;

      /** The type of builder used to construct the object. */
      using Builder = B;

      /**
       * Constructs a ScopedResource.
       * @param pool The ResourcePool the object belongs to.
       * @param object The pooled object to manage.
       */
      ScopedResource(Ref<ResourcePool<Type, Builder>> pool,
        std::unique_ptr<Type> object) noexcept;

      /**
       * Acquires a ScopedResource.
       * @param object The ScopedResource to acquire.
       */
      ScopedResource(ScopedResource&& object) noexcept;

      ~ScopedResource();

      /** Returns a reference to the object. */
      Type& operator *() const;

      /** Returns a pointer to the object. */
      Type* operator ->() const;

    private:
      ResourcePool<Type, Builder>* m_pool;
      std::unique_ptr<Type> m_object;

      ScopedResource(const ScopedResource&) = delete;
      ScopedResource& operator =(const ScopedResource&) = delete;
  };

  /**
   * A resource pool.
   * @tparam T The type of object to pool.
   * @tparam B The type of function used to construct pooled objects.
   */
  template<typename T, typename B> requires std::invocable<B> &&
    std::convertible_to<std::invoke_result_t<B>, std::unique_ptr<T>>
  class ResourcePool {
    public:

      /** The type of object to pool. */
      using Type = T;

      /** The type of function used to build objects. */
      using Builder = B;

      /** The type of scoped resource returned by the pool. */
      using ScopedResource = Beam::ScopedResource<Type, Builder>;

      /**
       * Constructs a ResourcePool.
       * @param timeout The amount of time to wait for an object before
       *        constructing a new one.
       * @param builder The function used to build objects.
       * @param min_count The minimum number of objects to build.
       * @param max_count The maximum number of objects to build.
       */
      template<typename BF> requires std::constructible_from<B, BF>
      ResourcePool(boost::posix_time::time_duration timeout, BF&& builder,
        std::size_t min_count = 1,
        std::size_t max_count = std::numeric_limits<std::size_t>::max());

      /**
       * Destroys all existing objects and rebuilds the minimum number of
       * objects.
       */
      void reset();

      /** Acquires the next resource. */
      ScopedResource load();

      /** Returns a resource if one is available without blocking. */
      boost::optional<ScopedResource> try_load();

    private:
      friend class Beam::ScopedResource<Type, Builder>;
      Mutex m_mutex;
      boost::posix_time::time_duration m_timeout;
      Builder m_builder;
      std::size_t m_min_count;
      std::size_t m_max_count;
      std::size_t m_current_count;
      std::deque<std::unique_ptr<Type>> m_objects;
      TimedConditionVariable m_is_available_condition;

      ResourcePool(const ResourcePool&) = delete;
      ResourcePool& operator =(const ResourcePool&) = delete;
      void add(std::unique_ptr<Type> object);
  };

  template<typename BF>
  ResourcePool(
    boost::posix_time::time_duration, BF&&, std::size_t, std::size_t) ->
      ResourcePool<std::remove_pointer_t<
        typename std::invoke_result_t<std::remove_cvref_t<BF>>::element_type>,
        std::remove_cvref_t<BF>>;

  template<typename BF>
  ResourcePool(boost::posix_time::time_duration, BF&&, std::size_t) ->
    ResourcePool<std::remove_pointer_t<
      typename std::invoke_result_t<std::remove_cvref_t<BF>>::element_type>,
      std::remove_cvref_t<BF>>;

  template<typename BF>
  ResourcePool(boost::posix_time::time_duration, BF&&) ->
    ResourcePool<std::remove_pointer_t<
      typename std::invoke_result_t<std::remove_cvref_t<BF>>::element_type>,
      std::remove_cvref_t<BF>>;

  template<typename T, typename B> requires std::invocable<B> &&
    std::convertible_to<std::invoke_result_t<B>, std::unique_ptr<T>>
  template<typename BF> requires std::constructible_from<B, BF>
  ResourcePool<T, B>::ResourcePool(boost::posix_time::time_duration timeout,
      BF&& builder, std::size_t min_count, std::size_t max_count)
      : m_timeout(timeout),
        m_builder(std::forward<BF>(builder)),
        m_min_count(std::max<std::size_t>(1, min_count)),
        m_max_count(std::max(m_min_count, max_count)),
        m_current_count(m_min_count) {
    for(auto i = std::size_t(0); i < m_current_count; ++i) {
      m_objects.push_back(m_builder());
    }
  }

  template<typename T, typename B> requires std::invocable<B> &&
    std::convertible_to<std::invoke_result_t<B>, std::unique_ptr<T>>
  void ResourcePool<T, B>::reset() {
    auto lock = boost::unique_lock(m_mutex);
    while(m_objects.size() != m_current_count) {
      m_is_available_condition.wait(lock);
    }
    m_objects.clear();
    m_current_count = m_min_count;
    for(auto i = std::size_t(0); i < m_current_count; ++i) {
      m_objects.push_back(m_builder());
    }
  }

  template<typename T, typename B> requires std::invocable<B> &&
    std::convertible_to<std::invoke_result_t<B>, std::unique_ptr<T>>
  ScopedResource<typename ResourcePool<T, B>::Type,
      typename ResourcePool<T, B>::Builder> ResourcePool<T, B>::load() {
    auto lock = boost::unique_lock(m_mutex);
    auto is_unconditionally_waiting = false;
    while(m_objects.empty()) {
      if(is_unconditionally_waiting || m_current_count >= m_max_count) {
        m_is_available_condition.wait(lock);
      } else {
        try {
          m_is_available_condition.timed_wait(m_timeout, lock);
        } catch(const TimeoutException&) {
          if(m_objects.empty() && m_current_count < m_max_count) {
            try {
              auto scoped_object = ScopedResource(Ref(*this), m_builder());
              ++m_current_count;
              return scoped_object;
            } catch(const std::exception&) {
              is_unconditionally_waiting = true;
            }
          }
        }
      }
    }
    auto object = ScopedResource(Ref(*this), std::move(m_objects.front()));
    m_objects.pop_front();
    return object;
  }

  template<typename T, typename B> requires std::invocable<B> &&
    std::convertible_to<std::invoke_result_t<B>, std::unique_ptr<T>>
  boost::optional<ScopedResource<typename ResourcePool<T, B>::Type,
      typename ResourcePool<T, B>::Builder>> ResourcePool<T, B>::try_load() {
    auto lock = boost::unique_lock(m_mutex);
    if(m_objects.empty()) {
      return boost::none;
    }
    auto object = ScopedResource(Ref(*this), std::move(m_objects.front()));
    m_objects.pop_front();
    return object;
  }

  template<typename T, typename B> requires std::invocable<B> &&
    std::convertible_to<std::invoke_result_t<B>, std::unique_ptr<T>>
  void ResourcePool<T, B>::add(std::unique_ptr<Type> object) {
    auto lock = boost::lock_guard(m_mutex);
    m_objects.push_back(std::move(object));
    m_is_available_condition.notify_one();
  }

  template<typename T, typename B> requires std::invocable<B> &&
    std::convertible_to<std::invoke_result_t<B>, std::unique_ptr<T>>
  ScopedResource<T, B>::ScopedResource(Ref<ResourcePool<Type, Builder>> pool,
    std::unique_ptr<Type> object) noexcept
    : m_pool(pool.get()),
      m_object(std::move(object)) {}

  template<typename T, typename B> requires std::invocable<B> &&
    std::convertible_to<std::invoke_result_t<B>, std::unique_ptr<T>>
  ScopedResource<T, B>::ScopedResource(ScopedResource<T, B>&& object) noexcept
    : m_pool(std::exchange(object.m_pool, nullptr)),
      m_object(std::move(object.m_object)) {}

  template<typename T, typename B> requires std::invocable<B> &&
    std::convertible_to<std::invoke_result_t<B>, std::unique_ptr<T>>
  ScopedResource<T, B>::~ScopedResource() {
    if(m_pool) {
      m_pool->add(std::move(m_object));
    }
  }

  template<typename T, typename B> requires std::invocable<B> &&
    std::convertible_to<std::invoke_result_t<B>, std::unique_ptr<T>>
  typename ScopedResource<T, B>::Type&
      ScopedResource<T, B>::operator *() const {
    return *m_object;
  }

  template<typename T, typename B> requires std::invocable<B> &&
    std::convertible_to<std::invoke_result_t<B>, std::unique_ptr<T>>
  typename ScopedResource<T, B>::Type*
      ScopedResource<T, B>::operator ->() const {
    return m_object.get();
  }
}

#endif
