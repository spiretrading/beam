#ifndef BEAM_BASICCHANNEL_HPP
#define BEAM_BASICCHANNEL_HPP
#include <utility>
#include "Beam/IO/Channel.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /*! \class BasicChannel
      \brief Basic implementation of the Channel interface.
      \tparam IdentifierType The type of Identifier.
      \tparam ConnectionType The type of Connection to compose.
      \tparam ReaderType The type of Reader to compose.
      \tparam WriterType The type of Writer to compose.
   */
  template<typename IdentifierType, typename ConnectionType,
    typename ReaderType, typename WriterType>
  class BasicChannel : private boost::noncopyable {
    public:
      using Identifier = IdentifierType;
      using Connection = typename TryDereferenceType<ConnectionType>::type;
      using Reader = typename TryDereferenceType<ReaderType>::type;
      using Writer = typename TryDereferenceType<WriterType>::type;

      //! Constructs a BasicChannel.
      /*!
        \tparam IdentifierForward A type compatible with IdentifierType.
        \tparam ConnectionForward A type compatible with ConnectionType.
        \tparam ReaderForward A type compatible with ReaderType.
        \tparam WriterForward A type compatible with WriterType.
        \param identifier The Channel's Identifier.
        \param connection The Channel's Connection.
        \param reader The Channel's Reader.
        \param writer The Channel's Writer.
      */
      template<typename IdentifierForward, typename ConnectionForward,
        typename ReaderForward, typename WriterForward>
      BasicChannel(IdentifierForward&& identifier,
        ConnectionForward&& connection, ReaderForward&& reader,
        WriterForward&& writer);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      Identifier m_identifier;
      typename OptionalLocalPtr<ConnectionType>::type m_connection;
      typename OptionalLocalPtr<ReaderType>::type m_reader;
      typename OptionalLocalPtr<WriterType>::type m_writer;
  };

  template<typename IdentifierType, typename ConnectionType,
    typename ReaderType, typename WriterType>
  template<typename IdentifierForward, typename ConnectionForward,
    typename ReaderForward, typename WriterForward>
  BasicChannel<IdentifierType, ConnectionType, ReaderType, WriterType>::
      BasicChannel(IdentifierForward&& identifier,
      ConnectionForward&& connection, ReaderForward&& reader,
      WriterForward&& writer)
      : m_identifier(std::forward<IdentifierForward>(identifier)),
        m_connection(std::forward<ConnectionForward>(connection)),
        m_reader(std::forward<ReaderForward>(reader)),
        m_writer(std::forward<WriterForward>(writer)) {}

  template<typename IdentifierType, typename ConnectionType,
    typename ReaderType, typename WriterType>
  const typename BasicChannel<IdentifierType, ConnectionType, ReaderType,
      WriterType>::Identifier& BasicChannel<IdentifierType, ConnectionType,
      ReaderType, WriterType>::GetIdentifier() const {
    return m_identifier;
  }

  template<typename IdentifierType, typename ConnectionType,
    typename ReaderType, typename WriterType>
  typename BasicChannel<IdentifierType, ConnectionType, ReaderType,
      WriterType>::Connection& BasicChannel<IdentifierType, ConnectionType,
      ReaderType, WriterType>::GetConnection() {
    return *m_connection;
  }

  template<typename IdentifierType, typename ConnectionType,
    typename ReaderType, typename WriterType>
  typename BasicChannel<IdentifierType, ConnectionType, ReaderType,
      WriterType>::Reader& BasicChannel<IdentifierType, ConnectionType,
      ReaderType, WriterType>::GetReader() {
    return *m_reader;
  }

  template<typename IdentifierType, typename ConnectionType,
    typename ReaderType, typename WriterType>
  typename BasicChannel<IdentifierType, ConnectionType, ReaderType,
      WriterType>::Writer& BasicChannel<IdentifierType, ConnectionType,
      ReaderType, WriterType>::GetWriter() {
    return *m_writer;
  }
}

  template<typename IdentifierType, typename ConnectionType,
    typename ReaderType, typename WriterType>
  struct ImplementsConcept<IO::BasicChannel<IdentifierType, ConnectionType,
    ReaderType, WriterType>, IO::Channel<
    typename IO::BasicChannel<IdentifierType, ConnectionType, ReaderType,
      WriterType>::Identifier,
    typename IO::BasicChannel<IdentifierType, ConnectionType, ReaderType,
      WriterType>::Connection,
    typename IO::BasicChannel<IdentifierType, ConnectionType, ReaderType,
      WriterType>::Reader,
    typename IO::BasicChannel<IdentifierType, ConnectionType, ReaderType,
      WriterType>::Writer>> : std::true_type {};
}

#endif
