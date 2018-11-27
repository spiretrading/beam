import { ServiceError } from './service_error';

/** Submits a POST request to a web service.
 * @param url - The URL to submit the request to.
 * @param parameters - The object to encode as a JSON parameter.
 * @returns The object representing the response to the request.
 */
export async function post(url: string, parameters?: any): Promise<any> {
  var xhr = new XMLHttpRequest();
  xhr.open('POST', url);
  if(parameters !== undefined) {
    xhr.setRequestHeader('Content-Type', 'application/json');
  }
  return new Promise<any>((resolve, reject) => {
    xhr.onload = function() {
      if(xhr.status === 200) {
        if(xhr.responseText.length === 0) {
          resolve();
        } else {
          resolve(JSON.parse(xhr.responseText));
        }
      } else {
        reject(new ServiceError(xhr.statusText, xhr.status));
      }
    };
    if(parameters !== undefined) {
      xhr.send(JSON.stringify(parameters));
    }
  });
}
