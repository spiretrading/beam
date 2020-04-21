#ifndef BEAM_DIFFERENCEPARSER_HPP
#define BEAM_DIFFERENCEPARSER_HPP
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/SubParserStream.hpp"
#include "Beam/Parsers/Traits.hpp"

namespace Beam::Parsers {

  /*! \class DifferenceParser
      \brief Matches if the left hand parser matches but not the right hand
             parser.
      \tparam L The parser that must match to the left.
      \tparam R The parser that must not match to the right.
   */
  template<typename L, typename R>
  class DifferenceParser {
    public:

      //! The parser that must match to the left.
      using LeftParser = L;

      //! The parser that must match to the right.
      using RightParser = R;

      using Result = parser_result_t<LeftParser>;

      DifferenceParser(LeftParser leftParser, RightParser rightParser)
        : m_leftParser(std::move(leftParser)),
          m_rightParser(std::move(rightParser)) {}

      template<typename Stream>
      bool Read(Stream& source, Result& value) const {
        {
          auto context = SubParserStream<Stream>(source);
          if(m_rightParser.Read(context)) {
            return false;
          }
        }
        if(m_leftParser.Read(source, value)) {
          return true;
        }
        return false;
      }

      template<typename Stream>
      bool Read(Stream& source) const {
        {
          auto context = SubParserStream<Stream>(source);
          if(m_rightParser.Read(context)) {
            return false;
          }
        }
        if(m_leftParser.Read(source)) {
          return true;
        }
        return false;
      }

    private:
      LeftParser m_leftParser;
      RightParser m_rightParser;
  };

  template<typename L, typename R>
  DifferenceParser(L, R) -> DifferenceParser<to_parser_t<L>, to_parser_t<R>>;
}

#endif
