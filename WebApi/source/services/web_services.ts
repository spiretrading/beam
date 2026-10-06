import { ServiceError } from './service_error';

function toErrorMessage(xhr: XMLHttpRequest): string {
  if(xhr.responseText.length === 0) {
    return xhr.statusText;
  }
  try {
    const body = JSON.parse(xhr.responseText);
    if(typeof body === 'string') {
      return body;
    } else if(body !== null && typeof body.message === 'string') {
      return body.message;
    } else if(body !== null && typeof body.error === 'string') {
      return body.error;
    }
  } catch(error) {
    return xhr.responseText;
  }
  return xhr.statusText;
}

/** Submits a POST request to a web service.
 * @param url - The URL to submit the request to.
 * @param parameters - The object to encode as a JSON parameter.
 * @returns The object representing the response to the request.
 */
export async function post(url: string, parameters?: any): Promise<any> {
  const xhr = new XMLHttpRequest();
  xhr.open('POST', url);
  if(parameters !== undefined) {
    xhr.setRequestHeader('Content-Type', 'application/json');
  }
  return new Promise<any>((resolve, reject) => {
    xhr.onload = () => {
      if(xhr.status >= 200 && xhr.status < 300) {
        try {
          if(xhr.responseText.length === 0) {
            resolve(undefined);
          } else {
            resolve(JSON.parse(xhr.responseText));
          }
        } catch(error) {
          reject(error);
        }
      } else {
        reject(new ServiceError(toErrorMessage(xhr), xhr.status));
      }
    };
    xhr.onerror = () => reject(new ServiceError('Network request failed.'));
    xhr.onabort = () => reject(new ServiceError('Request aborted.'));
    xhr.ontimeout = () => reject(new ServiceError('Request timed out.'));
    if(parameters !== undefined) {
      xhr.send(JSON.stringify(parameters));
    } else {
      xhr.send();
    }
  });
}
