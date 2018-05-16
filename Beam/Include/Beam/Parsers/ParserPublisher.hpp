#ifndef BEAM_PARSERPUBLISHER_HPP
#define BEAM_PARSERPUBLISHER_HPP
#include <atomic>
#include "Beam/Collections/Enum.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

namespace Beam {
namespace Parsers {

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
      \tparam ReaderType The type of Reader to parse from.
      \tparam ParserType The type of Parser to use.
   */
  template<typename ReaderType, typename ParserType>
  class ParserPublisher : public Publisher<typename ParserType::Result> {
    public:

      //! The type of Reader to parse from.
      using Reader = ReaderType;

      //! The type of Parser to use.
      using Parser = ParserType;
      using Source = typename Parser::Result;

      //! Constructs a ParserPublisher.
      /*!
        \param reader Initializes the Reader to use.
        \param parser Initializes the Parser.
        \param errorPolicy The policy used to handle an error.
      */
      template<typename ReaderForward>
      ParserPublisher(ReaderForward&& reader, const Parser& parser,
        ParserErrorPolicy errorPolicy);

      virtual void With(const std::function<void ()>& f) const override final;

      virtual void Monitor(
        std::shared_ptr<QueueWriter<Source>> monitor) const override final;

    private:
      GetOptionalLocalPtr<ReaderType> m_reader;
      Parser m_parser;
      ParserErrorPolicy m_errorPolicy;
      MultiQueueWriter<Source> m_publisher;
      mutable std::atomic_bool m_isParsing;
      mutable Routines::RoutineHandler m_parseLoop;

      void ParseLoop();
  };

  template<typename ReaderType, typename ParserType>
  template<typename ReaderForward>
  ParserPublisher<ReaderType, ParserType>::ParserPublisher(
      ReaderForward&& reader, const Parser& parser,
      ParserErrorPolicy errorPolicy)
      : m_reader(std::forward<ReaderForward>(reader)),
        m_parser(parser),
        m_errorPolicy(errorPolicy),
        m_isParsing(false) {}

  template<typename ReaderType, typename ParserType>
  void ParserPublisher<ReaderType, ParserType>::With(
      const std::function<void ()>& f) const {
    m_publisher.With(f);
  }

  template<typename ReaderType, typename ParserType>
  void ParserPublisher<ReaderType, ParserType>::Monitor(
      std::shared_ptr<QueueWriter<Source>> monitor) const {
    m_publisher.Monitor(std::move(monitor));
    bool isParsing = m_isParsing.exchange(true);
    if(!isParsing) {
      m_parseLoop = Routines::Spawn(std::bind(
        &ParserPublisher::ParseLoop, const_cast<ParserPublisher*>(this)));
    }
  }

  template<typename ReaderType, typename ParserType>
  void ParserPublisher<ReaderType, ParserType>::ParseLoop() {
    ReaderParserStream<Reader*> stream(&*m_reader);
    Source value;
    while(m_parser.Read(stream, value)) {
      m_publisher.Push(std::move(value));
    }
  }
}
}

#endif
