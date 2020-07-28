#ifndef BEAM_PERMISSIONS_HPP
#define BEAM_PERMISSIONS_HPP
#include "Beam/Collections/EnumSet.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

  /** Enumerates a DirectoryEntry's permissions. */
  BEAM_ENUM(Permission,

    /** Permission to read DirectoryEntries. */
    READ,

    /** Permission to move DirectoryEntries. */
    MOVE,

    /** Permissions to create/delete or modify DirectoryEntries. */
    ADMINISTRATE
  );

  /** A set of Permissions represented with a bitset. */
  using Permissions = EnumSet<Permission>;
}

#endif
