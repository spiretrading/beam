#ifndef BEAM_VIEW_HPP
#define BEAM_VIEW_HPP
#include <concepts>
#include <iterator>
#include <memory>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/tuple/tuple.hpp>
#include "Beam/Collections/AnyIterator.hpp"
#include "Beam/Collections/SharedIterator.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /**
   * Provides a shallow view over a collection or pair of iterators.
   * @tparam T The type to provide the view over.
   */
  template<typename T>
  class View {
    public:

      /** The type represented by the view. */
      using Type = T;

      /** The type of iterator used. */
      using iterator = AnyIterator<Type>;

      /** The type of iterator used. */
      using const_iterator = AnyIterator<const Type>;

      /** The type to view over. */
      using value_type = typename iterator::value_type;

      /** A reference to the type being viewed. */
      using reference = typename iterator::reference;

      /** A pointer to the type being viewed. */
      using pointer = typename iterator::pointer;

      /** Constructs an empty View. */
      View() = default;

      /**
       * Constructs a View from a collection.
       * @param collection The collection to view.
       */
      template<std::ranges::input_range C>
      View(const C& collection);

      /**
       * Constructs a View from a collection.
       * @param collection The collection to view.
       */
      template<std::ranges::input_range C,
        typename = disable_copy_constructor_t<View, C>>
      View(C& collection);

      /**
       * Constructs a View from a collection.
       * @param collection The collection to view.
       */
      template<std::ranges::input_range C,
        typename = disable_copy_constructor_t<View, C>>
      View(C&& collection);

      /**
       * Copies a View.
       * @param view The View to copy.
       */
      template<typename U>
      View(const View<U>& view);

      /**
       * Moves a View.
       * @param view The View to move.
       */
      template<typename U>
      View(View<U>&& view);

      /**
       * Constructs a View from two iterators.
       * @param begin An iterator to the beginning of the View.
       * @param end An iterator to the end of the View.
       */
      template<typename BeginIterator, typename EndIterator>
      View(BeginIterator&& begin, EndIterator&& end);

      View(const View&) = default;
      View(View&& view) = default;

      /**
       * Returns an element contained in this View.
       * @param index The index of the element to return.
       * @return The element at the specified <i>index</i>.
       */
      const Type& operator [](std::size_t index) const;

      /**
       * Returns an element contained in this View.
       * @param index The index of the element to return.
       * @return The element at the specified <i>index</i>.
       */
      Type& operator [](std::size_t index);

      /** Returns the number of elements in this View. */
      std::size_t size() const;

      /** Tests if the view is empty. */
      bool empty() const;

      /** Returns the first element in this View. */
      const Type& front() const;

      /** Returns the first element in this View. */
      Type& front();

      /** Returns the last element in this View. */
      const Type& back() const;

      /** Returns the last element in this View. */
      Type& back();

      /** Returns an iterator to the beginning of the view. */
      iterator begin() const;

      /** Returns an iterator to the end of the view. */
      iterator end() const;

      /** Returns a constant iterator to the beginning of the view. */
      const_iterator cbegin() const;

      /** Returns a constant iterator to the end of the view. */
      const_iterator cend() const;

    private:
      template<typename> friend class View;
      iterator m_begin;
      iterator m_end;
  };

  template<typename Collection>
  View(Collection&&) -> View<std::remove_cvref_t<
    decltype(*std::declval<Collection>().begin())>>;

  template<typename B, typename E>
  View(B&&, E&&) -> View<std::remove_cvref_t<decltype(*std::declval<B>())>>;

  /**
   * Drops the last elements of a View.
   * @param view The View to drop elements from.
   */
  template<typename T>
  View<T> drop_last(const View<T>& view) {
    if(view.size() <= 1) {
      return View<T>();
    } else {
      return View(view.begin(), view.begin() + (view.size() - 1));
    }
  }

  /**
   * Creates a View that dereferences each element of a collection of pointers.
   * @param range The collection to create the View over.
   * @return The dereferenced View.
   */
  template<typename C>
  auto make_dereference_view(C& range) {
    auto first = std::begin(range);
    auto last = std::end(range);
    auto transform = [] (const auto& ptr) -> decltype(auto) {
      return *ptr;
    };
    return View(
      boost::make_transform_iterator(first, transform),
      boost::make_transform_iterator(last, transform));
  }

  template<typename T>
  template<std::ranges::input_range C>
  View<T>::View(const C& collection)
    : m_begin(collection.cbegin()),
      m_end(collection.cend()) {}

  template<typename T>
  template<std::ranges::input_range C, typename>
  View<T>::View(C& collection)
    : m_begin(collection.begin()),
      m_end(collection.end()) {}

  template<typename T>
  template<std::ranges::input_range C, typename>
  View<T>::View(C&& collection) {
    auto shared_collection = std::make_shared<C>(std::forward<C>(collection));
    m_begin = SharedIterator(shared_collection, shared_collection->begin());
    m_end = SharedIterator(shared_collection, shared_collection->end());
  }

  template<typename T>
  template<typename U>
  View<T>::View(const View<U>& view)
    : View(view.m_begin, view.m_end) {}

  template<typename T>
  template<typename U>
  View<T>::View(View<U>&& view)
    : View(std::move(view.m_begin), std::move(view.m_end)) {}

  template<typename T>
  template<typename BeginIterator, typename EndIterator>
  View<T>::View(BeginIterator&& begin, EndIterator&& end)
    : m_begin(std::forward<BeginIterator>(begin)),
      m_end(std::forward<EndIterator>(end)) {}

  template<typename T>
  const typename View<T>::Type& View<T>::operator [](std::size_t index) const {
    return *(m_begin + index);
  }

  template<typename T>
  typename View<T>::Type& View<T>::operator [](std::size_t index) {
    return *(m_begin + index);
  }

  template<typename T>
  std::size_t View<T>::size() const {
    return std::distance(m_begin, m_end);
  }

  template<typename T>
  bool View<T>::empty() const {
    return m_begin == m_end;
  }

  template<typename T>
  const typename View<T>::Type& View<T>::front() const {
    return *m_begin;
  }

  template<typename T>
  typename View<T>::Type& View<T>::front() {
    return *m_begin;
  }

  template<typename T>
  const typename View<T>::Type& View<T>::back() const {
    auto end = m_end;
    --end;
    return *end;
  }

  template<typename T>
  typename View<T>::Type& View<T>::back() {
    auto end = m_end;
    --end;
    return *end;
  }

  template<typename T>
  typename View<T>::iterator View<T>::begin() const {
    return m_begin;
  }

  template<typename T>
  typename View<T>::iterator View<T>::end() const {
    return m_end;
  }

  template<typename T>
  typename View<T>::const_iterator View<T>::cbegin() const {
    return begin();
  }

  template<typename T>
  typename View<T>::const_iterator View<T>::cend() const {
    return end();
  }
}

#endif
