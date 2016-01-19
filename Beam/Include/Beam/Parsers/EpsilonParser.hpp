#ifndef BEAM_EPSILONPARSER_HPP
#define BEAM_EPSILONPARSER_HPP
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"

namespace Beam {
namespace Parsers {

  /*! \class EpsilonParser
      \brief Does not perform any read operations and always returns
             <code>true</code>.
   */
  class EpsilonParser : public ParserOperators {
    public:
      typedef NullType Result;

      template<typename ParserStreamType>
      bool Read(ParserStreamType& source);
  };

  template<typename ParserStreamType>
  inline bool EpsilonParser::Read(ParserStreamType& source) {
    return true;
  }
}
}

#endif
