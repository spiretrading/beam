#ifndef BEAM_PYTHONTIMESERVICE_HPP
#define BEAM_PYTHONTIMESERVICE_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the tz_database class.
  void ExportTzDatabase();

  //! Exports the FixedTimeClient class.
  void ExportFixedTimeClient();

  //! Exports the IncrementalTimeClient class.
  void ExportIncrementalTimeClient();

  //! Exports the LocalTimeClient class.
  void ExportLocalTimeClient();

  //! Exports the LiveNtpTimeClient class.
  void ExportNtpTimeClient();

  //! Exports the TestTimeClient class.
  void ExportTestTimeClient();

  //! Exports the TestTimer class.
  void ExportTestTimer();

  //! Exports the TimeClient class.
  void ExportTimeClient();

  //! Exports the TimeService namespace.
  void ExportTimeService();

  //! Exports the TimeServiceTestEnvironment class.
  void ExportTimeServiceTestEnvironment();
}
}

#endif
