#ifndef BEAM_PARSERPUBLISHER_HPP
#define BEAM_PARSERPUBLISHER_HPP
#include <atomic>
#include "Beam/Collections/Enum.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/Traits.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam::Parsers {

  /*! \enum ParserErrorPolicy
      \brief Specifies how a Parser Reactor should handle an error.
   */
  BEAM_ENUM(ParserErrorPolicy,

    //! Report the error as an exception.
    REPORT = 0,

    //! Skip the entry that produced the error.
    SKIP,

    //! Stop reading any further.
    STOP);

  /*! \class ParserPublisher
      \brief Publishes values parsed from a Reader.
      \tparam R The type of Reader to parse from.
      \tparam P The type of Parser to use.
   */
  template<typename R, typename P>
  class ParserPublisher final : public Publisher<parser_result_t<P>> {
    public:

      //! The type of Reader to parse from.
      using Reader = R;

      //! The type of Parser to use.
      using Parser = P;
      using Source = parser_result_t<Parser>;

      //! Constructs a ParserPublisher.
      /*!
        \param reader Initializes the Reader to use.
        \param parser Initializes the Parser.
        \param errorPolicy The policy used to handle an error.
      */
      template<typename RF>
      ParserPublisher(RF&& reader, Parser parser,
        ParserErrorPolicy errorPolicy);

      void With(const std::function<void ()>& f) const override;

      void Monitor(ScopedQueueWriter<Source> monitor) const override;

    private:
      GetOptionalLocalPtr<R> m_reader;
      Parser m_parser;
      ParserErrorPolicy m_errorPolicy;
      QueueWriterPublisher<Source> m_publisher;
      mutable std::atomic_bool m_isParsing;
      mutable Routines::RoutineHandler m_parseLoop;

      void ParseLoop();
  };

  template<typename RF, typename Parser>
  ParserPublisher(RF&&, Parser, ParserErrorPolicy) ->
    ParserPublisher<std::decay_t<RF>, to_parser_t<Parser>>;

  template<typename R, typename P>
  template<typename RF>
  ParserPublisher<R, P>::ParserPublisher(RF&& reader, Parser parser,
    ParserErrorPolicy errorPolicy)
    : m_reader(std::forward<RF>(reader)),
      m_parser(std::move(parser)),
      m_errorPolicy(errorPolicy),
      m_isParsing(false) {}

  template<typename R, typename P>
  void ParserPublisher<R, P>::With(const std::function<void ()>& f) const {
    m_publisher.With(f);
  }

  template<typename R, typename P>
  void ParserPublisher<R, P>::Monitor(ScopedQueueWriter<Source> monitor) const {
    m_publisher.Monitor(std::move(monitor));
    auto isParsing = m_isParsing.exchange(true);
    if(!isParsing) {
      m_parseLoop = Routines::Spawn(std::bind(&ParserPublisher::ParseLoop,
        const_cast<ParserPublisher*>(this)));
    }
  }

  template<typename R, typename P>
  void ParserPublisher<R, P>::ParseLoop() {
    auto stream = ReaderParserStream(&*m_reader);
    auto value = Source();
    while(m_parser.Read(stream, value)) {
      m_publisher.Push(std::move(value));
    }
  }
}

#endif
