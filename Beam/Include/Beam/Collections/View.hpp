#ifndef BEAM_VIEW_HPP
#define BEAM_VIEW_HPP
#include <iterator>
#include <memory>
#include <type_traits>
#include "Beam/Collections/AnyIterator.hpp"
#include "Beam/Collections/Collections.hpp"
#include "Beam/Collections/SharedIterator.hpp"

namespace Beam {

  /**
   * Provides a shallow view over a collection or pair of iterators.
   * @param <T> The type to provide the view over.
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
       * @param collection The Collection to view.
       */
      template<typename Collection>
      View(const Collection& collection);

      /**
       * Constructs a View from a collection.
       * @param collection The Collection to view.
       */
      template<typename Collection, typename = std::enable_if_t<
        !std::is_base_of_v<View, std::remove_reference_t<Collection>>>>
      View(Collection& collection);

      /**
       * Constructs a View from a collection.
       * @param collection The Collection to view.
       */
      template<typename Collection, typename = std::enable_if_t<
        !std::is_base_of_v<View, std::remove_reference_t<Collection>> &&
        !std::is_lvalue_reference_v<Collection>>>
      View(Collection&& collection);

      /**
       * Copies a View.
       * @param view The View to copy.
       */
      View(const View& view) = default;

      /**
       * Copies a View.
       * @param view The View to copy.
       */
      template<typename U, typename = std::enable_if_t<
        std::is_same_v<U, std::remove_const_t<Type>> && std::is_const_v<Type>>>
      View(const View<U>& view);

      /**
       * Moves a View.
       * @param view The View to move.
       */
      View(View&& view) = default;

      /**
       * Moves a View.
       * @param view The View to move.
       */
      template<typename U, typename = std::enable_if_t<
        std::is_same_v<U, std::remove_const_t<Type>> && std::is_const_v<Type>>>
      View(View<U>&& view);

      /**
       * Constructs a View from two iterators.
       * @param beginIterator An iterator to the beginning of the View.
       * @param endIterator An iterator to the end of the View.
       */
      template<typename BeginIterator, typename EndIterator>
      View(BeginIterator&& beginIterator, EndIterator&& endIterator);

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
  View(Collection&&) -> View<std::remove_reference_t<
    decltype(*std::declval<Collection>().begin())>>;

  template<typename B, typename E>
  View(B&&, E&&) -> View<std::remove_reference_t<decltype(*std::declval<B>())>>;

  /**
   * Drops the last elements of a View.
   * @param view The View to drop elements from.
   */
  template<typename T>
  View<T> DropLast(const View<T>& view) {
    if(view.size() <= 1) {
      return View<T>();
    } else {
      return View(view.begin(), view.begin() + (view.size() - 1));
    }
  }

  template<typename T>
  template<typename Collection>
  View<T>::View(const Collection& collection)
    : m_begin(collection.cbegin()),
      m_end(collection.cend()) {}

  template<typename T>
  template<typename Collection, typename>
  View<T>::View(Collection& collection)
    : m_begin(collection.begin()),
      m_end(collection.end()) {}

  template<typename T>
  template<typename Collection, typename>
  View<T>::View(Collection&& collection) {
    auto sharedCollection = std::make_shared<Collection>(
      std::forward<Collection>(collection));
    m_begin = SharedIterator(sharedCollection, sharedCollection->begin());
    m_end = SharedIterator(sharedCollection, sharedCollection->end());
  }

  template<typename T>
  template<typename U, typename>
  View<T>::View(const View<U>& view)
    : View(view.m_begin, view.m_end) {}

  template<typename T>
  template<typename U, typename>
  View<T>::View(View<U>&& view)
    : View(std::move(view.m_begin), std::move(view.m_end)) {}

  template<typename T>
  template<typename BeginIterator, typename EndIterator>
  View<T>::View(BeginIterator&& beginIterator, EndIterator&& endIterator)
    : m_begin(std::forward<BeginIterator>(beginIterator)),
      m_end(std::forward<EndIterator>(endIterator)) {}

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
