#ifndef BEAM_REPORTEXCEPTION_HPP
#define BEAM_REPORTEXCEPTION_HPP
#include <boost/exception/diagnostic_information.hpp>
#include "Beam/Utilities/Utilities.hpp"

#define BEAM_REPORT_CURRENT_EXCEPTION()                                        \
  __FILE__ << ":" << __LINE__ << " - " <<                                      \
  boost::current_exception_diagnostic_information()

#endif
