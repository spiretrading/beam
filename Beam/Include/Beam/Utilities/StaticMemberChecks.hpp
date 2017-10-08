#ifndef BEAM_STATICMEMBERCHECKS_HPP
#define BEAM_STATICMEMBERCHECKS_HPP
#include <type_traits>

#define BEAM_DEFINE_HAS_METHOD(Name, Method, ReturnType, ...)                  \
  template<typename T, typename Enable = void>                                 \
  struct Name {                                                                \
    private:                                                                   \
      class yes { char m; };                                                   \
                                                                               \
      class no { yes m[2]; };                                                  \
                                                                               \
      struct BaseMixin {                                                       \
        ReturnType Method(__VA_ARGS__) {}                                      \
      };                                                                       \
                                                                               \
      struct Base : public T, public BaseMixin {};                             \
                                                                               \
      template <typename V, V v>                                               \
      class Helper {};                                                         \
                                                                               \
      template <typename U>                                                    \
      static no deduce(U*, Helper<ReturnType (BaseMixin::*)(__VA_ARGS__),      \
        &U::Method>* = 0);                                                     \
                                                                               \
      static yes deduce(...);                                                  \
                                                                               \
    public:                                                                    \
      static const bool value = sizeof(yes) == sizeof(deduce((Base*)(0)));     \
  };                                                                           \
                                                                               \
  template<typename T>                                                         \
  struct Name<T, typename std::enable_if<!std::is_class<T>::value>::type> {    \
    static const bool value = false;                                           \
  };

#define BEAM_DEFINE_HAS_TYPEDEF(Name, Typedef)                                 \
  template<typename T>                                                         \
  struct Name {                                                                \
    typedef char YesType;                                                      \
    typedef struct {                                                           \
      char a[2];                                                               \
    } NoType;                                                                  \
                                                                               \
    template<typename C>                                                       \
    static YesType Test(typename C::Typedef*);                                 \
                                                                               \
    template<typename C>                                                       \
    static NoType Test(...);                                                   \
                                                                               \
    static const bool value = sizeof(Test<T>(nullptr)) == sizeof(YesType);     \
  };

#define BEAM_DEFINE_HAS_VARIABLE(Name, Variable)                               \
  template<typename T>                                                         \
  struct Name {                                                                \
    typedef char YesType;                                                      \
    typedef struct {                                                           \
      char a[2];                                                               \
    } NoType;                                                                  \
                                                                               \
    template<typename C>                                                       \
    static YesType Test(decltype(C::Variable)*);                               \
                                                                               \
    template<typename C>                                                       \
    static NoType Test(...);                                                   \
                                                                               \
    static const bool value = sizeof(Test<T>(nullptr)) == sizeof(YesType);     \
  };

#endif
