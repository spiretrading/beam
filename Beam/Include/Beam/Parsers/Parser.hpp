#ifndef BEAM_PARSER_HPP
#define BEAM_PARSER_HPP
#include <string>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/ParserException.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"
#include "Beam/Utilities/AssertionException.hpp"
#include "Beam/Utilities/Concept.hpp"
#include "Beam/Utilities/NullType.hpp"

namespace Beam::Parsers {

  /*! \struct Parser
      \brief Concept for parsing data from a stream.
      \tparam R The data type storing the parsed value.
   */
  template<typename R>
  struct Parser : Concept<Parser<R>> {

    //! The data type storing the parsed value.
    using Result = R;

    //! Parses the next value from a stream.
    /*!
      \param source The stream to parse from.
      \param result Stores the parsed value.
      \return <code>true</code> if a value was parsed.
    */
    template<typename Stream>
    bool Read(Stream& source, Result& result) const;

    //! Parses the next value from a stream.
    /*!
      \param source The stream to parse from.
      \return <code>true</code> if a value was parsed.
    */
    template<typename Stream>
    bool Read(Stream& source) const;
  };

  /*! \struct Parser
      \brief Concept for parsing data from a stream.
      \tparam R The data type storing the parsed value.
   */
  template<>
  struct Parser<NullType> : Concept<Parser<NullType>> {

    //! The data type storing the parsed value.
    using Result = NullType;

    //! Parses the next value from a stream.
    /*!
      \param source The stream to parse from.
      \return <code>true</code> iff the value was properly parsed.
    */
    template<typename Stream>
    bool Read(Stream& source) const;
  };

  //! Parses a value from a string.
  /*!
    \param source The string to parse.
    \return The parsed value.
  */
  template<typename Parser>
  auto Parse(const std::string& source) {
    auto value = typename Parser::Result();
    auto parser = Parser();
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
  auto Parse(const Buffer& source) {
    auto value = typename Parser::Result();
    auto stream = ReaderParserStream(source);
    auto parser = Parser();
    if(!parser.Read(stream, value)) {
      BOOST_THROW_EXCEPTION(ParserException("Invalid value."));
    }
    return value;
  }
}

#endif
