import { equals } from './comparators';
import { fromJson, toJson } from './serialization';

/** Stores a key and its associated value. */
export class KeyValuePair<K, V> {

  /**
   * Constructs a KeyValuePair from JSON.
   * @param keyType - The type of key to parse.
   * @param valueType - The type of value to parse.
   * @param value - The JSON object to parse.
   */
  public static fromJson<K, V>(keyType: any, valueType: any, value: any):
      KeyValuePair<K, V> {
    return new KeyValuePair(fromJson(keyType, value.key),
      fromJson(valueType, value.value));
  }

  /** The pair's key. */
  public key: K;

  /** The value associated with the key. */
  public value: V;

  /**
   * Constructs a KeyValuePair.
   * @param key - The pair's key.
   * @param value - The value associated with the key.
   */
  constructor(key: K, value: V) {
    this.key = key;
    this.value = value;
  }

  /** Tests if two pairs have equal keys and values. */
  public equals(other: KeyValuePair<K, V>): boolean {
    return other && equals(this.key, other.key) &&
      equals(this.value, other.value);
  }

  public toString(): string {
    return `(${this.key} ${this.value})`;
  }

  /** Converts this pair to JSON. */
  public toJson(): any {
    return {key: toJson(this.key), value: toJson(this.value)};
  }
}
