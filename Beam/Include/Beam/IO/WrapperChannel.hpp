#ifndef BEAM_WRAPPERCHANNEL_HPP
#define BEAM_WRAPPERCHANNEL_HPP
#include <boost/mpl/if.hpp>
#include <boost/noncopyable.hpp>
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

  template<typename ChannelType, typename ComponentType1 = NullType,
    typename ComponentType2 = NullType, typename ComponentType3 = NullType>
  class WrapperChannel {
    public:
      using Channel = GetTryDereferenceType<ChannelType>;
      using Component1 = GetTryDereferenceType<ComponentType1>;
      using Component2 = GetTryDereferenceType<ComponentType2>;
      using Component3 = GetTryDereferenceType<ComponentType3>;

      //! The type of Connection.
      using Connection = typename boost::mpl::if_c<ImplementsConcept<
        Component1, IO::Connection>::value, Component1,
        typename boost::mpl::if_c<ImplementsConcept<
        Component2, IO::Connection>::value, Component2,
        typename boost::mpl::if_c<ImplementsConcept<
        Component3, IO::Connection>::value, Component3,
        typename Channel::Connection>::type>::type>::type;

      //! The type of Reader.
      using Reader = typename boost::mpl::if_c<IsReader<Component1>::value,
        Component1, typename boost::mpl::if_c<IsReader<Component2>::value,
        Component2, typename boost::mpl::if_c<IsReader<Component3>::value,
        Component3, typename Channel::Reader>::type>::type>::type;

      //! The type of Writer, determined based on Component3.
      using Writer = typename boost::mpl::if_c<IsWriter<Component1>::value,
        Component1, typename boost::mpl::if_c<IsWriter<Component2>::value,
        Component2, typename boost::mpl::if_c<IsWriter<Component3>::value,
        Component3, typename Channel::Writer>::type>::type>::type;

      //! The type of Identifier, taken directly from the Channel.
      using Identifier = typename Channel::Identifier;

      template<typename ChannelForward>
      WrapperChannel(ChannelForward&& channel)
          : m_channel(std::forward<ChannelForward>(channel)) {}

      template<typename ChannelForward, typename ComponentForward1>
      WrapperChannel(ChannelForward&& channel, ComponentForward1&& component1)
          : m_channel(std::forward<ChannelForward>(channel)),
            m_component1(std::forward<ComponentForward1>(component1)) {}

      template<typename ChannelForward, typename ComponentForward1,
        typename ComponentForward2>
      WrapperChannel(ChannelForward&& channel, ComponentForward1&& component1,
          ComponentForward2&& component2)
          : m_channel(std::forward<ChannelForward>(channel)),
            m_component1(std::forward<ComponentForward1>(component1)),
            m_component2(std::forward<ComponentForward2>(component2)) {}

      template<typename ChannelForward, typename ComponentForward1,
        typename ComponentForward2, typename ComponentForward3>
      WrapperChannel(ChannelForward&& channel, ComponentForward1&& component1,
          ComponentForward2&& component2, ComponentForward3&& component3)
          : m_channel(std::forward<ChannelForward>(channel)),
            m_component1(std::forward<ComponentForward1>(component1)),
            m_component2(std::forward<ComponentForward2>(component2)),
            m_component3(std::forward<ComponentForward3>(component3)) {}

      const Identifier& GetIdentifier() const {
        return m_channel->GetIdentifier();
      }

      Connection& GetConnection() {
        return Details::Switch<std::is_same<Connection, Component1>::value>()(
          *m_component1, Details::Switch<std::is_same<Connection,
          Component2>::value>()(*m_component2, Details::Switch<std::is_same<
          Connection, Component3>::value>()(*m_component3,
          m_channel->GetConnection())));
      }

      Reader& GetReader() {
        return Details::Switch<std::is_same<Reader, Component1>::value>()(
          *m_component1, Details::Switch<std::is_same<Reader,
          Component2>::value>()(*m_component2, Details::Switch<std::is_same<
          Reader, Component3>::value>()(*m_component3,
          m_channel->GetReader())));
      }

      Writer& GetWriter() {
        return Details::Switch<std::is_same<Writer, Component1>::value>()(
          *m_component1, Details::Switch<std::is_same<Writer,
          Component2>::value>()(*m_component2, Details::Switch<std::is_same<
          Writer, Component3>::value>()(*m_component3,
          m_channel->GetWriter())));
      }

    private:
      typename OptionalLocalPtr<ChannelType>::type m_channel;
      typename OptionalLocalPtr<ComponentType1>::type m_component1;
      typename OptionalLocalPtr<ComponentType2>::type m_component2;
      typename OptionalLocalPtr<ComponentType3>::type m_component3;
  };
}

  template<typename ChannelType, typename ComponentType1,
    typename ComponentType2, typename ComponentType3>
  struct ImplementsConcept<IO::WrapperChannel<ChannelType, ComponentType1,
    ComponentType2, ComponentType3>, IO::Channel<typename IO::WrapperChannel<
    ChannelType, ComponentType1, ComponentType2, ComponentType3>::Identifier,
    typename IO::WrapperChannel<ChannelType, ComponentType1, ComponentType2,
    ComponentType3>::Connection, typename IO::WrapperChannel<ChannelType,
    ComponentType1, ComponentType2, ComponentType3>::Reader,
    typename IO::WrapperChannel<ChannelType, ComponentType1, ComponentType2,
    ComponentType3>::Writer>> : std::true_type {};
}

#endif
