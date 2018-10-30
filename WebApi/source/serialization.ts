/** An interface for an object that can be converted to JSON. */
export interface Serializable {
  toJson(): any;
}

/** Converts an array to JSON.
 * @param value - The value to convert to JSON.
 */
export function arrayToJson<T extends Serializable>(value: T[]): any[] {
  const result = [];
  for(let v of value) {
    result.push(toJson(v));
  }
  return result;
}

/** Constructs an array from JSON. */
export function arrayFromJson(type: any, value: any[]): any[] {
  let result = [];
  for(let v of value) {
    result.push(fromJson(type, v));
  }
  return result;
}

/** Converts an object to JSON. */
export function toJson(value: any): any {
  if(value.toJson !== undefined) {
    return value.toJson(value);
  } else if(value.constructor === Array) {
    return arrayToJson(value);
  }
  return value;
}

/** Parses a JSON object. */
export function fromJson(type: any, value: any): any {
  if(type.fromJson !== undefined) {
    return type.fromJson(value);
  } else if(value.constructor === Array) {
    return arrayFromJson(type, value);
  }
  return value;
}
