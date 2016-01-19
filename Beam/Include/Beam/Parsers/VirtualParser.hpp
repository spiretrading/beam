#ifndef BEAM_VIRTUALPARSER_HPP
#define BEAM_VIRTUALPARSER_HPP
#include <memory>
#include "Beam/Parsers/Parser.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/VirtualParserStream.hpp"

namespace Beam {
namespace Parsers {

  /*! \class VirtualParser
      \brief Implements a Parser using virtual methods.
      \tparam ResultType The data type storing the parsed value.
  */
  template<typename ResultType>
  class VirtualParser {
    public:
      typedef ResultType Result;

      virtual ~VirtualParser();

      virtual bool Read(VirtualParserStream& source, Result& value) = 0;

      virtual bool Read(VirtualParserStream& source) = 0;
  };

  template<>
  class VirtualParser<NullType> {
    public:
      typedef NullType Result;

      virtual ~VirtualParser();

      virtual bool Read(VirtualParserStream& source) = 0;
  };

  template<typename ParserType, typename Enabled>
  class WrapperParser {};

  template<typename ParserType>
  class WrapperParser<ParserType, typename std::enable_if<
      std::is_same<typename ParserType::Result, NullType>::value>::type> :
      public VirtualParser<typename ParserType::Result> {
    public:
      typedef ParserType Parser;
      typedef NullType Result;

      WrapperParser(const Parser& parser)
          : m_parser(parser) {}

      virtual bool Read(VirtualParserStream& source) {
        return m_parser.Read(source);
      }

    private:
      Parser m_parser;
  };

  template<typename ParserType>
  class WrapperParser<ParserType, typename std::enable_if<
      !std::is_same<typename ParserType::Result, NullType>::value>::type> :
      public VirtualParser<typename ParserType::Result> {
    public:
      typedef ParserType Parser;
      typedef typename Parser::Result Result;

      WrapperParser(const Parser& parser)
          : m_parser(parser) {}

      virtual bool Read(VirtualParserStream& source, Result& value) {
        return m_parser.Read(source, value);
      }

      virtual bool Read(VirtualParserStream& source) {
        return m_parser.Read(source);
      }

    private:
      Parser m_parser;
  };

  template<typename ResultType>
  VirtualParser<ResultType>::~VirtualParser() {}

  inline VirtualParser<NullType>::~VirtualParser() {}
}
}

#endif
