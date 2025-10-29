#ifndef BEAM_ENUMERATOR_PARSER_HPP
#define BEAM_ENUMERATOR_PARSER_HPP
#include <algorithm>
#include <concepts>
#include <iterator>
#include <type_traits>
#include <vector>
#include <boost/lexical_cast.hpp>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Parsers/ConversionParser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/SymbolParser.hpp"

namespace Beam {

  /**
   * Used to parse an Enumerator.
   * @tparam E The type of Enumerator to parse.
   */
  template<typename E>
  class EnumeratorParser {
    public:
      using Result = E;

      /**
       * Constructs an EnumeratorParser.
       * @param first An iterator to the first enumerated value.
       * @param last An iterator to one past the last enumerated value.
       * @param to_string The function used to convert the Enumerator to a
       *        string.
       */
      template<typename I1, typename I2, typename F> requires
        std::invocable<F, E> &&
        std::convertible_to<std::invoke_result_t<F, E>, std::string>
      EnumeratorParser(I1 first, I2 last, F to_string);

      /**
       * Constructs an EnumeratorParser.
       * @param first An iterator to the first enumerated value.
       * @param last An iterator to one past the last enumerated value.
       */
      template<typename I1, typename I2>
      EnumeratorParser(I1 first, I2 last);

      template<IsParserStream S>
      bool read(S& source, Result& value) const;
      template<IsParserStream S>
      bool read(S& source) const;

    private:
      struct EnumConverter {
        Result m_value;

        EnumConverter(Result value);
        Result operator ()() const;
      };
      std::vector<ConversionParser<SymbolParser, EnumConverter>> m_parsers;
  };

  template<typename I1, typename I2, typename F>
  EnumeratorParser(I1, I2, F) ->
    EnumeratorParser<typename std::iterator_traits<I1>::value_type>;

  template<typename I1, typename I2>
  EnumeratorParser(I1, I2) ->
    EnumeratorParser<typename std::iterator_traits<I1>::value_type>;

  template<typename E>
  EnumeratorParser<E>::EnumConverter::EnumConverter(Result value)
    : m_value(value) {}

  template<typename E>
  typename EnumeratorParser<E>::Result
      EnumeratorParser<E>::EnumConverter::operator ()() const {
    return m_value;
  }

  template<typename E>
  template<typename I1, typename I2, typename F> requires
    std::invocable<F, E> &&
    std::convertible_to<std::invoke_result_t<F, E>, std::string>
  EnumeratorParser<E>::EnumeratorParser(I1 first, I2 last, F to_string) {
    auto cases = std::vector<std::tuple<E, std::string>>();
    while(first != last) {
      cases.push_back(std::tuple(*first, to_string(*first)));
      ++first;
    }
    std::sort(cases.begin(), cases.end(),
      [] (const auto& lhs, const auto& rhs) {
        return std::get<1>(lhs).size() > std::get<1>(rhs).size();
      });
    for(auto& c : cases) {
      m_parsers.push_back(
        convert(SymbolParser(std::get<1>(c)), EnumConverter(std::get<0>(c))));
    }
  }

  template<typename E>
  template<typename I1, typename I2>
  EnumeratorParser<E>::EnumeratorParser(I1 first, I2 last)
    : EnumeratorParser(std::move(first), std::move(last),
        &boost::lexical_cast<std::string, Result>) {}

  template<typename E>
  template<IsParserStream S>
  bool EnumeratorParser<E>::read(S& source, Result& value) const {
    for(auto& parser : m_parsers) {
      auto context = SubParserStream<S>(source);
      if(parser.read(context, value)) {
        context.accept();
        return true;
      }
    }
    return false;
  }

  template<typename E>
  template<IsParserStream S>
  bool EnumeratorParser<E>::read(S& source) const {
    for(auto& parser : m_parsers) {
      auto context = SubParserStream<S>(source);
      if(parser.read(context)) {
        context.accept();
        return true;
      }
    }
    return false;
  }
}

#endif
