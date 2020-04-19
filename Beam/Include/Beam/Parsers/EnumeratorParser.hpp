#ifndef BEAM_ENUMERATORPARSER_HPP
#define BEAM_ENUMERATORPARSER_HPP
#include <vector>
#include <boost/lexical_cast.hpp>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Parsers/Operators.hpp"
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/SymbolParser.hpp"

namespace Beam {
namespace Parsers {

  /*! \class EnumeratorParser
      \brief Used to parse an Enumerator.
      \tparam EnumeratorType The type of Enumerator to parse.
   */
  template<typename EnumeratorType>
  class EnumeratorParser : public ParserOperators {
    public:
      typedef EnumeratorType Result;

      //! Constructs an EnumeratorParser.
      /*!
        \param first An iterator to the first enumerated value.
        \param last An iterator to one past the last enumerated value.
        \param toString The function used to convert the Enumerator to a string.
      */
      template<typename Iterator, typename F>
      EnumeratorParser(const Iterator& first, const Iterator& last, F toString);

      //! Constructs an EnumeratorParser.
      /*!
        \param first An iterator to the first enumerated value.
        \param last An iterator to one past the last enumerated value.
      */
      template<typename Iterator>
      EnumeratorParser(const Iterator& first, const Iterator& last);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source, Result& value);

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);

    private:
      struct EnumConverter {
        Result m_value;

        EnumConverter(Result value);

        Result operator ()() const;
      };
      std::vector<ConversionParser<SymbolParser, EnumConverter>> m_parsers;
  };

  template<typename EnumeratorType>
  EnumeratorParser<EnumeratorType>::EnumConverter::EnumConverter(Result value)
    : m_value(value) {}

  template<typename EnumeratorType>
  typename EnumeratorParser<EnumeratorType>::Result
      EnumeratorParser<EnumeratorType>::EnumConverter::operator ()() const {
    return m_value;
  }

  template<typename EnumeratorType>
  template<typename Iterator, typename F>
  EnumeratorParser<EnumeratorType>::EnumeratorParser(const Iterator& first,
      const Iterator& last, F toString) {
    for(auto i = first; i != last; ++i) {
      m_parsers.push_back(Convert(SymbolParser(toString(*i)),
        EnumConverter(*i)));
    }
  }

  template<typename EnumeratorType>
  template<typename Iterator>
  EnumeratorParser<EnumeratorType>::EnumeratorParser(const Iterator& first,
    const Iterator& last)
    : EnumeratorParser(first, last,
        &boost::lexical_cast<std::string, Result>) {}

  template<typename EnumeratorType>
  template<typename ParserStreamType>
  bool EnumeratorParser<EnumeratorType>::Read(ParserStreamType& source,
      Result& value) {
    for(ConversionParser<SymbolParser, EnumConverter>& parser : m_parsers) {
      SubParserStream<ParserStreamType> context(source);
      if(parser.Read(context, value)) {
        context.Accept();
        return true;
      }
    }
    return false;
  }

  template<typename EnumeratorType>
  template<typename ParserStreamType>
  bool EnumeratorParser<EnumeratorType>::Read(ParserStreamType& source) {
    for(ConversionParser<SymbolParser, EnumConverter>& parser : m_parsers) {
      SubParserStream<ParserStreamType> context(source);
      if(parser.Read(context)) {
        context.Accept();
        return true;
      }
    }
    return false;
  }
}
}

#endif
