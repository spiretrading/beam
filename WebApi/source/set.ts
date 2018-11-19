declare function require(name:string): any;
const Collections = require('collections/fast-set');
import { equals, fromJson, hash, toJson } from '.';

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
      result.set(fromJson(keyType, key));
    }
    return result;
  }

  /** Constructs an empty map. */
  constructor() {
    this._collection = new Collections.FastSet([], equals, hash);
  }

  /** Tests if a value is a member of this set.
   * @param value - The value to test for set membership.
   * @return true iff the value is a member of this set.
   */
  public test(key: Key): boolean {
    return this._collection.has(key);
  }

  /** Adds a value to this set.
   * @param value - The value to add.
   */
  public set(value: Key): void {
    this._collection.set(value);
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
    return this._collection.entries() as Iterator<[Key]>;
  }

  private _collection: any;
}
