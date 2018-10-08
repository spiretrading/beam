#ifndef BEAM_LOCAL_CONNECTION_HPP
#define BEAM_LOCAL_CONNECTION_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Routines/Async.hpp"

namespace Beam {
namespace IO {

  /** Implements a local Connection.
      \tparam BufferType The type of Buffer to use.
   */
  template<typename BufferType>
  class LocalConnection : private boost::noncopyable {
    public:

      //! The type of Buffer to use.
      using Buffer = BufferType;

      //! The type of LocalServerConnection this connects to.
      using LocalServerConnection = IO::LocalServerConnection<Buffer>;

      //! Constructs a LocalConnection.
      /*!
        \param pendingChannels The Queue to post pending Channels to.
        \param writer The Writer maintaining the Connection.
      */
      LocalConnection(const std::shared_ptr<Queue<
        typename LocalServerConnection::PendingChannelEntry*>>& pendingChannels,
        const std::shared_ptr<PipedWriter<Buffer>>& writer);

      ~LocalConnection();

      void Open();

      void Close();

    private:
      friend class IO::LocalServerChannel<Buffer>;
      friend class IO::LocalClientChannel<Buffer>;
      friend class IO::LocalServerConnection<Buffer>;
      using PendingChannelEntry =
        typename LocalServerConnection::PendingChannelEntry;
      IO::LocalClientChannel<Buffer>* m_channel;
      std::shared_ptr<Queue<PendingChannelEntry*>> m_pendingChannels;
      std::shared_ptr<PipedWriter<Buffer>> m_writer;
      std::shared_ptr<PipedWriter<Buffer>> m_endpointWriter;
      bool m_isOpen;
  };

  template<typename BufferType>
  LocalConnection<BufferType>::LocalConnection(const std::shared_ptr<
      Queue<PendingChannelEntry*>>& pendingChannels,
      const std::shared_ptr<PipedWriter<Buffer>>& writer)
      : m_channel(nullptr),
        m_pendingChannels(pendingChannels),
        m_writer(writer),
        m_isOpen(false) {}

  template<typename BufferType>
  LocalConnection<BufferType>::~LocalConnection() {
    Close();
  }

  template<typename BufferType>
  void LocalConnection<BufferType>::Open() {
    if(m_isOpen) {
      return;
    }
    auto openAsync = Routines::Async<void>();
    auto entry = PendingChannelEntry(Ref(*m_channel), openAsync.GetEval());
    m_pendingChannels->Push(&entry);
    openAsync.Get();
    m_isOpen = true;
  }

  template<typename BufferType>
  void LocalConnection<BufferType>::Close() {
    if(!m_isOpen) {
      return;
    }
    m_writer->Break();
    m_endpointWriter->Break();
    m_isOpen = false;
  }
}

  template<typename BufferType>
  struct ImplementsConcept<IO::LocalConnection<BufferType>, IO::Connection> :
    std::true_type {};
}

#endif
