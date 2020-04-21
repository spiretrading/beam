#ifndef BEAM_ENUMERATORPARSER_HPP
#define BEAM_ENUMERATORPARSER_HPP
#include <iterator>
#include <vector>
#include <boost/lexical_cast.hpp>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/SymbolParser.hpp"

namespace Beam::Parsers {

  /*! \class EnumeratorParser
      \brief Used to parse an Enumerator.
      \tparam E The type of Enumerator to parse.
   */
  template<typename E>
  class EnumeratorParser {
    public:
      using Result = E;

      //! Constructs an EnumeratorParser.
      /*!
        \param first An iterator to the first enumerated value.
        \param last An iterator to one past the last enumerated value.
        \param toString The function used to convert the Enumerator to a string.
      */
      template<typename Iterator, typename F>
      EnumeratorParser(Iterator first, Iterator last, F toString);

      //! Constructs an EnumeratorParser.
      /*!
        \param first An iterator to the first enumerated value.
        \param last An iterator to one past the last enumerated value.
      */
      template<typename Iterator>
      EnumeratorParser(Iterator first, Iterator last);

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;

    private:
      struct EnumConverter {
        Result m_value;

        EnumConverter(Result value);

        Result operator ()() const;
      };
      std::vector<ConversionParser<SymbolParser, EnumConverter>> m_parsers;
  };

  template<typename Iterator, typename F>
  EnumeratorParser(Iterator, Iterator, F) ->
    EnumeratorParser<typename std::iterator_traits<Iterator>::value_type>;

  template<typename E>
  EnumeratorParser<E>::EnumConverter::EnumConverter(Result value)
    : m_value(value) {}

  template<typename E>
  typename EnumeratorParser<E>::Result
      EnumeratorParser<E>::EnumConverter::operator ()() const {
    return m_value;
  }

  template<typename E>
  template<typename Iterator, typename F>
  EnumeratorParser<E>::EnumeratorParser(Iterator first, Iterator last,
      F toString) {
    while(first != last) {
      m_parsers.push_back(Convert(SymbolParser(toString(*first)),
        EnumConverter(*first)));
      ++first;
    }
  }

  template<typename E>
  template<typename Iterator>
  EnumeratorParser<E>::EnumeratorParser(Iterator first, Iterator last)
    : EnumeratorParser(first, last,
        &boost::lexical_cast<std::string, Result>) {}

  template<typename E>
  template<typename Stream>
  bool EnumeratorParser<E>::Read(Stream& source, Result& value) const {
    for(auto& parser : m_parsers) {
      auto context = SubParserStream<Stream>(source);
      if(parser.Read(context, value)) {
        context.Accept();
        return true;
      }
    }
    return false;
  }

  template<typename E>
  template<typename Stream>
  bool EnumeratorParser<E>::Read(Stream& source) const {
    for(auto& parser : m_parsers) {
      auto context = SubParserStream<Stream>(source);
      if(parser.Read(context)) {
        context.Accept();
        return true;
      }
    }
    return false;
  }
}

#endif
