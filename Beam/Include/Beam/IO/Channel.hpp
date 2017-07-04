#ifndef BEAM_CHANNEL_HPP
#define BEAM_CHANNEL_HPP
#include <string>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Utilities/Concept.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam {
namespace IO {
namespace Details {
  BEAM_DEFINE_HAS_TYPEDEF(ChannelHasIdentifierType, Identifier);
  BEAM_DEFINE_HAS_TYPEDEF(ChannelHasConnectionType, Connection);
  BEAM_DEFINE_HAS_TYPEDEF(ChannelHasReaderType, Reader);
  BEAM_DEFINE_HAS_TYPEDEF(ChannelHasWriterType, Writer);
}

  /*! \struct ChannelIdentifier
      \brief Provides a way of identifying a Channel.
    */
  struct ChannelIdentifier : Concept<ChannelIdentifier> {

    //! Returns the string representation of this ChannelIdentifier.
    std::string ToString() const;
  };

  /*! \struct Channel
      \brief Composes a Connection, a Reader, and a Writer into an IO channel.
      \tparam IdentifierType The type of Identifier.
      \tparam ConnectionType The type of Connection.
      \tparam ReaderType The type of Reader.
      \tparam WriterType The type of Writer.
   */
  template<typename IdentifierType, typename ConnectionType,
    typename ReaderType, typename WriterType>
  struct Channel : Concept<Channel<IdentifierType, ConnectionType, ReaderType,
      WriterType>> {
    static_assert(ImplementsConcept<IdentifierType, ChannelIdentifier>::value,
      "IdentifierType must implement the ChannelIdentifier Concept.");
    static_assert(ImplementsConcept<ConnectionType, IO::Connection>::value,
      "ConnectionType must implement the Connection Concept.");
    static_assert(IsReader<ReaderType>::value,
      "ReaderType must implement the Reader Concept.");
    static_assert(IsWriter<WriterType>::value,
      "WriterType must implement the Writer Concept.");

    //! Defines the type of ChannelIdentifier.
    using Identifier = IdentifierType;

    //! Defines the type of Connection.
    using Connection = ConnectionType;

    //! Defines the type of Reader.
    using Reader = ReaderType;

    //! Defines the type of Writer.
    using Writer = WriterType;

    //! Returns the Channel's identifier.
    const Identifier& GetIdentifier() const;

    //! Returns the Connection.
    Connection& GetConnection();

    //! Returns the Reader.
    Reader& GetReader();

    //! Returns the Writer.
    Writer& GetWriter();
  };

  /*! \struct IsChannel
      \brief Tests whether a type satisfies some particular Channel Concept.
      \tparam T The type to test.
   */
  template<typename T, typename Enabled = void>
  struct IsChannel : std::false_type {};

  template<typename T>
  struct IsChannel<T, typename std::enable_if<
    Details::ChannelHasIdentifierType<T>::value &&
    Details::ChannelHasConnectionType<T>::value &&
    Details::ChannelHasReaderType<T>::value &&
    Details::ChannelHasWriterType<T>::value>::type> : boost::mpl::if_c<
    ImplementsConcept<T, Channel<typename T::Identifier, typename T::Connection,
    typename T::Reader, typename T::Writer>>::value, std::true_type,
    std::false_type>::type {};
}
}

#endif
