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

  /** Constructs an empty set. */
  constructor() {
    this._buckets = new globalThis.Map<string, Key[]>();
    this._size = 0;
  }

  /** Returns the number of elements in this set. */
  public get size(): number {
    return this._size;
  }

  /** Tests if a value is a member of this set.
   * @param value - The value to test for set membership.
   * @return true iff the value is a member of this set.
   */
  public test(key: Key): boolean {
    return this.has(key);
  }

  /** Tests if a value is a member of this set.
   * @param key - The value to test for set membership.
   * @return true iff the value is a member of this set.
   */
  public has(key: Key): boolean {
    const bucket = this._buckets.get(hash(key));
    if(bucket) {
      for(const k of bucket) {
        if(equals(k, key)) {
          return true;
        }
      }
    }
    return false;
  }

  /** Adds a value to this set.
   * @param value - The value to add.
   */
  public add(value: Key): void {
    const h = hash(value);
    const bucket = this._buckets.get(h);
    if(bucket) {
      for(const k of bucket) {
        if(equals(k, value)) {
          return;
        }
      }
      bucket.push(value);
    } else {
      this._buckets.set(h, [value]);
    }
    ++this._size;
  }

  /** Removes a value from this set.
   * @param value - The value to remove.
   */
  public remove(value: Key): void {
    this.delete(value);
  }

  /** Removes a value from this set.
   * @param value - The value to remove.
   * @return true if the value was in the set.
   */
  public delete(value: Key): boolean {
    const h = hash(value);
    const bucket = this._buckets.get(h);
    if(bucket) {
      for(let i = 0; i < bucket.length; ++i) {
        if(equals(bucket[i], value)) {
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

  *[Symbol.iterator](): Iterator<Key> {
    for(const bucket of this._buckets.values()) {
      for(const entry of bucket) {
        yield entry;
      }
    }
  }

  private _buckets: globalThis.Map<string, Key[]>;
  private _size: number;
}
