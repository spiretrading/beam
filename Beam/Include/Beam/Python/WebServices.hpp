#ifndef BEAM_PYTHONWEBSERVICES_HPP
#define BEAM_PYTHONWEBSERVICES_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  //! Exports the HttpHeader class.
  void ExportHttpHeader();

  //! Exports the HttpMethod enum.
  void ExportHttpMethod();

  //! Exports the HttpRequest class.
  void ExportHttpRequest();

  //! Exports the HttpResponse class.
  void ExportHttpResponse();

  //! Exports the Uri class.
  void ExportUri();

  //! Exports the WebServices namespace.
  void ExportWebServices();
}
}

#endif
