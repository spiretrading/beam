#ifndef BEAM_UIDDATASTORE_HPP
#define BEAM_UIDDATASTORE_HPP
#include <cstdint>
#include "Beam/IO/Connection.hpp"
#include "Beam/UidService/UidService.hpp"

namespace Beam {
namespace UidService {

  /*! \class UidDataStore
      \brief Base class used to manage UIDs.
   */
  class UidDataStore : private boost::noncopyable {
    public:
      virtual ~UidDataStore() = default;

      //! Returns the next available UID.
      virtual std::uint64_t GetNextUid() = 0;

      //! Reserves a block of UIDs.
      /*!
        \param size The size of the UID block to reserve.
        \return The first UID in the reserved block.
      */
      virtual std::uint64_t Reserve(std::uint64_t size) = 0;

      //! Performs an atomic transaction.
      /*!
        \param transaction The transaction to perform.
      */
      virtual void WithTransaction(
        const std::function<void ()>& transaction) = 0;

      virtual void Open() = 0;

      virtual void Close() = 0;
  };
}
}

#endif
