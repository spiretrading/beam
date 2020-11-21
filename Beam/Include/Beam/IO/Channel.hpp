#ifndef BEAM_CHANNEL_HPP
#define BEAM_CHANNEL_HPP
#include <ostream>
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Utilities/Concept.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam::IO {
namespace Details {
  BEAM_DEFINE_HAS_TYPEDEF(ChannelHasIdentifierType, Identifier);
  BEAM_DEFINE_HAS_TYPEDEF(ChannelHasConnectionType, Connection);
  BEAM_DEFINE_HAS_TYPEDEF(ChannelHasReaderType, Reader);
  BEAM_DEFINE_HAS_TYPEDEF(ChannelHasWriterType, Writer);
}

  /** Provides a way of identifying a Channel. */
  struct ChannelIdentifier : Concept<ChannelIdentifier> {
    protected:

      /** Streams the string representation of this value. */
      virtual std::ostream& Stream(std::ostream& out) const = 0;
  };

  /**
   * Composes a Connection, a Reader, and a Writer into an IO channel.
   * @param <I> The type of Identifier.
   * @param <C> The type of Connection.
   * @param <R> The type of Reader.
   * @param <W> The type of Writer.
   */
  template<typename I, typename C, typename R, typename W>
  struct Channel : Concept<Channel<I, C, R, W>> {
    static_assert(ImplementsConcept<I, ChannelIdentifier>::value,
      "I must implement the ChannelIdentifier Concept.");
    static_assert(ImplementsConcept<C, Connection>::value,
      "C must implement the Connection Concept.");
    static_assert(IsReader<R>::value, "R must implement the Reader Concept.");
    static_assert(IsWriter<W>::value, "W must implement the Writer Concept.");

    /** Defines the type of ChannelIdentifier. */
    using Identifier = I;

    /** Defines the type of Connection. */
    using Connection = C;

    /** Defines the type of Reader. */
    using Reader = R;

    /** Defines the type of Writer. */
    using Writer = W;

    /** Returns the Channel's identifier. */
    const Identifier& GetIdentifier() const;

    /** Returns the Connection. */
    Connection& GetConnection();

    /** Returns the Reader. */
    Reader& GetReader();

    /** Returns the Writer. */
    Writer& GetWriter();
  };

  /**
   * Tests whether a type satisfies some particular Channel Concept.
   * @param <T> The type to test.
   */
  template<typename T, typename Enabled = void>
  struct IsChannel : std::false_type {};

  template<typename T>
  struct IsChannel<T, std::enable_if_t<
    Details::ChannelHasIdentifierType<T>::value &&
    Details::ChannelHasConnectionType<T>::value &&
    Details::ChannelHasReaderType<T>::value &&
    Details::ChannelHasWriterType<T>::value>> :
    ImplementsConcept<T, Channel<typename T::Identifier, typename T::Connection,
    typename T::Reader, typename T::Writer>>::type {};
}

#endif
