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
  var xhr = new XMLHttpRequest();
  xhr.open('POST', url);
  if(parameters !== undefined) {
    xhr.setRequestHeader('Content-Type', 'application/json');
  }
  return new Promise<any>((resolve, reject) => {
    xhr.onload = function() {
      if(xhr.status === 200) {
        if(xhr.responseText.length === 0) {
          resolve(undefined);
        } else {
          resolve(JSON.parse(xhr.responseText));
        }
      } else {
        reject(new ServiceError(toErrorMessage(xhr), xhr.status));
      }
    };
    if(parameters !== undefined) {
      xhr.send(JSON.stringify(parameters));
    }
  });
}
