#ifndef BEAM_CUSTOMPARSER_HPP
#define BEAM_CUSTOMPARSER_HPP
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/RuleParser.hpp"

namespace Beam::Parsers {

  /*! \class CustomParser
      \brief Used as a base class to a custom rule based parser.
      \tparam R The data type storing the parsed value.
   */
  template<typename R>
  class CustomParser {
    public:
      using Result = R;

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;

    protected:
      template<typename Parser>
      void SetRule(Parser parser);

    private:
      RuleParser<Result> m_parser;
  };

  template<typename R>
  template<typename Stream>
  bool CustomParser<R>::Read(Stream& source, Result& value) const {
    return m_parser.Read(source, value);
  }

  template<typename R>
  template<typename Stream>
  bool CustomParser<R>::Read(Stream& source) const {
    return m_parser.Read(source);
  }

  template<typename R>
  template<typename Parser>
  void CustomParser<R>::SetRule(Parser parser) {
    m_parser.SetRule(std::move(parser));
  }
}

#endif
