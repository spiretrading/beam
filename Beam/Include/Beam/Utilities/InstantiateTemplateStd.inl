#ifndef BEAM_INSTANTIATETEMPLATE_HPP
#define BEAM_INSTANTIATETEMPLATE_HPP
#include <stdexcept>
#include <typeinfo>
#include <type_traits>
#include <boost/typeof/typeof.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/throw_exception.hpp>
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/empty.hpp>
#include <boost/preprocessor/expand.hpp>
#include <boost/preprocessor/facilities/expand.hpp>
#include <boost/preprocessor/facilities/intercept.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repeat.hpp>

namespace Beam {

  /*! \class InstantiationNotSupportedException
      \brief Indicates that a template instantiation is not supported.
   */
  class InstantiationNotSupportedException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs an InstantiationNotSupportedException.
      InstantiationNotSupportedException();
  };

namespace Detail {
  #define BEAM_INSTANTIATE_PARAMETERS 10

  template<typename TemplateMetaClass, int Args>
  struct TemplateInstantiater {};

  #define BEAM_DEDUCE_TYPE_PARAMETERS(z, n, q)                                 \
     BOOST_PP_COMMA_IF(n) typename boost::mpl::at_c<typename boost::mpl::front<\
      typename TemplateMetaClass::SupportedTypes>::type, n>::type

  #define BEAM_PARAMETER_TYPEDEFS(z, n, q)                                     \
    using T##n = typename boost::mpl::at_c<Signature, n>::type;

  #define BEAM_COMPARE_TYPES(z, n, q) && type##n == typeid(T##n)

  template<typename TemplateMetaClass>
  struct TemplateInstantiater<TemplateMetaClass, 1> {
    using FunctionType = boost::type_of::remove_cv_ref_t<decltype(
      (TemplateMetaClass::template Template<typename boost::mpl::at_c<
          typename TemplateMetaClass::SupportedTypes, 0>::type>))>*;

    template<typename MetaClass, typename Signatures>
    struct FindInstantiation {
      static FunctionType Invoke(const std::type_info& type) {
        using T = typename boost::mpl::front<Signatures>::type;
        if(type == typeid(T)) {
          return MetaClass::template Template<T>;
        }
        return FindInstantiation<MetaClass,
          typename boost::mpl::pop_front<Signatures>::type>::Invoke(type);
      }
    };

    template<typename MetaClass>
    struct FindInstantiation<MetaClass, boost::mpl::l_end> {
      static FunctionType Invoke(const std::type_info& type) {
        BOOST_THROW_EXCEPTION(InstantiationNotSupportedException());
      }
    };

    static FunctionType Instantiate(const std::type_info& type) {
      return FindInstantiation<TemplateMetaClass,
        typename TemplateMetaClass::SupportedTypes>::Invoke(type);
    }
  };

  #define BOOST_PP_LOCAL_MACRO(n)                                              \
  template<typename TemplateMetaClass>                                         \
  struct TemplateInstantiater<TemplateMetaClass, n> {                          \
    using FunctionType = BOOST_PP_EXPAND(BOOST_TYPEOF_TPL                      \
      (TemplateMetaClass::template Template<                                   \
      BOOST_PP_REPEAT(n, BEAM_DEDUCE_TYPE_PARAMETERS, BOOST_PP_EMPTY)>))*;     \
                                                                               \
    template<typename MetaClass, typename Signatures>                          \
    struct FindInstantiation {                                                 \
      static FunctionType Invoke(                                              \
          BOOST_PP_ENUM_PARAMS(n, const std::type_info& type)) {               \
        using Signature = typename boost::mpl::front<Signatures>::type;        \
        BOOST_PP_REPEAT(n, BEAM_PARAMETER_TYPEDEFS, BOOST_PP_EMPTY);           \
        if(true BOOST_PP_REPEAT(n, BEAM_COMPARE_TYPES, BOOST_PP_EMPTY)) {      \
          return MetaClass::template Template<BOOST_PP_ENUM_PARAMS(n, T)>;     \
        }                                                                      \
        return FindInstantiation<MetaClass,                                    \
          typename boost::mpl::pop_front<Signatures>::type>::Invoke(           \
          BOOST_PP_ENUM_PARAMS(n, type));                                      \
      }                                                                        \
    };                                                                         \
                                                                               \
    template<typename MetaClass>                                               \
    struct FindInstantiation<MetaClass, boost::mpl::l_end> {                   \
      static FunctionType Invoke(                                              \
          BOOST_PP_ENUM_PARAMS(n, const std::type_info& type)) {               \
        BOOST_THROW_EXCEPTION(InstantiationNotSupportedException());           \
      }                                                                        \
    };                                                                         \
                                                                               \
    static FunctionType Instantiate(                                           \
        BOOST_PP_ENUM_PARAMS(n, const std::type_info& type)) {                 \
      return FindInstantiation<TemplateMetaClass,                              \
        typename TemplateMetaClass::SupportedTypes>::Invoke(                   \
        BOOST_PP_ENUM_PARAMS(n, type));                                        \
    }                                                                          \
  };

  #define BOOST_PP_LOCAL_LIMITS (2, BEAM_INSTANTIATE_PARAMETERS)
  #include BOOST_PP_LOCAL_ITERATE()
  #undef BEAM_DEDUCE_TYPE_PARAMETERS
  #undef BEAM_PARAMETER_TYPEDEFS
  #undef BEAM_COMPARE_TYPES
}

  inline InstantiationNotSupportedException::
      InstantiationNotSupportedException()
      : std::runtime_error("") {}

  //! Returns a function template's instantiation.
  /*!
    \param type The template parameter type.
    \return The function template instantiated with the specified types.
  */
  #define BOOST_PP_LOCAL_MACRO(n)                                              \
  template<typename TemplateMetaClass>                                         \
  auto Instantiate(BOOST_PP_ENUM_PARAMS(n, const std::type_info& type)) {      \
    return Detail::TemplateInstantiater<TemplateMetaClass, n>::Instantiate(    \
      BOOST_PP_ENUM_PARAMS(n, type));                                          \
  }

  #define BOOST_PP_LOCAL_LIMITS (1, BEAM_INSTANTIATE_PARAMETERS)
  #include BOOST_PP_LOCAL_ITERATE()
}

#endif
