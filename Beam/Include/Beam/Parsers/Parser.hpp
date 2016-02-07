#ifndef BEAM_PARSER_HPP
#define BEAM_PARSER_HPP
#include <boost/optional/optional.hpp>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/ParserException.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Utilities/AssertionException.hpp"
#include "Beam/Utilities/Concept.hpp"
#include "Beam/Utilities/NullType.hpp"

namespace Beam {
namespace Parsers {
namespace Details {
  template<typename T>
  struct GetParserReadResultHelper {
    typedef boost::optional<T> type;
  };

  template<>
  struct GetParserReadResultHelper<NullType> {
    typedef bool type;
  };
}

  /*! \struct GetParserType
      \brief Type trait that returns the canonical Parser for a given type.
      \tparam T The type to get the canonical Parser for.
   */
  template<typename T>
  struct GetParserType {

    //! The canonical Parser for the specified type.
    typedef T type;
  };

  /*! \struct GetParserReadResult
      \brief Type trait that returns the type of a Parser's Read method.
      \tparam T The type of Parser to inspect.
   */
  template<typename T>
  struct GetParserReadResult {

    //! The return type of the Parser's Read method.
    typedef typename Details::GetParserReadResultHelper<
      typename GetParserType<T>::type::Result>::type type;
  };

  /*! \struct Parser
      \brief Concept for parsing data from a stream.
      \tparam ResultType The data type storing the parsed value.
   */
  template<typename ResultType>
  struct Parser : Concept<Parser<ResultType>> {

    //! The data type storing the parsed value.
    typedef ResultType Result;

    //! Parses the next value from a stream.
    /*!
      \param source The stream to parse from.
      \return The parsed value if it exists.
    */
    template<typename ParserStreamType>
    boost::optional<Result> Read(ParserStreamType& source);
  };

  /*! \struct Parser
      \brief Concept for parsing data from a stream.
      \tparam ResultType The data type storing the parsed value.
   */
  template<>
  struct Parser<NullType> : Concept<Parser<NullType>> {

    //! The data type storing the parsed value.
    typedef NullType Result;

    //! Parses the next value from a stream.
    /*!
      \param source The stream to parse from.
      \return <code>true</code> iff the value was properly parsed.
    */
    template<typename ParserStreamType>
    bool Read(ParserStreamType& source);
  };

  //! Parses a value from a string.
  /*!
    \param source The string to parse.
    \return The parsed value.
  */
  template<typename Parser>
  typename Parser::Result Parse(const std::string& source) {
    typename Parser::Result value;
    Parser parser;
    auto stream = ParserStreamFromString(source);
    if(!parser.Read(stream, value)) {
      BOOST_THROW_EXCEPTION(ParserException("Invalid value."));
    }
    return value;
  }

  //! Parses a value from a buffer.
  /*!
    \param source The string to parse.
    \return The parsed value.
  */
  template<typename Parser, typename Buffer>
  typename Parser::Result Parse(const Buffer& source) {
    typename Parser::Result value;
    ReaderParserStream<IO::BufferReader<Buffer>> stream{source};
    Parser parser;
    if(!parser.Read(stream, value)) {
      BOOST_THROW_EXCEPTION(ParserException("Invalid value."));
    }
    return value;
  }

  // TODO
  class ParserOperators {};
}
}

#endif
