/** Used to order sequential data. */
export class Sequence {

  /** Represents the first possible Sequence. */
  public static readonly FIRST = new Sequence(0n);

  /** Represents the last possible Sequence. */
  public static readonly LAST = new Sequence(18446744073709551615n);

  /** Represents the 'present' in a query. */
  public static readonly PRESENT = Sequence.LAST.decrement();

  /** Constructs a Sequence from a JSON object. */
  public static fromJson(value: any): Sequence {
    return new Sequence(BigInt(value));
  }

  /**
   * Constructs a Sequence.
   * @param ordinal - The Sequence's ordinal.
   */
  constructor(ordinal: bigint = 0n) {
    this._ordinal = ordinal;
  }

  /** Returns the ordinal. */
  public get ordinal(): bigint {
    return this._ordinal;
  }

  /** Returns the immediately following Sequence. */
  public increment(): Sequence {
    if(this._ordinal === Sequence.LAST._ordinal) {
      return this;
    }
    return new Sequence(this._ordinal + 1n);
  }

  /** Returns the immediately preceding Sequence. */
  public decrement(): Sequence {
    if(this._ordinal === 0n) {
      return this;
    }
    return new Sequence(this._ordinal - 1n);
  }

  /** Compares this Sequence with another.
   * @return A negative value if this < other, 0 if equal, positive if
   *         this > other.
   */
  public compare(other: Sequence): number {
    if(this._ordinal < other._ordinal) {
      return -1;
    } else if(this._ordinal > other._ordinal) {
      return 1;
    }
    return 0;
  }

  /** Tests if two sequences share the same ordinal. */
  public equals(other: Sequence): boolean {
    return other && this._ordinal === other._ordinal;
  }

  public toString(): string {
    return this._ordinal.toString();
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return this._ordinal.toString();
  }

  private _ordinal: bigint;
}
