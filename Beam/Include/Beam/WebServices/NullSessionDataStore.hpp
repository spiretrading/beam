#ifndef BEAM_NULL_SESSION_DATA_STORE_HPP
#define BEAM_NULL_SESSION_DATA_STORE_HPP
#include <boost/noncopyable.hpp>
#include "Beam/WebServices/Session.hpp"
#include "Beam/WebServices/SessionDataStore.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class NullSessionDataStore
      \brief A data store used to avoid persisting sessions.
   */
  class NullSessionDataStore : private boost::noncopyable {
    public:

      //! Constructs a NullSessionDataStore.
      NullSessionDataStore();

      ~NullSessionDataStore();

      template<typename SessionType>
      std::unique_ptr<SessionType> Load(const std::string& id);

      template<typename SessionType>
      void Store(const SessionType& session);

      template<typename SessionType>
      void Delete(const SessionType& session);

      template<typename F>
      void WithTransaction(F&& transaction);

      void Close();
  };

  inline NullSessionDataStore::NullSessionDataStore() {}

  inline NullSessionDataStore::~NullSessionDataStore() {
    Close();
  }

  template<typename SessionType>
  std::unique_ptr<SessionType> NullSessionDataStore::Load(
      const std::string& id) {
    return nullptr;
  }

  template<typename SessionType>
  void NullSessionDataStore::Store(const SessionType& session) {}

  template<typename SessionType>
  void NullSessionDataStore::Delete(const SessionType& session) {}

  template<typename F>
  void NullSessionDataStore::WithTransaction(F&& transaction) {
    transaction();
  }

  inline void NullSessionDataStore::Close() {}
}
}

#endif
