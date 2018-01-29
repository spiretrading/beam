import {DirectoryEntry} from './directory_entry';

/** Client used to access service locator services. */
export abstract class ServiceLocatorClient {

  /** Returns the directory entry representing the account currently logged
   *  in. */
  public abstract get account(): DirectoryEntry;

  /** Logs into the service locator.
   * @param username - The username.
   * @param password - The password.
   * @return The DirectoryEntry of the account that logged in.
   * @throws ServiceError Indicates the log in failed.
   */
  public abstract async login(username: string, password: string):
    Promise<DirectoryEntry>;
}
