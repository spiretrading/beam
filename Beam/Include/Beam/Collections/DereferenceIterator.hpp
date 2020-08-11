#ifndef BEAM_DEREFERENCE_ITERATOR_HPP
#define BEAM_DEREFERENCE_ITERATOR_HPP
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/iterator/transform_iterator.hpp>
#include "Beam/Collections/Collections.hpp"
#include "Beam/Collections/View.hpp"

namespace Beam {
namespace Details {
  template<typename T>
  struct Dereference {
    using value_type = decltype(*std::declval<typename T::value_type>());
    using result_type = value_type&;

    template<typename U>
    result_type operator ()(const U& value) const {
      return *value;
    }
  };
}

  /**
   * Returns an iterator that dereferences its underlying value.
   * @param iterator The underlying iterator.
   */
  template<typename Iterator>
  auto MakeDereferenceIterator(Iterator&& iterator) {
    return boost::make_transform_iterator(std::forward<Iterator>(iterator),
      Details::Dereference<Iterator>());
  }

  /**
   * Makes a View that dereferences elements within a Collection.
   * @param collection The Collection to create the dereferenced View over.
   */
  template<typename Collection>
  auto MakeDereferenceView(Collection&& collection) {
    return View(MakeDereferenceIterator(collection.begin()),
      MakeDereferenceIterator(collection.end()));
  }
}

#endif
