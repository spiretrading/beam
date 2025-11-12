#ifndef BEAM_PARSER_PUBLISHER_HPP
#define BEAM_PARSER_PUBLISHER_HPP
#include <atomic>
#include "Beam/Collections/Enum.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/ParserTraits.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Specifies how a Parser Reactor should handle an error. */
  BEAM_ENUM(ParserErrorPolicy,

    /** Report the error as an exception. */
    REPORT = 0,

    /** Skip the entry that produced the error. */
    SKIP,

    /** Stop reading any further. */
    STOP);

  /**
   * Publishes values parsed from a Reader.
   * @tparam R The type of Reader to parse from.
   * @tparam P The type of Parser to use.
   */
  template<typename R, IsParser P> requires IsReader<dereference_t<R>>
  class ParserPublisher final : public Publisher<parser_result_t<P>> {
    public:

      /** The type of Reader to parse from. */
      using Reader = dereference_t<R>;

      /** The type of Parser to use. */
      using Parser = P;

      using Source = parser_result_t<Parser>;

      /**
       * Constructs a ParserPublisher.
       * @param reader Initializes the Reader to use.
       * @param parser Initializes the Parser.
       * @param error_policy The policy used to handle an error.
       */
      template<Initializes<R> RF>
      ParserPublisher(
        RF&& reader, Parser parser, ParserErrorPolicy error_policy);

      void with(const std::function<void ()>& f) const override;
      void monitor(ScopedQueueWriter<Source> monitor) const override;

    private:
      local_ptr_t<R> m_reader;
      Parser m_parser;
      ParserErrorPolicy m_error_policy;
      QueueWriterPublisher<Source> m_publisher;
      mutable std::atomic_bool m_is_parsing;
      mutable RoutineHandler m_loop;

      void loop();
  };

  template<typename RF, typename P>
  ParserPublisher(RF&&, P, ParserErrorPolicy) ->
    ParserPublisher<std::remove_cvref_t<RF>, to_parser_t<P>>;

  template<typename R, IsParser P> requires IsReader<dereference_t<R>>
  template<Initializes<R> RF>
  ParserPublisher<R, P>::ParserPublisher(
    RF&& reader, Parser parser, ParserErrorPolicy error_policy)
    : m_reader(std::forward<RF>(reader)),
      m_parser(std::move(parser)),
      m_error_policy(error_policy),
      m_is_parsing(false) {}

  template<typename R, IsParser P> requires IsReader<dereference_t<R>>
  void ParserPublisher<R, P>::with(const std::function<void ()>& f) const {
    m_publisher.with(f);
  }

  template<typename R, IsParser P> requires IsReader<dereference_t<R>>
  void ParserPublisher<R, P>::monitor(ScopedQueueWriter<Source> monitor) const {
    m_publisher.monitor(std::move(monitor));
    if(!m_is_parsing.exchange(true)) {
      m_loop = spawn(std::bind_front(
        &ParserPublisher::loop, const_cast<ParserPublisher*>(this)));
    }
  }

  template<typename R, IsParser P> requires IsReader<dereference_t<R>>
  void ParserPublisher<R, P>::loop() {
    auto stream = ReaderParserStream(&*m_reader);
    auto value = Source();
    while(m_parser.read(stream, value)) {
      m_publisher.push(std::move(value));
    }
    m_publisher.close();
  }
}

#endif
