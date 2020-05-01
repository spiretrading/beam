#ifndef BEAM_SHUTTLEVARIANT_HPP
#define BEAM_SHUTTLEVARIANT_HPP
#include <boost/mpl/at.hpp>
#include <boost/mpl/size.hpp>
#include <boost/throw_exception.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/SerializationException.hpp"
#include "Beam/Utilities/ApplyTuple.hpp"

namespace Beam {
namespace Serialization {
namespace Details {
  template<typename Shuttler, typename Variant>
  void Send(Shuttler& shuttle, IntegerSequence<>, int which,
      const Variant& value) {
    BOOST_THROW_EXCEPTION(SerializationException("Invalid variant."));
  }

  template<typename Shuttler, typename Variant, int Head, int... Tail>
  void Send(Shuttler& shuttle, IntegerSequence<Head, Tail...>, int which,
      const Variant& value) {
    using Type = typename boost::mpl::at_c<typename Variant::types, Head>::type;
    if(which == Head) {
      shuttle.Shuttle("value", boost::get<Type>(value));
      return;
    }
    Send(shuttle, IntegerSequence<Tail...>(), which, value);
  }

  template<typename Shuttler, typename Variant>
  void Receive(Shuttler& shuttle, IntegerSequence<>, int which,
      Variant& value) {
    BOOST_THROW_EXCEPTION(SerializationException("Invalid variant."));
  }

  template<typename Shuttler, typename Variant, int Head, int... Tail>
  void Receive(Shuttler& shuttle, IntegerSequence<Head, Tail...>, int which,
      Variant& value) {
    using Type = typename boost::mpl::at_c<typename Variant::types, Head>::type;
    if(which == Head) {
      Type v;
      shuttle.Shuttle("value", v);
      value = std::move(v);
      return;
    }
    Receive(shuttle, IntegerSequence<Tail...>(), which, value);
  }
}
  template<typename T>
  struct Send<boost::variant<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const boost::variant<T>& value,
        unsigned int version) const {
      shuttle.Shuttle("value", boost::get<T>(value));
    }
  };

  template<typename Arg, typename... Args>
  struct Send<boost::variant<Arg, Args...>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        const boost::variant<Arg, Args...>& value, unsigned int version) const {
      using Sequence = typename IntegerSequenceGenerator<boost::mpl::size<
        typename boost::variant<Arg, Args...>::types>::value>::type;
      int which = value.which();
      shuttle.Shuttle("which", which);
      Details::Send(shuttle, Sequence(), which, value);
    }
  };

  template<typename T>
  struct Receive<boost::variant<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, boost::variant<T>& value,
        unsigned int version) const {
      T v;
      shuttle.Shuttle("value", v);
      value = v;
    }
  };

  template<typename Arg, typename... Args>
  struct Receive<boost::variant<Arg, Args...>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, boost::variant<Arg, Args...>& value,
        unsigned int version) const {
      using Sequence = typename IntegerSequenceGenerator<boost::mpl::size<
        typename boost::variant<Arg, Args...>::types>::value>::type;
      int which;
      shuttle.Shuttle("which", which);
      Details::Receive(shuttle, Sequence(), which, value);
    }
  };
}
}

#endif
