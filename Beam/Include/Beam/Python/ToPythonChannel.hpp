#ifndef BEAM_TO_PYTHON_CHANNEL_HPP
#define BEAM_TO_PYTHON_CHANNEL_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/ToPythonConnection.hpp"
#include "Beam/Python/ToPythonReader.hpp"
#include "Beam/Python/ToPythonWriter.hpp"

namespace Beam::Python {

  /**
   * Wraps a Channel for use within Python.
   * @tparam C The type of Channel to wrap.
   */
  template<IsChannel C>
  class ToPythonChannel {
    public:

      /** The type of Channel to wrap. */
      using Channel = C;

      using Identifier = typename Channel::Identifier;
      using Connection = Beam::Connection;
      using Reader = Beam::Reader;
      using Writer = Beam::Writer;

      /**
       * Constructs a ToPythonChannel in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonChannel(Args&&... args);

      ~ToPythonChannel();

      /** Returns a reference to the underlying channel. */
      Channel& get();

      /** Returns a reference to the underlying channel. */
      const Channel& get() const;

      const Identifier& get_identifier() const;
      Connection& get_connection();
      Reader& get_reader();
      Writer& get_writer();

    private:
      boost::optional<Channel> m_channel;
      boost::optional<Connection> m_connection;
      boost::optional<Reader> m_reader;
      boost::optional<Writer> m_writer;

      ToPythonChannel(const ToPythonChannel&) = delete;
      ToPythonChannel& operator =(const ToPythonChannel&) = delete;
  };

  template<typename Channel>
  ToPythonChannel(Channel&&) -> ToPythonChannel<std::remove_cvref_t<Channel>>;

  template<IsChannel C>
  template<typename... Args>
  ToPythonChannel<C>::ToPythonChannel(Args&&... args)
    : m_channel((GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...),
      m_connection(boost::in_place_init,
        std::in_place_type<ToPythonConnection<Connection>>,
        &m_channel->get_connection()),
      m_reader(boost::in_place_init,
        std::in_place_type<ToPythonReader<Reader>>, &m_channel->get_reader()),
      m_writer(boost::in_place_init,
        std::in_place_type<ToPythonWriter<Writer>>, &m_channel->get_writer()) {}

  template<IsChannel C>
  ToPythonChannel<C>::~ToPythonChannel() {
    auto release = GilRelease();
    m_writer.reset();
    m_reader.reset();
    m_connection.reset();
    m_channel.reset();
  }

  template<IsChannel C>
  typename ToPythonChannel<C>::Channel& ToPythonChannel<C>::get() {
    return *m_channel;
  }

  template<IsChannel C>
  const typename ToPythonChannel<C>::Channel& ToPythonChannel<C>::get()
      const {
    return *m_channel;
  }

  template<IsChannel C>
  const typename ToPythonChannel<C>::Identifier&
      ToPythonChannel<C>::get_identifier() const {
    return m_channel->get_identifier();
  }

  template<IsChannel C>
  typename ToPythonChannel<C>::Connection&
      ToPythonChannel<C>::get_connection() {
    return *m_connection;
  }

  template<IsChannel C>
  typename ToPythonChannel<C>::Reader& ToPythonChannel<C>::get_reader() {
    return *m_reader;
  }

  template<IsChannel C>
  typename ToPythonChannel<C>::Writer& ToPythonChannel<C>::get_writer() {
    return *m_writer;
  }
}

#endif
