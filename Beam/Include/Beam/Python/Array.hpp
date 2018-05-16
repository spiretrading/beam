#ifndef BEAM_PYTHON_ARRAY_HPP
#define BEAM_PYTHON_ARRAY_HPP
#include <array>
#include <type_traits>
#include <typeinfo>
#include <boost/noncopyable.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/indexing_suite.hpp>
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename>
  struct array_trait;

  template<typename T>
  class array_proxy {
    public:
      using value_type = T;
      using iterator = T*;
      using reference = T&;
      using size_type = std::size_t;
      using difference_type = std::ptrdiff_t;

      array_proxy()
        : m_ptr{0},
          m_length{0} {}

      template<typename Iterator>
      array_proxy(Iterator begin, Iterator end)
        : m_ptr{&*begin},
          m_length{std::distance(begin, end)} {}

      array_proxy(reference begin, std::size_t length)
        : m_ptr{&begin},
          m_length{length} {}

      iterator begin() {
        return m_ptr;
      }

      iterator end() {
        return m_ptr + m_length;
      }

      reference operator[](size_t i) {
        return m_ptr[i];
      }

      size_type size() {
        return m_length;
      }

    private:
      T* m_ptr;
      std::size_t m_length;
  };

  template<typename T>
  auto make_array_proxy(T& array) {
    return array_proxy<typename array_trait<T>::element_type>(
      array[0], array_trait<T>::static_size);
  }

  template<typename Container>
  class ref_index_suite : public boost::python::indexing_suite<
      Container, ref_index_suite<Container>> {
    public:
      using data_type = typename Container::value_type;
      using index_type = typename Container::size_type;
      using size_type = typename Container::size_type;

      static data_type& get_item(Container& container, index_type index) {
        return container[index];
      }

      static void set_item(Container& container, index_type index,
          const data_type& value) {
        container[index] = value;
      }

      static void delete_item(Container& container, index_type index) {
        set_item(container, index, data_type{});
      };

      static boost::python::object get_slice(Container& container,
          index_type from, index_type to) {
        boost::python::list list;
        while(from != to) {
          list.append(container[from]);
          ++from;
        }
        return list;
      };

      static void set_slice(Container& container, index_type from,
          index_type to, const data_type& value) {
        while(from < to) {
          container[from] = value;
          ++from;
        }
      }

      template<typename Iterator>
      static void set_slice(Container& container, index_type from,
          index_type to, Iterator first, Iterator last) {
        while(from < to) {
          container[from] = *first;
          ++from;
          ++first;
        }
      }

      static void delete_slice(Container& container, index_type from,
          index_type to) {
        set_slice(container, from, to, data_type{});
      }

      static std::size_t size(Container& container) {
        return container.size();
      }

      template<typename T>
      static bool contains(Container& container, const T& value) {
        return std::find(container.begin(), container.end(), value) !=
          container.end();
      }

      static bool compare_index(Container& container,
          index_type a, index_type b) {
        return a < b;
      }

      static index_type get_min_index(Container&) {
        return 0;
      }

      static index_type get_max_index(Container& container) {
        return size(container);
      }

      static index_type convert_index(Container& container, PyObject* object) {
        boost::python::extract<long> py_index{object};
        if(!py_index.check()) {
          PyErr_SetString(PyExc_TypeError, "Invalid index type");
          boost::python::throw_error_already_set(); 
        }
        long index = py_index();
        if(index < 0) {
          index += static_cast<long>(container.size());
        }
        if(index >= static_cast<long>(container.size()) || index < 0) {
          PyErr_SetString(PyExc_IndexError, "Index out of range");
          boost::python::throw_error_already_set();
        }
        return index;
      }
  };

  template<typename T>
  struct array_trait_impl;

  template<typename T, std::size_t N>
  struct array_trait_impl<T[N]> {
    using element_type = T;
    enum {
      static_size = N
    };
    using proxy_type = array_proxy<element_type>;
    using policy = boost::python::default_call_policies;
    using signature = boost::mpl::vector<array_proxy<element_type>>;
  };

  template<typename T, std::size_t N>
  struct array_trait_impl<std::array<T, N>> : public array_trait_impl<T[N]> {};

  template<typename T, typename C>
  struct array_trait_impl<T (C::*)> : public array_trait_impl<T> {
    using policy = boost::python::with_custodian_and_ward_postcall<0, 1>;
    using signature = typename boost::mpl::push_back<
      typename array_trait_impl<T>::signature, C&>::type;
  };

  template<typename T>
  struct array_trait: public array_trait_impl<
      typename std::remove_pointer<T>::type> {
    using native_type = T;
  };

  template<typename Trait>
  class array_proxy_getter {
    public:
      using native_type = typename Trait::native_type;
      using proxy_type = typename Trait::proxy_type;

      array_proxy_getter(native_type array)
          : m_array{array} {}

      template<typename C>
      proxy_type operator()(C& c) {
        return make_array_proxy(c.*m_array);
      }

      proxy_type operator()() {
        return make_array_proxy(*m_array);
      }

    private:
      native_type m_array;
  };

  template<typename T>
  struct ArrayGetItemDispatch {
    template<typename Container>
    auto operator ()(Container& container, int index) const {
      return container[index];
    }
  };

  template<typename T, std::size_t N>
  struct ArrayGetItemDispatch<std::array<T, N>> {
    template<typename Container>
    auto operator ()(Container& container, int index) const {
      return array_proxy<T>{container[index][0], N};
    }
  };

  template<typename ProxyType>
  auto array_get_item(ProxyType& container, int index) {
    return ArrayGetItemDispatch<
      typename std::decay<decltype(container[index])>::type>()(
      container, index);
  }

  template<typename Trait>
  void register_array_proxy() {
#ifdef _MSC_VER
    using element_type = typename Trait::element_type;
    using proxy_type = typename Trait::proxy_type;
    auto isRegistered = boost::python::converter::registry::query(
      boost::python::type_id<proxy_type>())->to_python_target_type() != 0;
    if(isRegistered) {
      return;
    }
    auto name = std::string{"_"} + typeid(element_type).name();
    boost::python::class_<proxy_type>(name.c_str(), boost::python::no_init)
      .def(ref_index_suite<proxy_type>())
      .def("__getitem__", &array_get_item<proxy_type>)
      .def("__iter__", boost::python::iterator<proxy_type>());
#else
    // TODO: Remove a lot of GCC template deduction workarounds.
    using element_type = typename Trait::element_type;
    using proxy_type = typename Trait::proxy_type;
    using F = decltype(&array_get_item<proxy_type>);
    using python_class = boost::python::class_<proxy_type>;
    auto isRegistered = boost::python::converter::registry::query(
      boost::python::type_id<proxy_type>())->to_python_target_type() != 0;
    if(isRegistered) {
      return;
    }
    auto name = std::string{"_"} + typeid(element_type).name();
    auto f = static_cast<python_class& (python_class::*)(const char*, F)>(
      &python_class::template def<F>);
    (boost::python::class_<proxy_type>(name.c_str(), boost::python::no_init)
      .def(ref_index_suite<proxy_type>())
      .*f)("__getitem__", &array_get_item<proxy_type>)
      .def("__iter__", boost::python::iterator<proxy_type>());
#endif
  }

  template <typename Array>
  boost::python::object make_array_aux(Array array) {
    using trait_type = array_trait<Array>;
    register_array_proxy<trait_type>();
    return boost::python::make_function(array_proxy_getter<trait_type>(array),
      typename trait_type::policy(), typename trait_type::signature());
  }

  template<typename T>
  struct ArrayDispatcher {
    auto operator ()(T array) const {
      return make_array_aux(array);
    }
  };

  template<typename T, typename C, std::size_t N1, std::size_t N2>
  struct ArrayDispatcher<std::array<std::array<T, N2>, N1> (C::*)> {
    auto operator ()(std::array<std::array<T, N2>, N1> (C::* array)) const {
      register_array_proxy<array_trait<std::array<T, N2>>>();
      return make_array_aux(array);
    }
  };
}

  //! Converts an array to Python.
  /*!
    \param array The array to convert.
  */
  template <typename T>
  boost::python::object MakeArray(T array) {
    return Details::ArrayDispatcher<T>()(array);
  }
}
}

#endif
