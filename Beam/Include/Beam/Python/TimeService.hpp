#ifndef BEAM_PYTHONTIMESERVICE_HPP
#define BEAM_PYTHONTIMESERVICE_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the tz_database class.
  void ExportTzDatabase();

  //! Exports the LiveNtpTimeClient class.
  void ExportTimeClient();

  //! Exports the TimeService namespace.
  void ExportTimeService();
}
}

#endif
