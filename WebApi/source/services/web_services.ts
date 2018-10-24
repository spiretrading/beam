/** An interface for an object that can be converted to JSON. */
export interface Serializable {
  toJson(): any;
}

/** Converts an array to JSON.
 * @param value - The value to convert to JSON.
 */
export function arrayToJson<T extends Serializable>(
    value: Array<T>): Array<any> {
  const result = new Array<any>();
  for(let v of value) {
    result.push(v.toJson());
  }
  return result;
}

/** Constructs an array from JSON. */
export function arrayFromJson(type: any, value: Array<any>): Array<any> {
  let result = new Array<any>();
  for(let v of value) {
    result.push(type.fromJson(v));
  }
  return result;
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
          resolve();
        } else {
          resolve(JSON.parse(xhr.responseText));
        }
      } else {
        reject({
          status: this.status,
          statusText: xhr.statusText
        });
      }
    };
    if(parameters !== undefined) {
      xhr.send(JSON.stringify(parameters));
    }
  });
}
