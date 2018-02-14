import {DirectoryEntry} from '.';
import {ServiceLocatorClient} from '.';
import {ServiceError} from '..';
import * as web_services from '..';

/** Implements the ServiceLocatorClient using web services. */
export class WebServiceLocatorClient extends ServiceLocatorClient {

  /** Constructs a WebServiceLocatorClient. */
  constructor() {
    super();
    this._account = DirectoryEntry.INVALID;
  }

  public async loadCurrentAccount(): Promise<DirectoryEntry> {
    if(!this._account.equals(DirectoryEntry.INVALID)) {
      return this._account;
    }
    try {
      let response = await web_services.post(
        '/api/service_locator/load_current_account', {});
      this._account = DirectoryEntry.fromJson(response);
      return this._account;
    } catch(e) {
      throw new ServiceError(e.statusText);
    }
  }

  public async loadDirectoryEntryFromId(id: number): Promise<DirectoryEntry> {
    if(id === this._account.id) {
      return this._account;
    }
    try {
      let response = await web_services.post(
        '/api/service_locator/load_directory_entry_from_id',
        {
          id: id
        });
      return DirectoryEntry.fromJson(response);
    } catch(e) {
      throw new ServiceError(e.statusText);
    }
  }

  public async login(username: string, password: string):
      Promise<DirectoryEntry> {
    if(!this._account.equals(DirectoryEntry.INVALID)) {
      return this._account;
    }
    try {
      let response = await web_services.post('/api/service_locator/login',
        {
          username: username,
          password: password
        });
      this._account = DirectoryEntry.fromJson(response);
      return this._account;
    } catch(e) {
      if(e.status === 401) {
        throw new ServiceError('Incorrect username or password.');
      }
      throw new ServiceError(e.statusText);
    }
  }

  public async close(): Promise<void> {
    if(this._account.equals(DirectoryEntry.INVALID)) {
      return;
    }
    try {
      let response = await web_services.post('/api/service_locator/logout', {});
      this._account = DirectoryEntry.INVALID;
      return;
    } catch(e) {
      throw new ServiceError(e.statusText);
    }
  }

  private _account: DirectoryEntry;
}
