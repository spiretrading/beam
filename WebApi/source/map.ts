declare function require(name:string): any;
const Collections = require('collections/fast-map');
import { fromJson, toJson } from '.';

function hash(value: any) {
  return value.hash().toString();
}

function equals(left: any, right: any) {
  if(left.equals !== undefined) {
    return left.equals(right);
  }
  return left === right;
}

/** Computes the hash of a string.
 * @param value - The string to hash.
 * @return The hash of the specified string.
 */
export function hashString(value: string): number {
  if(value.length === 0) {
    return 0;
  }
  let hash = 0;
  for(let i = 0; i < value.length; ++i) {
    hash = ((hash << 5) - hash) + value.charCodeAt(i);
    hash |= 0;
  }
  return hash;
}

/** Combines two hash values together.
 * @param seed - The initial hash value.
 * @param hash - The hash to combine with the seed.
 * @return The combined hash value.
 */
export function hashCombine(seed: number, hash: number): number {
  return seed ^ (hash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

/** Represents a hash-based associative container.
 * @template Key The type used as a key, this type should define a hash method
 *           and equals method to work correctly.
 * @template Value The type used as the value. Any value may be used other than
 *           undefined.
 */
export class Map<Key, Value> {

  /** Parses a Map from JSON. */
  public static fromJson<Key, Value>(keyType: any, valueType: any,
      value: any): Map<Key, Value> {
    value = value as [];
    const result = new Map<Key, Value>();
    for(let entry of value) {
      result.set(fromJson(keyType, entry[0]), fromJson(valueType, entry[1]));
    }
    return result;
  }

  /** Constructs an empty map. */
  constructor() {
    this._collection = new Collections.FastMap([], equals, hash);
  }

  /** Returns the value associated with a key.
   * @param key - The key used as an index to the value.
   * @return The value associated with the key, or undefined if no such key
   *         exists.
   */
  public get(key: Key): Value {
    return this._collection.get(key);
  }

  /** Associates a key with a value.
   * @param key - The value to use as the key.
   * @param value - The value to associate with the key.
   */
  public set(key: Key, value: Value): void {
    this._collection.set(key, value);
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    const value = [];
    const entries = this._collection.entries() as Iterator<[Key, Value]>;
    let i = entries.next();
    while(!i.done) {
      const entry = i.value;
      value.push(toJson(entry));
      i = entries.next();
    }
    return value;
  }

  private _collection: any;
}
