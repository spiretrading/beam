import * as Services from '..';
import { DirectoryEntry } from './directory_entry';
import { ServiceLocatorClient } from './service_locator_client';

/** Implements the ServiceLocatorClient using HTTP requests. */
export class HttpServiceLocatorClient extends ServiceLocatorClient {

  /** Constructs an HttpServiceLocatorClient. */
  constructor() {
    super();
    this._account = DirectoryEntry.INVALID;
  }

  public async loadCurrentAccount(): Promise<DirectoryEntry> {
    if(!this._account.equals(DirectoryEntry.INVALID)) {
      return this._account;
    }
    let response = await Services.post(
      '/api/service_locator/load_current_account', {});
    this._account = DirectoryEntry.fromJson(response);
    return this._account;
  }

  public async loadDirectoryEntryFromId(id: number): Promise<DirectoryEntry> {
    if(id === this._account.id) {
      return this._account;
    }
    let response = await Services.post(
      '/api/service_locator/load_directory_entry_from_id',
      {
        id: id
      });
    return DirectoryEntry.fromJson(response);
  }

  public async login(username: string, password: string):
      Promise<DirectoryEntry> {
    if(!this._account.equals(DirectoryEntry.INVALID)) {
      return this._account;
    }
    try {
      let response = await Services.post('/api/service_locator/login',
        {
          username: username,
          password: password
        });
      this._account = DirectoryEntry.fromJson(response);
      return this._account;
    } catch(e) {
      if(e.code === 401) {
        throw new Services.ServiceError('Incorrect username or password.');
      }
      throw e;
    }
  }

  public async storePassword(account: DirectoryEntry,
      password: string): Promise<void> {
    await Services.post('/api/service_locator/store_password',
      {
        account: account.toJson(),
        password: password
      });
  }

  public async close(): Promise<void> {
    if(this._account.equals(DirectoryEntry.INVALID)) {
      return;
    }
    await Services.post('/api/service_locator/logout', {});
    this._account = DirectoryEntry.INVALID;
  }

  private _account: DirectoryEntry;
}
