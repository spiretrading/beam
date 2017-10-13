#ifndef BEAM_REACTOR_ERROR_HPP
#define BEAM_REACTOR_ERROR_HPP
#include "Beam/Reactors/ReactorException.hpp"
#include "Beam/Reactors/Reactors.hpp"

namespace Beam {
namespace Reactors {

  /*! \class ReactorError
      \brief Signals an error that can not be recovered from.
   */
  class ReactorError : public ReactorException {
    public:

      //! Constructs a ReactorError.
      ReactorError() = default;

      //! Constructs a ReactorError.
      /*!
        \param message A message describing the error.
      */
      ReactorError(std::string message);

      virtual ~ReactorError() throw();
  };

  inline ReactorError::ReactorError(std::string message)
      : ReactorException{std::move(message)} {}

  inline ReactorError::~ReactorError() throw() {}
}
}

#endif
