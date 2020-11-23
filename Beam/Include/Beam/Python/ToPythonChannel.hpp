#ifndef BEAM_TO_PYTHON_CHANNEL_HPP
#define BEAM_TO_PYTHON_CHANNEL_HPP
#include <boost/optional/optional.hpp>
#include "Beam/IO/Channel.hpp"
#include "Beam/Python/GilRelease.hpp"

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
       * Constructs a ToPythonChannel instance.
       * @param channel The Channel to wrap.
       */
      ToPythonChannel(std::unique_ptr<Channel> channel);

      ~ToPythonChannel();

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      std::unique_ptr<Channel> m_channel;
      boost::optional<Identifier> m_identifier;
      boost::optional<Connection> m_connection;
      boost::optional<Reader> m_reader;
      boost::optional<Writer> m_writer;
  };

  /**
   * Builds a ToPythonChannel instance.
   * @param channel The Channel to wrap.
   */
  template<typename Channel>
  auto MakeToPythonChannel(std::unique_ptr<Channel> channel) {
    return std::make_unique<ToPythonChannel<Channel>>(std::move(channel));
  }

  template<typename C>
  ToPythonChannel<C>::ToPythonChannel(std::unique_ptr<Channel> channel)
    : m_channel(std::move(channel)),
      m_identifier(&m_channel->GetIdentifier()),
      m_connection(MakeToPythonConnection(std::make_unique<ConnectionBox>(
        &m_channel->GetConnection()))),
      m_reader(MakeToPythonReader(std::make_unique<ReaderBox>(
        &m_channel->GetReader()))),
      m_writer(MakeToPythonWriter(std::make_unique<WriterBox>(
        &m_channel->GetWriter()))) {}

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
