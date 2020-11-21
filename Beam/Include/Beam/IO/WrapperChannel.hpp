#ifndef BEAM_WRAPPER_CHANNEL_HPP
#define BEAM_WRAPPER_CHANNEL_HPP
#include <type_traits>
#include <boost/mpl/if.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {
namespace Details {
  template<bool Selector>
  struct Switch {
    template<typename A, typename B>
    B& operator()(A& a, B& b) const {
      return b;
    }
  };

  template<>
  struct Switch<true> {
    template<typename A, typename B>
    A& operator()(A& a, B& b) const {
      return a;
    }
  };
}

  template<typename C, typename CT1 = NullType, typename CT2 = NullType,
    typename CT3 = NullType>
  class WrapperChannel {
    private:
      using Component1 = GetTryDereferenceType<CT1>;
      using Component2 = GetTryDereferenceType<CT2>;
      using Component3 = GetTryDereferenceType<CT3>;

    public:

      /** The type of Channel being wrapped. */
      using Channel = GetTryDereferenceType<C>;

      /** The type of Connection. */
      using Connection = typename boost::mpl::if_c<ImplementsConcept<
        Component1, IO::Connection>::value, Component1,
        typename boost::mpl::if_c<ImplementsConcept<
        Component2, IO::Connection>::value, Component2,
        typename boost::mpl::if_c<ImplementsConcept<
        Component3, IO::Connection>::value, Component3,
        typename Channel::Connection>::type>::type>::type;

      /** The type of Reader. */
      using Reader = typename boost::mpl::if_c<IsReader<Component1>::value,
        Component1, typename boost::mpl::if_c<IsReader<Component2>::value,
        Component2, typename boost::mpl::if_c<IsReader<Component3>::value,
        Component3, typename Channel::Reader>::type>::type>::type;

      /** The type of Writer, determined based on Component3. */
      using Writer = typename boost::mpl::if_c<IsWriter<Component1>::value,
        Component1, typename boost::mpl::if_c<IsWriter<Component2>::value,
        Component2, typename boost::mpl::if_c<IsWriter<Component3>::value,
        Component3, typename Channel::Writer>::type>::type>::type;

      /** The type of Identifier, taken directly from the Channel. */
      using Identifier = typename Channel::Identifier;

      /**
       * Constructs a WrapperChannel.
       * @param channel The channel to wrap.
       */
      template<typename CF>
      WrapperChannel(CF&& channel);

      /**
       * Constructs a WrapperChannel wrapping a single component.
       * @param channel The channel to wrap.
       * @param component1 The component used as the wrapper.
       */
      template<typename CF, typename CF1>
      WrapperChannel(CF&& channel, CF1&& component1);

      /**
       * Constructs a WrapperChannel wrapping two components.
       * @param channel The channel to wrap.
       * @param component1 One of the components used as a wrapper.
       * @param component2 The other component used as a wrapper.
       */
      template<typename CF, typename CF1, typename CF2>
      WrapperChannel(CF&& channel, CF1&& component1, CF2&& component2);

      /**
       * Constructs a WrapperChannel wrapping all components.
       * @param channel The channel to wrap.
       * @param component1 One of the components used as a wrapper.
       * @param component2 The second component used as a wrapper.
       * @param component3 The last component used as a wrapper.
       */
      template<typename CF, typename CF1, typename CF2, typename CF3>
      WrapperChannel(CF&& channel, CF1&& component1, CF2&& component2,
        CF3&& component3);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      GetOptionalLocalPtr<C> m_channel;
      GetOptionalLocalPtr<CT1> m_component1;
      GetOptionalLocalPtr<CT2> m_component2;
      GetOptionalLocalPtr<CT3> m_component3;
  };

  template<typename C, typename CT1, typename CT2, typename CT3>
  template<typename CF>
  WrapperChannel<C, CT1, CT2, CT3>::WrapperChannel(CF&& channel)
    : m_channel(std::forward<CF>(channel)) {}

  template<typename C, typename CT1, typename CT2, typename CT3>
  template<typename CF, typename CF1>
  WrapperChannel<C, CT1, CT2, CT3>::WrapperChannel(CF&& channel,
    CF1&& component1)
    : m_channel(std::forward<CF>(channel)),
      m_component1(std::forward<CF1>(component1)) {}

  template<typename C, typename CT1, typename CT2, typename CT3>
  template<typename CF, typename CF1, typename CF2>
  WrapperChannel<C, CT1, CT2, CT3>::WrapperChannel(CF&& channel,
    CF1&& component1, CF2&& component2)
    : m_channel(std::forward<CF>(channel)),
      m_component1(std::forward<CF1>(component1)),
      m_component2(std::forward<CF2>(component2)) {}

  template<typename C, typename CT1, typename CT2, typename CT3>
  template<typename CF, typename CF1, typename CF2, typename CF3>
  WrapperChannel<C, CT1, CT2, CT3>::WrapperChannel(CF&& channel,
    CF1&& component1, CF2&& component2, CF3&& component3)
    : m_channel(std::forward<CF>(channel)),
      m_component1(std::forward<CF1>(component1)),
      m_component2(std::forward<CF2>(component2)),
      m_component3(std::forward<CF3>(component3)) {}

  template<typename C, typename CT1, typename CT2, typename CT3>
  const typename WrapperChannel<C, CT1, CT2, CT3>::Identifier&
      WrapperChannel<C, CT1, CT2, CT3>::GetIdentifier() const {
    return m_channel->GetIdentifier();
  }

  template<typename C, typename CT1, typename CT2, typename CT3>
  typename WrapperChannel<C, CT1, CT2, CT3>::Connection&
      WrapperChannel<C, CT1, CT2, CT3>::GetConnection() {
    return Details::Switch<std::is_same_v<Connection, Component1>>()(
      *m_component1,
      Details::Switch<std::is_same_v<Connection, Component2>>()(
      *m_component2,
      Details::Switch<std::is_same_v<Connection, Component3>>()(
      *m_component3, m_channel->GetConnection())));
  }

  template<typename C, typename CT1, typename CT2, typename CT3>
  typename WrapperChannel<C, CT1, CT2, CT3>::Reader&
      WrapperChannel<C, CT1, CT2, CT3>::GetReader() {
    return Details::Switch<std::is_same_v<Reader, Component1>>()(
      *m_component1, Details::Switch<std::is_same_v<Reader, Component2>>()(
      *m_component2, Details::Switch<std::is_same_v<Reader, Component3>>()(
      *m_component3, m_channel->GetReader())));
  }

  template<typename C, typename CT1, typename CT2, typename CT3>
  typename WrapperChannel<C, CT1, CT2, CT3>::Writer&
      WrapperChannel<C, CT1, CT2, CT3>::GetWriter() {
    return Details::Switch<std::is_same_v<Writer, Component1>>()(
      *m_component1, Details::Switch<std::is_same_v<Writer, Component2>>()(
      *m_component2, Details::Switch<std::is_same_v<Writer, Component3>>()(
      *m_component3, m_channel->GetWriter())));
  }
}

  template<typename C, typename CT1, typename CT2, typename CT3>
  struct ImplementsConcept<IO::WrapperChannel<C, CT1, CT2, CT3>,
    IO::Channel<typename IO::WrapperChannel<C, CT1, CT2, CT3>::Identifier,
    typename IO::WrapperChannel<C, CT1, CT2, CT3>::Connection,
    typename IO::WrapperChannel<C, CT1, CT2, CT3>::Reader,
    typename IO::WrapperChannel<C, CT1, CT2, CT3>::Writer>> : std::true_type {};
}

#endif
