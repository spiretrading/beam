#ifndef BEAM_BASICPARSER_HPP
#define BEAM_BASICPARSER_HPP
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/RuleParser.hpp"

namespace Beam::Parsers {

  /*! \class BasicParser
      \brief Simplifies building a custom Parser.
      \tparam R The data type storing the parsed value.
   */
  template<typename R>
  class BasicParser : public ParserOperators {
    public:
      using Result = R;

      //! Constructs a BasicParser.
      BasicParser();

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;

    protected:

      //! Provides the definition of this parser.
      /*!
        \param parser Specifies the definition of this parser.
      */
      template<typename Parser>
      void SetParser(Parser parser);

    private:
      RuleParser<Result> m_parser;
  };

  template<typename R>
  BasicParser<R>::BasicParser() {}

  template<typename R>
  template<typename Stream>
  bool BasicParser<R>::Read(Stream& source, Result& value) const {
    return m_parser.Read(source, value);
  }

  template<typename R>
  template<typename Stream>
  bool BasicParser<R>::Read(Stream& source) const {
    return m_parser.Read(source);
  }

  template<typename R>
  template<typename Parser>
  void BasicParser<R>::SetParser(Parser parser) {
    m_parser = std::move(parser);
  }
}

#endif
