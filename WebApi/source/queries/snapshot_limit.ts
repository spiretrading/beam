/** Specifies a limit on a query snapshot. */
export class SnapshotLimit {

  /** A SnapshotLimit that returns no results. */
  public static readonly NONE: SnapshotLimit;

  /** A SnapshotLimit that returns all results. */
  public static readonly UNLIMITED: SnapshotLimit;

  /** Constructs a SnapshotLimit from a JSON object. */
  public static fromJson(value: any): SnapshotLimit {
    return new SnapshotLimit(value.type, value.size);
  }

  /**
   * Returns a SnapshotLimit that loads from the head (oldest first).
   * @param size - The maximum number of items to load.
   */
  public static fromHead(size: number): SnapshotLimit {
    return new SnapshotLimit(SnapshotLimit.Type.HEAD, size);
  }

  /**
   * Returns a SnapshotLimit that loads from the tail (most recent first).
   * @param size - The maximum number of items to load.
   */
  public static fromTail(size: number): SnapshotLimit {
    return new SnapshotLimit(SnapshotLimit.Type.TAIL, size);
  }

  /**
   * Constructs a SnapshotLimit.
   * @param type - The direction to load from.
   * @param size - The maximum number of items to load.
   */
  constructor(
      type: SnapshotLimit.Type = SnapshotLimit.Type.HEAD, size: number = 0) {
    this._type = type;
    this._size = size;
  }

  /** Returns the direction to load from. */
  public get type(): SnapshotLimit.Type {
    return this._type;
  }

  /** Returns the maximum number of items to load. */
  public get size(): number {
    return this._size;
  }

  /** Tests if two limits load the same number of items. */
  public equals(other: SnapshotLimit): boolean {
    if(!other) {
      return false;
    } else if(this._size === 0 || this._size === UNLIMITED_SIZE) {
      return this._size === other._size;
    }
    return this._type === other._type && this._size === other._size;
  }

  public toString(): string {
    if(this.equals(SnapshotLimit.NONE)) {
      return 'None';
    } else if(this.equals(SnapshotLimit.UNLIMITED)) {
      return 'Unlimited';
    }
    return `(${typeToString(this._type)} ${this._size})`;
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      type: this._type,
      size: this._size
    };
  }

  private _type: SnapshotLimit.Type;
  private _size: number;
}

export namespace SnapshotLimit {

  /** The direction to load from. */
  export enum Type {
    HEAD = 0,
    TAIL
  }
}

const UNLIMITED_SIZE = 2147483647;

(SnapshotLimit as any).NONE = new SnapshotLimit(SnapshotLimit.Type.HEAD, 0);
(SnapshotLimit as any).UNLIMITED =
  new SnapshotLimit(SnapshotLimit.Type.HEAD, UNLIMITED_SIZE);

function typeToString(type: SnapshotLimit.Type): string {
  if(type === SnapshotLimit.Type.HEAD) {
    return 'HEAD';
  } else if(type === SnapshotLimit.Type.TAIL) {
    return 'TAIL';
  }
  return 'NONE';
}
