#ifndef BEAM_BASIC_CHANNEL_HPP
#define BEAM_BASIC_CHANNEL_HPP
#include <type_traits>
#include <utility>
#include "Beam/IO/Channel.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /**
   * Basic implementation of the Channel interface.
   * @param <I> The type of Identifier.
   * @param <C> The type of Connection to compose.
   * @param <R> The type of Reader to compose.
   * @param <W> The type of Writer to compose.
   */
  template<typename I, typename C, typename R, typename W>
  class BasicChannel {
    public:

      /** The type of Identifier. */
      using Identifier = I;

      /** The type of Connection to compose. */
      using Connection = GetTryDereferenceType<C>;

      /** The type of Reader to compose. */
      using Reader = GetTryDereferenceType<R>;

      /** The type of Writer to compose. */
      using Writer = GetTryDereferenceType<W>;

      /**
       * Constructs a BasicChannel.
       * @param identifier The Channel's Identifier.
       * @param connection The Channel's Connection.
       * @param reader The Channel's Reader.
       * @param writer The Channel's Writer.
       */
      template<typename IF, typename CF, typename RF, typename WF>
      BasicChannel(IF&& identifier, CF&& connection, RF&& reader, WF&& writer);

      const Identifier& GetIdentifier() const;

      Connection& GetConnection();

      Reader& GetReader();

      Writer& GetWriter();

    private:
      Identifier m_identifier;
      GetOptionalLocalPtr<C> m_connection;
      GetOptionalLocalPtr<R> m_reader;
      GetOptionalLocalPtr<W> m_writer;

      BasicChannel(const BasicChannel&) = delete;
      BasicChannel& operator =(const BasicChannel&) = delete;
  };

  template<typename I, typename C, typename R, typename W>
  BasicChannel(I&&, C&&, R&&, W&&) -> BasicChannel<std::decay_t<I>,
    std::decay_t<C>, std::decay_t<R>, std::decay_t<W>>;

  template<typename I, typename C, typename R, typename W>
  template<typename IF, typename CF, typename RF, typename WF>
  BasicChannel<I, C, R, W>::BasicChannel(IF&& identifier, CF&& connection,
    RF&& reader, WF&& writer)
    : m_identifier(std::forward<IF>(identifier)),
      m_connection(std::forward<CF>(connection)),
      m_reader(std::forward<RF>(reader)),
      m_writer(std::forward<WF>(writer)) {}

  template<typename I, typename C, typename R, typename W>
  const typename BasicChannel<I, C, R, W>::Identifier&
      BasicChannel<I, C, R, W>::GetIdentifier() const {
    return m_identifier;
  }

  template<typename I, typename C, typename R, typename W>
  typename BasicChannel<I, C, R, W>::Connection&
      BasicChannel<I, C, R, W>::GetConnection() {
    return *m_connection;
  }

  template<typename I, typename C, typename R, typename W>
  typename BasicChannel<I, C, R, W>::Reader&
      BasicChannel<I, C, R, W>::GetReader() {
    return *m_reader;
  }

  template<typename I, typename C, typename R, typename W>
  typename BasicChannel<I, C, R, W>::Writer&
      BasicChannel<I, C, R, W>::GetWriter() {
    return *m_writer;
  }
}

  template<typename I, typename C, typename R, typename W>
  struct ImplementsConcept<IO::BasicChannel<I, C, R, W>, IO::Channel<
    typename IO::BasicChannel<I, C, R, W>::Identifier,
    typename IO::BasicChannel<I, C, R, W>::Connection,
    typename IO::BasicChannel<I, C, R, W>::Reader,
    typename IO::BasicChannel<I, C, R, W>::Writer>> : std::true_type {};
}

#endif
