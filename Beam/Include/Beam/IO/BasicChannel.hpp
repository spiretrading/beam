#ifndef BEAM_BASIC_CHANNEL_HPP
#define BEAM_BASIC_CHANNEL_HPP
#include <type_traits>
#include <utility>
#include "Beam/IO/Channel.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {

  /**
   * Basic implementation of the Channel interface.
   * @tparam I The type of Identifier.
   * @tparam C The type of Connection to compose.
   * @tparam R The type of Reader to compose.
   * @tparam W The type of Writer to compose.
   */
  template<typename I, typename C, typename R, typename W>
    requires IsChannelIdentifier<dereference_t<I>> &&
      IsConnection<dereference_t<C>> && IsReader<dereference_t<R>> &&
      IsWriter<dereference_t<W>>
  class BasicChannel {
    public:

      /** The type of Identifier. */
      using Identifier = dereference_t<I>;

      /** The type of Connection to compose. */
      using Connection = dereference_t<C>;

      /** The type of Reader to compose. */
      using Reader = dereference_t<R>;

      /** The type of Writer to compose. */
      using Writer = dereference_t<W>;

      /**
       * Constructs a BasicChannel.
       * @param identifier The Channel's Identifier.
       * @param connection The Channel's Connection.
       * @param reader The Channel's Reader.
       * @param writer The Channel's Writer.
       */
      template<Initializes<I> IF, Initializes<C> CF, Initializes<R> RF,
        Initializes<W> WF>
      BasicChannel(IF&& identifier, CF&& connection, RF&& reader, WF&& writer);

      const Identifier& get_identifier() const;
      Connection& get_connection();
      Reader& get_reader();
      Writer& get_writer();

    private:
      local_ptr_t<I> m_identifier;
      local_ptr_t<C> m_connection;
      local_ptr_t<R> m_reader;
      local_ptr_t<W> m_writer;

      BasicChannel(const BasicChannel&) = delete;
      BasicChannel& operator =(const BasicChannel&) = delete;
  };

  template<typename I, typename C, typename R, typename W>
  BasicChannel(I&&, C&&, R&&, W&&) -> BasicChannel<std::remove_cvref_t<I>,
    std::remove_cvref_t<C>, std::remove_cvref_t<R>, std::remove_cvref_t<W>>;

  template<typename I, typename C, typename R, typename W> requires
    IsChannelIdentifier<dereference_t<I>> && IsConnection<dereference_t<C>> &&
    IsReader<dereference_t<R>> && IsWriter<dereference_t<W>>
  template<Initializes<I> IF, Initializes<C> CF, Initializes<R> RF,
    Initializes<W> WF>
  BasicChannel<I, C, R, W>::BasicChannel(
    IF&& identifier, CF&& connection, RF&& reader, WF&& writer)
    : m_identifier(std::forward<IF>(identifier)),
      m_connection(std::forward<CF>(connection)),
      m_reader(std::forward<RF>(reader)),
      m_writer(std::forward<WF>(writer)) {}

  template<typename I, typename C, typename R, typename W> requires
    IsChannelIdentifier<dereference_t<I>> && IsConnection<dereference_t<C>> &&
    IsReader<dereference_t<R>> && IsWriter<dereference_t<W>>
  const typename BasicChannel<I, C, R, W>::Identifier&
      BasicChannel<I, C, R, W>::get_identifier() const {
    return *m_identifier;
  }

  template<typename I, typename C, typename R, typename W> requires
    IsChannelIdentifier<dereference_t<I>> && IsConnection<dereference_t<C>> &&
    IsReader<dereference_t<R>> && IsWriter<dereference_t<W>>
  typename BasicChannel<I, C, R, W>::Connection&
      BasicChannel<I, C, R, W>::get_connection() {
    return *m_connection;
  }

  template<typename I, typename C, typename R, typename W> requires
    IsChannelIdentifier<dereference_t<I>> && IsConnection<dereference_t<C>> &&
    IsReader<dereference_t<R>> && IsWriter<dereference_t<W>>
  typename BasicChannel<I, C, R, W>::Reader&
      BasicChannel<I, C, R, W>::get_reader() {
    return *m_reader;
  }

  template<typename I, typename C, typename R, typename W> requires
    IsChannelIdentifier<dereference_t<I>> && IsConnection<dereference_t<C>> &&
    IsReader<dereference_t<R>> && IsWriter<dereference_t<W>>
  typename BasicChannel<I, C, R, W>::Writer&
      BasicChannel<I, C, R, W>::get_writer() {
    return *m_writer;
  }
}

#endif
