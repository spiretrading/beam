#ifndef BEAM_SCOPEDGROUPCONNECTION_HPP
#define BEAM_SCOPEDGROUPCONNECTION_HPP
#include "Beam/SignalHandling/GroupConnection.hpp"

namespace Beam {
namespace SignalHandling {

  /*! \class ScopedGroupConnection
      \brief Ensures proper disconnection of a GroupConnection upon destruction.
   */
  class ScopedGroupConnection : public GroupConnection,
      private boost::noncopyable {
    public:

      //! Constructs a ScopedGroupConnection.
      ScopedGroupConnection() = default;

      //! Constructs a ScopedGroupConnection from a GroupConnection.
      /*!
        \param connection The GroupConnection to copy the connections from.
      */
      ScopedGroupConnection(const GroupConnection& connection);

      ~ScopedGroupConnection();

      //! Assigns a GroupConnection to this.
      /*!
        \param rhs The right hand side of the assignment.
        \return A reference to this.
      */
      ScopedGroupConnection& operator =(const GroupConnection& rhs);
  };

  inline ScopedGroupConnection::ScopedGroupConnection(
      const GroupConnection& connection)
      : GroupConnection(connection) {}

  inline ScopedGroupConnection::~ScopedGroupConnection() {
    Disconnect();
  }

  inline ScopedGroupConnection& ScopedGroupConnection::operator =(
      const GroupConnection& rhs) {
    if(this == &rhs) {
      return *this;
    }
    Disconnect();
    GroupConnection::operator =(rhs);
    return *this;
  }
}
}

#endif
