#ifndef BEAM_REGISTRY_SERVICES_HPP
#define BEAM_REGISTRY_SERVICES_HPP
#include <vector>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Services/Service.hpp"
#include "Beam/RegistryService/RegistryEntry.hpp"
#include "Beam/RegistryService/RegistryService.hpp"

namespace Beam::RegistryService {
  BEAM_DEFINE_SERVICES(RegistryServices,

    /*! \interface Beam::RegistryService::LoadPathService
        \brief Loads a RegistryEntry from a path.
        \param root <code>RegistryEntry</code> The RegistryEntry to load from.
        \param path <code>std::string</code> The path from the <i>root</i> to
               the RegistryEntry to load.
        \return <code>RegistryEntry</code> The RegistryEntry at the specified
                path from the <i>root</i>.
    */
    //! \cond
    (LoadPathService, "Beam.RegistryService.LoadPathService", RegistryEntry,
      RegistryEntry, root, std::string, path),

    /*! \interface Beam::RegistryService::LoadParentService
        \brief Loads a RegistryEntry's parent.
        \param registryEntry <code>RegistryEntry</code> The RegistryEntry whose
               parent is to be loaded.
        \return <code>RegistryEntry</code> The <i>registryEntry</i>'s parent.
    */
    //! \cond
    (LoadParentService, "Beam.RegistryService.LoadParentService", RegistryEntry,
      RegistryEntry, registryEntry),

    /*! \interface Beam::RegistryService::LoadChildrenService
        \brief Loads a RegistryEntry's children.
        \param registryEntry <code>RegistryEntry</code> The RegistryEntry whose
               children are to be loaded.
        \return <code>std::vector\<RegistryEntry\></code> The
                <i>registryEntry</i>'s children.
    */
    //! \cond
    (LoadChildrenService, "Beam.RegistryService.LoadChildrenService",
      std::vector<RegistryEntry>, RegistryEntry, registryEntry),

    /*! \interface Beam::RegistryService::MakeDirectoryService
        \brief Creates a RegistryEntry directory.
        \param name <code>std::string</code> The name of the directory to make.
        \param parent <code>RegistryEntry</code> The directory's parent.
        \return <code>RegistryEntry</code> The RegistryEntry representing the
                new directory.
    */
    //! \cond
    (MakeDirectoryService, "Beam.RegistryService.MakeDirectoryService",
      RegistryEntry, std::string, name, RegistryEntry, parent),
    //! \endcond

    /*! \interface Beam::RegistryService::CopyService
        \brief Copies a RegistryEntry.
        \param registryEntry <code>RegistryEntry</code> The RegistryEntry to
               copy.
        \param destination <code>RegistryEntry</code> Where to copy the
               RegistryEntry to.
        \return <code>RegistryEntry</code> The copy made of the RegistryEntry.
    */
    //! \cond
    (CopyService, "Beam.RegistryService.CopyService", RegistryEntry,
      RegistryEntry, registryEntry, RegistryEntry, destination),
    //! \endcond

    /*! \interface Beam::RegistryService::MoveService
        \brief Moves a RegistryEntry.
        \param registryEntry <code>RegistryEntry</code> The RegistryEntry to
               copy.
        \param destination <code>RegistryEntry</code> Where to move the
               RegistryEntry to.
    */
    //! \cond
    (MoveService, "Beam.RegistryService.MoveService", void, RegistryEntry,
      registryEntry, RegistryEntry, destination),
    //! \endcond

    /*! \interface Beam::RegistryService::LoadValueService
        \brief Loads a registry value.
        \param registryEntry <code>RegistryEntry</code> The RegistryEntry of
               the value to load.
        \return <code>IO::SharedBuffer</code> The value associated with the
                <i>registryEntry</i>.
    */
    //! \cond
    (LoadValueService, "Beam.RegistryService.LoadValueService",
      IO::SharedBuffer, RegistryEntry, registryEntry),
    //! \endcond

    /*! \interface Beam::RegistryService::MakeValueService
        \brief Creates a RegistryEntry value.
        \param name <code>std::string</code> The name of the registry value.
        \param value <code>IO::SharedBuffer</code> The value to store.
        \param parent <code>RegistryEntry</code> The directory's parent.
        \return <code>RegistryEntry</code> The RegistryEntry representing the
                new value.
    */
    //! \cond
    (MakeValueService, "Beam.RegistryService.MakeValueService", RegistryEntry,
      std::string, name, IO::SharedBuffer, value, RegistryEntry, parent),
    //! \endcond

    /*! \interface Beam::RegistryService::StoreValueService
        \brief Creates or updates a RegistryEntry value.
        \param name <code>std::string</code> The name of the registry value.
        \param value <code>IO::SharedBuffer</code> The value to store.
        \param parent <code>RegistryEntry</code> The directory's parent.
        \return <code>RegistryEntry</code> The RegistryEntry representing the
                value.
    */
    //! \cond
    (StoreValueService, "Beam.RegistryService.StoreValueService", RegistryEntry,
      std::string, name, IO::SharedBuffer, value, RegistryEntry, parent),
    //! \endcond

    /*! \interface Beam::RegistryService::DeleteService
        \brief Deletes a RegistryEntry.
        \param registryEntry <code>RegistryEntry</code> The RegistryEntry to
               delete.
    */
    //! \cond
    (DeleteService, "Beam.RegistryService.DeleteService", void, RegistryEntry,
      registryEntry));
    //! \endcond
}

#endif
