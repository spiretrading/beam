#ifndef BEAM_EPSILONPARSER_HPP
#define BEAM_EPSILONPARSER_HPP
#include "Beam/Parsers/Parsers.hpp"

namespace Beam::Parsers {

  /*! \class EpsilonParser
      \brief Does not perform any read operations and always returns
             <code>true</code>.
   */
  class EpsilonParser {
    public:
      using Result = NullType;

      template<typename Stream>
      bool Read(Stream& source) const;
  };

  template<typename Stream>
  bool EpsilonParser::Read(Stream& source) const {
    return true;
  }
}

#endif
