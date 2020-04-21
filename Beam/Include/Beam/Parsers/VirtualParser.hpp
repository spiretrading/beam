#ifndef BEAM_VIRTUALPARSER_HPP
#define BEAM_VIRTUALPARSER_HPP
#include <memory>
#include <type_traits>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/VirtualParserStream.hpp"
#include "Beam/Utilities/NullType.hpp"

namespace Beam::Parsers {

  /*! \class VirtualParser
      \brief Implements a Parser using virtual methods.
      \tparam R The data type storing the parsed value.
  */
  template<typename R>
  class VirtualParser {
    public:
      using Result = R;

      virtual ~VirtualParser() = default;

      template<typename Stream>
      bool Read(Stream& source, Result& value) const;

      template<typename Stream>
      bool Read(Stream& source) const;

      virtual bool Read(VirtualParserStream& source, Result& value) const = 0;

      virtual bool Read(VirtualParserStream& source) const = 0;
  };

  template<>
  class VirtualParser<NullType> {
    public:
      using Result = NullType;

      virtual ~VirtualParser() = default;

      template<typename Stream>
      bool Read(Stream& source) const;

      virtual bool Read(VirtualParserStream& source) const = 0;
  };

  template<typename P, typename E>
  class WrapperParser {};

  template<typename P>
  class WrapperParser<P, std::enable_if_t<
      std::is_same_v<parser_result_t<P>, NullType>>> :
      public VirtualParser<NullType> {
    public:
      using Parser = P;
      using Result = NullType;

      WrapperParser(Parser parser)
        : m_parser(std::move(parser)) {}

      bool Read(VirtualParserStream& source) const override {
        return m_parser.Read(source);
      }

    private:
      Parser m_parser;
  };

  template<typename P>
  class WrapperParser<P, std::enable_if_t<
      !std::is_same_v<parser_result_t<P>, NullType>>> :
      public VirtualParser<parser_result_t<P>> {
    public:
      using Parser = P;
      using Result = parser_result_t<Parser>;

      WrapperParser(Parser parser)
        : m_parser(std::move(parser)) {}

      bool Read(VirtualParserStream& source, Result& value) const override {
        return m_parser.Read(source, value);
      }

      bool Read(VirtualParserStream& source) const override {
        return m_parser.Read(source);
      }

    private:
      Parser m_parser;
  };

  template<typename R>
  template<typename Stream>
  bool VirtualParser<R>::Read(Stream& source, Result& value) const {
    auto stream = WrapperParserStream(source);
    return Read(static_cast<VirtualParserStream&>(stream), value);
  }

  template<typename R>
  template<typename Stream>
  bool VirtualParser<R>::Read(Stream& source) const {
    auto stream = WrapperParserStream(source);
    return Read(static_cast<VirtualParserStream&>(stream));
  }

  template<typename Stream>
  bool VirtualParser<NullType>::Read(Stream& source) const {
    auto stream = WrapperParserStream(source);
    return Read(static_cast<VirtualParserStream&>(stream));
  }
}

#endif
