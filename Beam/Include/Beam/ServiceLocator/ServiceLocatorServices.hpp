#ifndef BEAM_SERVICELOCATORSERVICES_HPP
#define BEAM_SERVICELOCATORSERVICES_HPP
#include <boost/optional.hpp>
#include "Beam/Serialization/ShuttleBitset.hpp"
#include "Beam/Serialization/ShuttleDateTime.hpp"
#include "Beam/Serialization/ShuttleOptional.hpp"
#include "Beam/Serialization/ShuttleVector.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/Permissions.hpp"
#include "Beam/ServiceLocator/ServiceEntry.hpp"
#include "Beam/Services/RecordMessage.hpp"
#include "Beam/Services/Service.hpp"

namespace Beam {
namespace ServiceLocator {
  BEAM_DEFINE_RECORD(LoginServiceResult, DirectoryEntry, account,
    std::string, session_id);

  BEAM_DEFINE_SERVICES(ServiceLocatorServices,

    /*! \interface Beam::ServiceLocator::SendSessionIdService
        \brief Sends an encoded session id.
        \param key The key used to encode the session id.
        \param session_id The encoded session id.
    */
    //! \cond
    (SendSessionIdService, "Beam.ServiceLocator.SendSessionIdService", void,
      unsigned int, key, std::string, session_id),
    //! \endcond

    /*! \interface Beam::ServiceLocator::LoginService
        \brief Logs into the ServiceLocator.
        \param username <code>std::string</code> The account's username.
        \param password <code>std::string</code> The account's password.
        \return A record as follows:
                account: <code>DirectoryEntry</code> The DirectoryEntry
                         identifying the account that logged in.
                session_id: <code>std::string</code> The login's session id.
    */
    //! \cond
    (LoginService, "Beam.ServiceLocator.LoginService", LoginServiceResult,
      std::string, username, std::string, password),
    //! \endcond

    /*! \interface Beam::ServiceLocator::RegisterService
        \brief Registers a service with the ServiceLocator.
        \param name <code>std::string</code> The name of the service.
        \param properties <code>JsonObject</code> The service's properties.
        \return <code>ServiceEntry</code> A ServiceEntry representing the
                registered service.
    */
    //! \cond
    (RegisterService, "Beam.ServiceLocator.RegisterService", ServiceEntry,
      std::string, name, JsonObject, properties),
    //! \endcond

    /*! \interface Beam::ServiceLocator::UnregisterService
        \brief Unregisters a service from the ServiceLocator.
        \param service <code>int</code> The id of the service to unregister.
    */
    //! \cond
    (UnregisterService, "Beam.ServiceLocator.UnregisterService", void, int,
      service),
    //! \endcond

    /*! \interface Beam::ServiceLocator::LocateService
        \brief Locates a service.
        \param name <code>std::string</code> The name of the service to locate.
        \return <code>std::vector\<ServiceEntry\></code> A list of
                ServiceEntries for the service that was located.
    */
    //! \cond
    (LocateService, "Beam.ServiceLocator.LocateService",
      std::vector<ServiceEntry>, std::string, name),
    //! \endcond

    /*! \interface Beam::ServiceLocator::SubscribeAvailabilityService
        \brief Subscribes to notifications about the availability of a service.
        \param name <code>std::string</code> The name of the service to
                    subscribe to.
        \return <code>std::vector\<ServiceEntry\></code> A list of
                ServiceEntries for the services currently available with the
                specified <i>name</i>.
    */
    //! \cond
    (SubscribeAvailabilityService,
      "Beam.ServiceLocator.SubscribeAvailabilityService",
      std::vector<ServiceEntry>, std::string, name),
    //! \endcond

    /*! \interface Beam::ServiceLocator::UnsubscribeAvailabilityService
        \brief Unsubscribes from notifications about the availability of a
               service.
        \param name <code>std::string</code> The name of the service to
                    unsubscribe from.
    */
    //! \cond
    (UnsubscribeAvailabilityService,
      "Beam.ServiceLocator.UnsubscribeAvailabilityService", void, std::string,
      name),
    //! \endcond

    /*! \interface Beam::ServiceLocator::MonitorDirectoryEntryService
        \brief Requests update messages to a DirectoryEntry.
        \param entry <code>DirectoryEntry</code> The DirectoryEntry to monitor.
        \return The list of the <i>entry</i>'s parents.
    */
    //! \cond
    (MonitorDirectoryEntryService,
      "Beam.ServiceLocator.MonitorDirectoryEntryService",
      std::vector<DirectoryEntry>, DirectoryEntry, entry),
    //! \endcond

    /*! \interface Beam::ServiceLocator::LoadDirectoryEntryService
        \brief Loads a DirectoryEntry from its id.
        \param id <code>unsigned int</code> The id of the DirectoryEntry to
               load.
        \return <code>DirectoryEntry</code> The DirectoryEntry with the
                specified <i>id</i>.
    */
    //! \cond
    (LoadDirectoryEntryService,
      "Beam.ServiceLocator.LoadDirectoryEntryService", DirectoryEntry,
      unsigned int, id),
    //! \endcond

    /*! \interface Beam::ServiceLocator::LoadPathService
        \brief Loads a DirectoryEntry from a path.
        \param path <code>std::string</code> The path to the DirectoryEntry to
               load.
        \param root The root DirectoryEntry to search from.
        \return The DirectoryEntry at the specified <i>path</i>.
    */
    //! \cond
    (LoadPathService, "Beam.ServiceLocator.LoadPathService", DirectoryEntry,
      DirectoryEntry, root, std::string, path),
    //! \endcond

    /*! \interface Beam::ServiceLocator::LoadParentsService
        \brief Loads all of a DirectoryEntry's parents.
        \param entry <code>DirectoryEntry</code> The DirectoryEntry whose
               parents are to be loaded.
        \return <code>std::vector\<DirectoryEntry\></code> The list of parents.
    */
    //! \cond
    (LoadParentsService, "Beam.ServiceLocator.LoadParentsService",
      std::vector<DirectoryEntry>, DirectoryEntry, entry),
    //! \endcond

    /*! \interface Beam::ServiceLocator::LoadChildrenService
        \brief Loads all of a DirectoryEntry's children.
        \param entry <code>DirectoryEntry</code> The DirectoryEntry whose
               children are to be loaded.
        \return <code>std::vector\<DirectoryEntry\></code> The list of children.
    */
    //! \cond
    (LoadChildrenService, "Beam.ServiceLocator.LoadChildrenService",
      std::vector<DirectoryEntry>, DirectoryEntry, entry),
    //! \endcond

    /*! \interface Beam::ServiceLocator::LoadAllAccountsService
        \brief Loads all accounts the session is permissioned to read from.
        \param dummy <code>int</code> Unused.
        \return <code>std::vector\<DirectoryEntry\></code> The list of all
                accounts the session is permissioned to read from.
    */
    //! \cond
    (LoadAllAccountsService, "Beam.ServiceLocator.LoadAllAccountsService",
      std::vector<DirectoryEntry>, int, dummy),
    //! \endcond

    /*! \interface Beam::ServiceLocator::FindAccountService
        \brief Finds an account with a specified name.
        \param name <code>std::string</code> The name of the account to find.
        \return <code>boost::optional\<DirectoryEntry\></code> The
                DirectoryEntry of the account with the specified <i>name</i>.
    */
    //! \cond
    (FindAccountService, "Beam.ServiceLocator.FindAccountService",
      boost::optional<DirectoryEntry>, std::string, name),
    //! \endcond

    /*! \interface Beam::ServiceLocator::MakeAccountService
        \brief Creates an account.
        \param name <code>std::string</code> The name of the account.
        \param password <code>std::string</code> The password.
        \param parent <code>DirectoryEntry</code> The initial parent to place
               the DirectoryEntry.
        \return <code>DirectoryEntry</code> The DirectoryEntry of the account
                that was created.
    */
    //! \cond
    (MakeAccountService, "Beam.ServiceLocator.MakeAccountService",
      DirectoryEntry, std::string, name, std::string, password,
      DirectoryEntry, parent),
    //! \endcond

    /*! \interface Beam::ServiceLocator::MakeDirectoryService
        \brief Creates a directory.
        \param name <code>std::string</code> The name of the directory.
        \param parent <code>DirectoryEntry</code> The initial parent to place
               the DirectoryEntry.
        \return <code>DirectoryEntry</code> The DirectoryEntry of the directory
                that was created.
    */
    //! \cond
    (MakeDirectoryService, "Beam.ServiceLocator.MakeDirectoryService",
      DirectoryEntry, std::string, name, DirectoryEntry, parent),
    //! \endcond

    /*! \interface Beam::ServiceLocator::DeleteDirectoryEntryService
        \brief Deletes a DirectoryEntry.
        \param entry <code>DirectoryEntry</code> The DirectoryEntry to delete.
    */
    //! \cond
    (DeleteDirectoryEntryService,
      "Beam.ServiceLocator.DeleteDirectoryEntryService", void,
      DirectoryEntry, entry),
    //! \endcond

    /*! \interface Beam::ServiceLocator::AssociateService
        \brief Associates a DirectoryEntry with a parent.
        \param entry <code>DirectoryEntry</code> The DirectoryEntry to
               associate.
        \param parent <code>DirectoryEntry</code> The DirectoryEntry to serve as
               a parent of the <i>entry</i>.
    */
    //! \cond
    (AssociateService, "Beam.ServiceLocator.AssociateService", void,
      DirectoryEntry, entry, DirectoryEntry, parent),
    //! \endcond

    /*! \interface Beam::ServiceLocator::DetachService
        \brief Detaches a DirectoryEntry from a parent.
        \param entry <code>DirectoryEntry</code> The DirectoryEntry to detach.
        \param parent <code>DirectoryEntry</code> The parent DirectoryEntry to
               detach.
    */
    //! \cond
    (DetachService, "Beam.ServiceLocator.DetachService", void,
      DirectoryEntry, entry, DirectoryEntry, parent),
    //! \endcond

    /*! \interface Beam::ServiceLocator::StorePasswordService
        \brief Sets an account's password.
        \param account <code>DirectoryEntry</code> The account to set the
               password for.
        \param password <code>std::string</code> The <i>account</i>'s new
               password.
    */
    //! \cond
    (StorePasswordService, "Beam.ServiceLocator.StorePasswordService", void,
      DirectoryEntry, account, std::string, password),
    //! \endcond

    /*! \interface Beam::ServiceLocator::HasPermissionsService
        \brief Tests if an account has Permissions to a DirectoryEntry.
        \param account <code>DirectoryEntry</code> The account to test.
        \param target <code>DirectoryEntry</code> The DirectoryEntry to test.
        \param permissions <code>Permissions</code> The Permissions to test.
        \return <code>true</code> iff the <i>account</i> has the specified
                <code>permissions</code> on the <i>target</i>.
    */
    //! \cond
    (HasPermissionsService, "Beam.ServiceLocator.HasPermissionsService", bool,
      DirectoryEntry, account, DirectoryEntry, target, Permissions,
      permissions),
    //! \endcond

    /*! \interface Beam::ServiceLocator::StorePermissionsService
        \brief Sets a DirectoryEntry's Permissions to another DirectoryEntry.
        \param source <code>DirectoryEntry</code> The DirectoryEntry being
               granted or revoked of Permissions.
        \param target <code>DirectoryEntry</code> The DirectoryEntry whose
               Permissions are being granted or revoked from the <i>source</i>.
        \param permissions <code>Permissions</code> The Permissions to grant the
               <i>source</i> over the <i>target</i>.
    */
    //! \cond
    (StorePermissionsService, "Beam.ServiceLocator.StorePermissionsService",
      void, DirectoryEntry, source, DirectoryEntry, target, Permissions,
      permissions),
    //! \endcond

    /*! \interface Beam::ServiceLocator::LoadRegistrationTimeService
        \brief Loads the registration time of an account.
        \param account <code>DirectoryEntry</code> The account whose
               registration time is to be loaded.
        \return <code>boost::posix_time::ptime</code> The <i>account</i>'s
                time of registration.
    */
    //! \cond
    (LoadRegistrationTimeService,
      "Beam.ServiceLocator.LoadRegistrationTimeService",
      boost::posix_time::ptime, DirectoryEntry, account),
    //! \endcond

    /*! \interface Beam::ServiceLocator::LoadLastLoginTimeService
        \brief Loads an account's most recent login time.
        \param account <code>DirectoryEntry</code> The account whose last login
               time is to be loaded.
        \return <code>boost::posix_time::ptime</code> The <i>account</i>'s most
                recent login time.
    */
    //! \cond
    (LoadLastLoginTimeService, "Beam.ServiceLocator.LoadLastLoginTimeService",
      boost::posix_time::ptime, DirectoryEntry, account),
    //! \endcond

    /*! \interface Beam::ServiceLocator::RenameService
        \brief Renames a DirectoryEntry.
        \param entry <code>DirectoryEntry</code> The DirectoryEntry to rename.
        \param name <code>std::string</code> The name to assign to the
                    <i>entry</i>.
        \return <code>DirectoryEntry</code> The updated DirectoryEntry.
    */
    //! \cond
    (RenameService, "Beam.ServiceLocator.RenameService", DirectoryEntry,
      DirectoryEntry, entry, std::string, name),
    //! \endcond

    /*! \interface Beam::ServiceLocator::AuthenticateAccountService
        \brief Authenticates an account.
        \param username <code>std::string</code> The account's username.
        \param password <code>std::string</code> The account's password.
        \return <code>DirectoryEntry</code> The account's DirectoryEntry if the
                <i>username</i> and <i>password</i> match.
    */
    //! \cond
    (AuthenticateAccountService,
      "Beam.ServiceLocator.AuthenticateAccountService", DirectoryEntry,
      std::string, username, std::string, password),
    //! \endcond

    /*! \interface Beam::ServiceLocator::SessionAuthenticationService
        \brief Authenticates a session.
        \param session_id <code>std::string</code> The encrypted session id to
               authenticate.
        \param key The encryption key used to encode the session id.
        \return <code>DirectoryEntry</code> The DirectoryEntry associated with
                the session_id.
    */
    //! \cond
    (SessionAuthenticationService,
      "Beam.ServiceLocator.SessionAuthenticationService", DirectoryEntry,
      std::string, session_id, unsigned int, key));
    //! \endcond

