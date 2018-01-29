import {DirectoryEntry} from './directory_entry';
import {ServiceLocatorClient} from './service_locator_client';
import {ServiceError} from '../services/service_error';
import * as web_services from '../services/web_services';

/** Implements the ServiceLocatorClient using web services. */
export class WebServiceLocatorClient extends ServiceLocatorClient {

  /** Constructs a WebServiceLocatorClient. */
  constructor() {
    super();
    this._account = DirectoryEntry.INVALID;
  }

  public get account(): DirectoryEntry {
    return this._account;
  }

  public async login(username: string, password: string):
      Promise<DirectoryEntry> {
    if(this._account != null) {
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
      throw new ServiceError(e.toString());
    }
  }

  private _account: DirectoryEntry;
}
