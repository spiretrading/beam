#ifndef BEAM_FILESYSTEMREGISTRYDATASTOREDETAILS_HPP
#define BEAM_FILESYSTEMREGISTRYDATASTOREDETAILS_HPP
#include <vector>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/RegistryService/RegistryEntry.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace RegistryService {
namespace Details {
  struct RegistryEntryRecord {
    RegistryEntry m_registryEntry;
    std::uint64_t m_parent;
    std::vector<std::uint64_t> m_children;
    IO::SharedBuffer m_value;
  };
}
}
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<RegistryService::Details::RegistryEntryRecord> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle,
        RegistryService::Details::RegistryEntryRecord& value,
        unsigned int version) const {
      shuttle.Shuttle("registry_entry", value.m_registryEntry);
      shuttle.Shuttle("parent", value.m_parent);
      shuttle.Shuttle("children", value.m_children);
      shuttle.Shuttle("value", value.m_value);
    }
  };
}
}

#endif
