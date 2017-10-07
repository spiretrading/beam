#ifndef BEAM_PYTHONIO_HPP
#define BEAM_PYTHONIO_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the Channel class.
  void ExportChannel();

  //! Exports the ChannelIdentifier class.
  void ExportChannelIdentifier();

  //! Exports the Connection class.
  void ExportConnection();

  //! Exports the IO namespace.
  void ExportIO();

  //! Exports the OpenState class.
  void ExportOpenState();

  //! Exports the Reader class.
  void ExportReader();

  //! Exports the SharedBuffer class.
  void ExportSharedBuffer();

  //! Exports the Writer class.
  void ExportWriter();
}
}

#endif
