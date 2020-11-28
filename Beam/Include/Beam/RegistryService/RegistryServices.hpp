#ifndef BEAM_REGISTRY_SERVICES_HPP
#define BEAM_REGISTRY_SERVICES_HPP
#include <vector>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Services/Service.hpp"
#include "Beam/RegistryService/RegistryEntry.hpp"
#include "Beam/RegistryService/RegistryService.hpp"

namespace Beam::RegistryService {
  BEAM_DEFINE_SERVICES(RegistryServices,

    /**
     * Loads a RegistryEntry from a path.
     * @param root The RegistryEntry to load from.
     * @param path The path from the <i>root</i> to the RegistryEntry to load.
     * @return The RegistryEntry at the specified path from the <i>root</i>.
     */
    (LoadPathService, "Beam.RegistryService.LoadPathService", RegistryEntry,
      RegistryEntry, root, std::string, path),

    /**
     * Loads a RegistryEntry's parent.
     * @param registryEntry The RegistryEntry whose parent is to be loaded.
     * @return The <i>registryEntry</i>'s parent.
     */
    (LoadParentService, "Beam.RegistryService.LoadParentService", RegistryEntry,
      RegistryEntry, registryEntry),

    /**
     * Loads a RegistryEntry's children.
     * @param registryEntry The RegistryEntry whose children are to be loaded.
     * @return The <i>registryEntry</i>'s children.
     */
    (LoadChildrenService, "Beam.RegistryService.LoadChildrenService",
      std::vector<RegistryEntry>, RegistryEntry, registryEntry),

    /**
     * Creates a RegistryEntry directory.
     * @param name The name of the directory to make.
     * @param parent The directory's parent.
     * @return The RegistryEntry representing the new directory.
     */
    (MakeDirectoryService, "Beam.RegistryService.MakeDirectoryService",
      RegistryEntry, std::string, name, RegistryEntry, parent),

    /**
     * Copies a RegistryEntry.
     * @param registryEntry The RegistryEntry to copy.
     * @param destination Where to copy the RegistryEntry to.
     * @return The copy made of the RegistryEntry.
     */
    (CopyService, "Beam.RegistryService.CopyService", RegistryEntry,
      RegistryEntry, registryEntry, RegistryEntry, destination),

    /**
     * Moves a RegistryEntry.
     * @param registryEntry <code>RegistryEntry</code> The RegistryEntry to
     *        copy.
     * @param destination <code>RegistryEntry</code> Where to move the
     *        RegistryEntry to.
     */
    (MoveService, "Beam.RegistryService.MoveService", void, RegistryEntry,
      registryEntry, RegistryEntry, destination),

    /**
     * Loads a registry value.
     * @param registryEntry The RegistryEntry of the value to load.
     * @return The value associated with the <i>registryEntry</i>.
     */
    (LoadValueService, "Beam.RegistryService.LoadValueService",
      IO::SharedBuffer, RegistryEntry, registryEntry),

    /**
     * Creates a RegistryEntry value.
     * @param name The name of the registry value.
     * @param value The value to store.
     * @param parent The directory's parent.
     * @return The RegistryEntry representing the new value.
     */
    (MakeValueService, "Beam.RegistryService.MakeValueService", RegistryEntry,
      std::string, name, IO::SharedBuffer, value, RegistryEntry, parent),

    /**
     * Creates or updates a RegistryEntry value.
     * @param name The name of the registry value.
     * @param value The value to store.
     * @param parent The directory's parent.
     * @return The RegistryEntry representing the value.
     */
    (StoreValueService, "Beam.RegistryService.StoreValueService", RegistryEntry,
      std::string, name, IO::SharedBuffer, value, RegistryEntry, parent),

    /**
     * Deletes a RegistryEntry.
     * @param registryEntry The RegistryEntry to delete.
     */
    (DeleteService, "Beam.RegistryService.DeleteService", void, RegistryEntry,
      registryEntry));
}

#endif
