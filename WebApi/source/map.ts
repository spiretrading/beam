declare function require(name:string): any;
const Collections = require('collections/map');

function hash(value: any) {
  return value.hash();
}

function equals(left: any, right: any) {
  const f = left['equals'];
  if(f) {
    return f(right);
  }
  return left === right;
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
      result.set(keyType.fromJson(value[0]), valueType.fromJson(value[1]));
    }
    return result;
  }

  /** Constructs an empty map. */
  constructor() {
    this._collection = new Collections.Map([], hash, equals);
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
    for(let entry of this._collection.entries()) {
      value.push([entry[0].toJson(), entry[1].toJson()]);
    }
    return value;
  }

  private _collection: any;
}
