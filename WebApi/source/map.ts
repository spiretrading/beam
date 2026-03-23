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
    this._buckets = new globalThis.Map<string, [Key, Value][]>();
    this._size = 0;
  }

  /** Returns the number of entries in this map. */
  public get size(): number {
    return this._size;
  }

  /** Returns the value associated with a key.
   * @param key - The key used as an index to the value.
   * @return The value associated with the key, or undefined if no such key
   *         exists.
   */
  public get(key: Key): Value {
    const bucket = this._buckets.get(hash(key));
    if(bucket) {
      for(const [k, v] of bucket) {
        if(equals(k, key)) {
          return v;
        }
      }
    }
    return undefined;
  }

  /** Tests if a key exists in this map.
   * @param key - The key to test.
   * @return true iff the key exists in this map.
   */
  public has(key: Key): boolean {
    const bucket = this._buckets.get(hash(key));
    if(bucket) {
      for(const [k] of bucket) {
        if(equals(k, key)) {
          return true;
        }
      }
    }
    return false;
  }

  /** Associates a key with a value.
   * @param key - The value to use as the key.
   * @param value - The value to associate with the key.
   */
  public set(key: Key, value: Value): void {
    const h = hash(key);
    const bucket = this._buckets.get(h);
    if(bucket) {
      for(let i = 0; i < bucket.length; ++i) {
        if(equals(bucket[i][0], key)) {
          bucket[i] = [key, value];
          return;
        }
      }
      bucket.push([key, value]);
    } else {
      this._buckets.set(h, [[key, value]]);
    }
    ++this._size;
  }

  /** Removes a key from this map.
   * @param key - The key to remove.
   */
  public remove(key: Key): void {
    this.delete(key);
  }

  /** Removes a key from this map.
   * @param key - The key to remove.
   * @return true if the key was in the map.
   */
  public delete(key: Key): boolean {
    const h = hash(key);
    const bucket = this._buckets.get(h);
    if(bucket) {
      for(let i = 0; i < bucket.length; ++i) {
        if(equals(bucket[i][0], key)) {
          bucket.splice(i, 1);
          if(bucket.length === 0) {
            this._buckets.delete(h);
          }
          --this._size;
          return true;
        }
      }
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

  *[Symbol.iterator](): Iterator<[Key, Value]> {
    for(const bucket of this._buckets.values()) {
      for(const entry of bucket) {
        yield entry;
      }
    }
  }

  private _buckets: globalThis.Map<string, [Key, Value][]>;
  private _size: number;
}
