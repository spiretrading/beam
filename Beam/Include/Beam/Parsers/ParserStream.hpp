#ifndef BEAM_PARSERSTREAM_HPP
#define BEAM_PARSERSTREAM_HPP
#include <cstddef>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Utilities/Concept.hpp"

namespace Beam::Parsers {

  /*! \struct ParserStream
      \brief Concept for a source used to parse data from.
   */
  struct ParserStream : Concept<ParserStream> {

    //! Returns the last character read from the stream.
    char GetChar() const;

    //! Reads the next character in the stream.
    /*!
      \return <code>true</code> iff a character was successfully read from the
              stream.
    */
    bool Read();

    //! Puts back the last character read.
    void Undo();

    //! Puts back multiple characters last read.
    /*!
      \param count The number of characters to put back.
    */
    void Undo(std::size_t count);

    //! Used to flush the stream's buffer.
    void Accept();
  };
}

#endif
