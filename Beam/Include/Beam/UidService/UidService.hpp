#ifndef BEAM_UID_SERVICE_HPP
#define BEAM_UID_SERVICE_HPP
#include <string>

namespace Beam::UidService {
  class LocalUidDataStore;
  template<typename C> class SqlUidDataStore;
  template<typename B> class UidClient;
  class UidClientBox;
  class UidDataStore;
  template<typename C, typename D> class UidServlet;

  /** Standard name for the uid service. */
  inline const auto SERVICE_NAME = std::string("uid_service");
}

#endif
