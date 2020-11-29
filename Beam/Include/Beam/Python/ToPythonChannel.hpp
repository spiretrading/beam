#ifndef BEAM_TO_PYTHON_CHANNEL_HPP
#define BEAM_TO_PYTHON_CHANNEL_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/ToPythonConnection.hpp"
#include "Beam/Python/ToPythonReader.hpp"
#include "Beam/Python/ToPythonWriter.hpp"

namespace Beam::IO {

  /**
   * Wraps a Channel for use within Python.
   * @param <C> The type of Channel to wrap.
   */
  template<typename C>
  class ToPythonChannel {
    public:

      /** The type of Channel to wrap. */
      using Channel = C;
      using Identifier = ChannelIdentifierBox;
      using Connection = ConnectionBox;
      using Reader = ReaderBox;
      using Writer = WriterBox;

      /**
       * Constructs a ToPythonChannel in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      ToPythonChannel(Args&&... args);

      ToPythonChannel(ToPythonChannel&&) = default;

      ~ToPythonChannel();

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

      ToPythonChannel& operator =(ToPythonChannel&&) = default;

    private:
      boost::optional<Channel> m_channel;
      boost::optional<Identifier> m_identifier;
      boost::optional<Connection> m_connection;
      boost::optional<Reader> m_reader;
      boost::optional<Writer> m_writer;

      ToPythonChannel(const ToPythonChannel&) = delete;
      ToPythonChannel& operator =(const ToPythonChannel&) = delete;
  };

  template<typename Channel>
  ToPythonChannel(Channel&&) -> ToPythonChannel<std::decay_t<Channel>>;

  template<typename C>
  template<typename... Args>
  ToPythonChannel<C>::ToPythonChannel(Args&&... args)
    : m_channel((Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...),
      m_identifier(&m_channel->GetIdentifier()),
      m_connection(ToPythonConnection<ConnectionBox>(
        &m_channel->GetConnection())),
      m_reader(ToPythonReader<ReaderBox>(&m_channel->GetReader())),
      m_writer(ToPythonWriter<WriterBox>(&m_channel->GetWriter())) {}

  template<typename C>
  ToPythonChannel<C>::~ToPythonChannel() {
    auto release = Beam::Python::GilRelease();
    m_writer.reset();
    m_reader.reset();
    m_connection.reset();
    m_identifier.reset();
    m_channel.reset();
  }

  template<typename C>
  const typename ToPythonChannel<C>::Identifier&
      ToPythonChannel<C>::GetIdentifier() const {
    return *m_identifier;
  }

  template<typename C>
  typename ToPythonChannel<C>::Connection& ToPythonChannel<C>::GetConnection() {
    return *m_connection;
  }

  template<typename C>
  typename ToPythonChannel<C>::Reader& ToPythonChannel<C>::GetReader() {
    return *m_reader;
  }

  template<typename C>
  typename ToPythonChannel<C>::Writer& ToPythonChannel<C>::GetWriter() {
    return *m_writer;
  }
}

#endif
