#include "Beam/Python/Beam.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace pybind11;

PYBIND11_MODULE(beam, m) {
  export_collections(m);
  export_io(m);
  export_codecs(m);
  export_json(m);
  export_network(m);
  export_queues(m);
  export_queries(m);
  export_routines(m);
  export_service_locator(m);
  export_sql(m);
  export_threading(m);
  export_time_service(m);
  export_uid_service(m);
  export_utilities(m);
  export_web_services(m);
}
