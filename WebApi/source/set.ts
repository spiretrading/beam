declare function require(name:string): any;
const Collections = require('collections/fast-set');
import { equals, hash } from './comparators';
import { fromJson, toJson } from './serialization';

/** Represents a hash-based associative container.
 * @template Key The type used as a key, this type should define a hash method
 *           and equals method to work correctly.
 */
export class Set<Key> {

  /** Parses a Set from JSON. */
  public static fromJson<Key>(keyType: any, keys: any): Set<Key> {
    keys = keys as [];
    const result = new Set<Key>();
    for(let key of keys) {
      result.add(fromJson(keyType, key));
    }
    return result;
  }

  /** Constructs an empty map. */
  constructor() {
    this._collection = new Collections.FastSet([], equals, hash);
  }

  /** Returns the number of elements in this set. */
  public get size(): number {
    return this._collection.length;
  }

  /** Tests if a value is a member of this set.
   * @param value - The value to test for set membership.
   * @return true iff the value is a member of this set.
   */
  public test(key: Key): boolean {
    return this._collection.has(key);
  }

  /** Tests if a value is a member of this set.
   * @param key - The value to test for set membership.
   * @return true iff the value is a member of this set.
   */
  public has(key: Key): boolean {
    return this._collection.has(key);
  }

  /** Adds a value to this set.
   * @param value - The value to add.
   */
  public add(value: Key): void {
    this._collection.add(value);
  }

  /** Removes a value from this set.
   * @param value - The value to remove.
   */
  public remove(value: Key): void {
    this._collection.delete(value);
  }

  /** Removes a value from this set.
   * @param value - The value to remove.
   * @return true if the value was in the set.
   */
  public delete(value: Key): boolean {
    if(this._collection.has(value)) {
      this._collection.delete(value);
      return true;
    }
    return false;
  }

  /** Calls a function for each element in the set.
   * @param callback - The function to call for each element.
   */
  public forEach(callback: (value: Key) => void): void {
    for(const entry of this) {
      callback(entry);
    }
  }

  /** Returns a shallow copy of this set. */
  public clone(): Set<Key> {
    const clone = new Set<Key>();
    for(const entry of this) {
      clone.add(entry);
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
    return this._collection.iterator() as Iterator<Key>;
  }

  private _collection: any;
}
