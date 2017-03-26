#ifndef BEAM_STREAMABLE_HPP
#define BEAM_STREAMABLE_HPP
#include <ostream>
#include <typeinfo>
#include <boost/core/demangle.hpp>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class Streamable
      \brief Interface for an object that can be output to a stream.
   */
  class Streamable {
    protected:

      //! Outputs a textual representation of this object to a stream.
      /*!
        \param out The stream to output the textual representation to.
        \return <i>out</i>.
      */
      virtual std::ostream& ToStream(std::ostream& out) const;

    private:
      friend std::ostream& operator <<(std::ostream& out,
        const Streamable& object);
  };

  inline std::ostream& Streamable::ToStream(std::ostream& out) const {
    return out << boost::core::demangle(typeid(*this).name());
  }

  inline std::ostream& operator <<(std::ostream& out,
      const Streamable& object) {
    return object.ToStream(out);
  }
}

#endif
