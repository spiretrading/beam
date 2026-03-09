declare function require(name:string): any;
const Collections = require('collections/fast-map');
import { equals, hash } from './comparators';
import { fromJson, toJson } from './serialization';

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

  /** Returns the number of entries in this map. */
  public get size(): number {
    return this._collection.length;
  }

  /** Returns the value associated with a key.
   * @param key - The key used as an index to the value.
   * @return The value associated with the key, or undefined if no such key
   *         exists.
   */
  public get(key: Key): Value {
    return this._collection.get(key);
  }

  /** Tests if a key exists in this map.
   * @param key - The key to test.
   * @return true iff the key exists in this map.
   */
  public has(key: Key): boolean {
    return this._collection.has(key);
  }

  /** Associates a key with a value.
   * @param key - The value to use as the key.
   * @param value - The value to associate with the key.
   */
  public set(key: Key, value: Value): void {
    this._collection.set(key, value);
  }

  /** Removes a key from this map.
   * @param key - The key to remove.
   */
  public remove(key: Key): void {
    this._collection.delete(key);
  }

  /** Removes a key from this map.
   * @param key - The key to remove.
   * @return true if the key was in the map.
   */
  public delete(key: Key): boolean {
    if(this._collection.has(key)) {
      this._collection.delete(key);
      return true;
    }
    return false;
  }

  /** Calls a function for each entry in the map.
   * @param callback - The function to call for each entry.
   */
  public forEach(callback: (value: Value, key: Key) => void): void {
    for(const [key, value] of this) {
      callback(value, key);
    }
  }

  /** Returns a shallow copy of this map. */
  public clone(): Map<Key, Value> {
    const clone = new Map<Key, Value>();
    for(const [key, value] of this) {
      clone.set(key, value);
    }
    return clone;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    const value = [];
    for(const entry of this) {
      value.push(toJson(entry));
    }
    return value;
  }

  [Symbol.iterator]() {
    return this._collection.entries() as Iterator<[Key, Value]>;
  }

  private _collection: any;
}
