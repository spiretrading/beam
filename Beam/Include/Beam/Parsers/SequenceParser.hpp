#ifndef BEAM_SEQUENCE_PARSER_HPP
#define BEAM_SEQUENCE_PARSER_HPP
#include <vector>
#include "Beam/Parsers/ParserTraits.hpp"
#include "Beam/Parsers/SkipSpaceParser.hpp"
#include "Beam/Parsers/SubParserStream.hpp"

namespace Beam {

  /**
   * Matches a sequence of Parsers seperated by a delimiter.
   * @tparam P The Parser used for each value in the list.
   */
  template<IsParser P>
  class SequenceParser {
    public:

      /** The Parser used for each value in the list. */
      using Parser = P;
  };

  template<typename P>
  SequenceParser(std::vector<P>, char) -> SequenceParser<to_parser_t<P>>;

  /**
   * Returns a SequenceParser based on the list of Parsers passed to it.
   * @param parsers The sequence of Parsers to match.
   * @param delimiter The delimiter used to separate list items.
   */
  template<IsParser P>
  auto sequence(std::vector<P> parsers, char delimiter) {
    return SequenceParser(std::move(parsers), delimiter);
  }

  template<IsParser P> requires(!std::same_as<parser_result_t<P>, void>)
  class SequenceParser<P> {
    public:
      using Parser = P;
      using Result = std::vector<parser_result_t<Parser>>;

      /**
       * Constructs a SequenceParser.
       * @param parsers The sequence of Parsers to match.
       * @param delimiter The delimiter used to separate list items.
       */
      SequenceParser(std::vector<Parser> parsers, char delimiter)
        : m_parsers(std::move(parsers)),
          m_delimiter(delimiter) {}

      template<IsParserStream S>
      bool read(S& source, Result& value) const {
        auto context = SubParserStream<S>(source);
        value.clear();
        for(auto i = m_parsers.begin(); i != m_parsers.end(); ++i) {
          SkipSpaceParser().read(context);
          if(i != m_parsers.begin()) {
            if(!context.read()) {
              return false;
            }
            if(context.peek() != m_delimiter) {
              return false;
            }
            SkipSpaceParser().read(context);
          }
          auto list_value = parser_result_t<Parser>();
          if(!i->read(context, list_value)) {
            return false;
          }
          value.push_back(std::move(list_value));
        }
        context.accept();
        return true;
      }

      template<IsParserStream S>
      bool read(S& source) const {
        auto context = SubParserStream<S>(source);
        for(auto i = m_parsers.begin(); i != m_parsers.end(); ++i) {
          SkipSpaceParser().read(context);
          if(i != m_parsers.begin()) {
            if(!context.read()) {
              return false;
            }
            if(context.peek() != m_delimiter) {
              return false;
            }
            SkipSpaceParser().read(context);
          }
          if(!i->read(context)) {
            return false;
          }
        }
        context.accept();
        return true;
      }

    private:
      std::vector<Parser> m_parsers;
      char m_delimiter;
  };

  template<IsParserOf<void> P>
  class SequenceParser<P> {
    public:
      using Parser = P;
      using Result = void;

      SequenceParser(std::vector<Parser> parsers, char delimiter)
        : m_parsers(std::move(parsers)),
          m_delimiter(delimiter) {}

      template<IsParserStream S>
      bool read(S& source) const {
        auto context = SubParserStream<S>(source);
        for(auto i = m_parsers.begin(); i != m_parsers.end(); ++i) {
          SkipSpaceParser().read(context);
          if(i != m_parsers.begin()) {
            if(!context.read()) {
              return false;
            }
            if(context.peek() != m_delimiter) {
              return false;
            }
            SkipSpaceParser().read(context);
          }
          if(!i->read(context)) {
            return false;
          }
        }
        context.accept();
        return true;
      }

    private:
      std::vector<Parser> m_parsers;
      char m_delimiter;
  };
}

#endif
