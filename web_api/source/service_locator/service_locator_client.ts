import {DirectoryEntry} from './directory_entry';

/** Client used to access service locator services. */
export abstract class ServiceLocatorClient {

  /** Returns the directory entry representing the account currently logged
   *  in. */
  public abstract get account(): DirectoryEntry;

  /** Loads a directory entry from an id.
   * @param id - The id of the directory entry to load.
   * @return The directory entry with the specified id.
   */
  public abstract async load(id: number): Promise<DirectoryEntry>;

  /** Logs into the service locator.
   * @param username - The username.
   * @param password - The password.
   * @return The DirectoryEntry of the account that logged in.
   * @throws ServiceError Indicates the log in failed.
   */
  public abstract async login(username: string, password: string):
    Promise<DirectoryEntry>;

  /** Logs out of the service locator. */
  public abstract async logout(): Promise<void>;
}
