#ifndef BEAM_PARSER_HPP
#define BEAM_PARSER_HPP
#include "Beam/Parsers/Parsers.hpp"
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
}

#endif
