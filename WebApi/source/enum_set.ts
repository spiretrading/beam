/** Implements a bitset over an enum type. */
export class EnumSet<T> {

  /** Parses an EnumSet from a JSON object. */
  public static fromJson<T>(value: any) {
    return new EnumSet<T>(value as number);
  }

  /** Constructs an empty set.
   * @param value - The integer representation of this set.
   */
  constructor(value: number = 0) {
    this._value = value;
  }

  /** Tests if a value is a member of this set.
   * @param value - The value to test.
   * @return Whether value belongs to this set.
   */
  public test(value: T): boolean {
    return (this._value & Math.pow(2, value as any)) != 0;
  }

  /** Adds a value to this set.
   * @param value - The value to add.
   */
  public set(value: T): void {
    this._value |= Math.pow(2, value as any);
  }

  /** Removes a value from this set.
   * @param value - The value to remove.
   */
  public unset(value: T): void {
    this._value &= ~Math.pow(2, value as any);
  }

  /** Returns a clone of this set. */
  public clone(): EnumSet<T> {
    return new EnumSet<T>(this._value);
  }

  /** Convers this set to a JSON object. */
  public toJson() {
    return this._value;
  }

  private _value: number;
}
