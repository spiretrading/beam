#ifndef BEAM_TO_PYTHON_CHANNEL_HPP
#define BEAM_TO_PYTHON_CHANNEL_HPP
#include "Beam/Python/GilRelease.hpp"
#include "Beam/IO/VirtualChannel.hpp"

namespace Beam::IO {

  /**
   * Wraps a Channel for use within Python.
   * @param <C> The type of Channel to wrap.
   */
  template<typename C>
  class ToPythonChannel : public VirtualChannel {
    public:

      /** The type of Channel to wrap. */
      using Channel = C;

      /**
       * Constructs a ToPythonChannel instance.
       * @param channel The Channel to wrap.
       */
      ToPythonChannel(std::unique_ptr<Channel> channel);

      ~ToPythonChannel() override;

      const Identifier& GetIdentifier() const override;

      Connection& GetConnection() override;

      Reader& GetReader() override;

      Writer& GetWriter() override;

    private:
      std::unique_ptr<Channel> m_channel;
      std::unique_ptr<Identifier> m_identifier;
      std::unique_ptr<Connection> m_connection;
      std::unique_ptr<Reader> m_reader;
      std::unique_ptr<Writer> m_writer;
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
      m_identifier(MakeVirtualChannelIdentifier(&m_channel->GetIdentifier())),
      m_connection(MakeToPythonConnection(MakeVirtualConnection(
        &m_channel->GetConnection()))),
      m_reader(MakeToPythonReader(MakeVirtualReader(&m_channel->GetReader()))),
      m_writer(MakeToPythonWriter(
        MakeVirtualWriter(&m_channel->GetWriter()))) {}

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