  BEAM_DEFINE_MESSAGES(ServiceLocatorMessages,

    /*! \interface Beam::ServiceLocator::DirectoryEntryAssociatedMessage
        \brief Indicates one DirectoryEntry has been associated with another.
        \param entry <code>DirectoryEntry</code> The DirectoryEntry that's
               being associated.
        \param parent <code>DirectoryEntry</code> The parent the <i>entry</i>
               is now associated with.
    */
    //! \cond
    (DirectoryEntryAssociatedMessage,
      "Beam.ServiceLocator.DirectoryEntryAssociatedMessage",
      DirectoryEntry, entry, DirectoryEntry, parent),
    //! \endcond

    /*! \interface Beam::ServiceLocator::DirectoryEntryDetachedMessage
        \brief Indicates one DirectoryEntry has been detached from another.
        \param entry <code>DirectoryEntry</code> The DirectoryEntry that's
               detached.
        \param parent <code>DirectoryEntry</code> The parent the <i>entry</i>
               is now detached from.
    */
    //! \cond
    (DirectoryEntryDetachedMessage,
      "Beam.ServiceLocator.DirectoryEntryDetachedMessage",
      DirectoryEntry, entry, DirectoryEntry, parent),
    //! \endcond

    /*! \interface Beam::ServiceLocator::ServiceAvailabilityMessage
        \brief Indicates whether a service is available.
        \param service <code>ServiceEntry</code> The service.
        \param is_available <code>bool true</code> iff the <i>service</i> is
               available.
    */
    //! \cond
    (ServiceAvailabilityMessage,
      "Beam.ServiceLocator.ServiceAvailabilityMessage", ServiceEntry, service,
      bool, is_available));
    //! \endcond
}
}

#endif
