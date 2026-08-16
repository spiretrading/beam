import { SnapshotLimit } from './snapshot_limit';

/** Queries for a snapshot with a specified limit. */
export class SnapshotLimitedQuery {

  /** Constructs a SnapshotLimitedQuery from a JSON object. */
  public static fromJson(value: any): SnapshotLimitedQuery {
    return new SnapshotLimitedQuery(
      SnapshotLimit.fromJson(value.snapshot_limit));
  }

  /**
   * Constructs a SnapshotLimitedQuery.
   * @param snapshotLimit - The limit on the snapshot to load.
   */
  constructor(snapshotLimit: SnapshotLimit = SnapshotLimit.NONE) {
    this._snapshotLimit = snapshotLimit;
  }

  /** Returns the SnapshotLimit. */
  public get snapshotLimit(): SnapshotLimit {
    return this._snapshotLimit;
  }

  public set snapshotLimit(value: SnapshotLimit) {
    this._snapshotLimit = value;
  }

  /** Tests if two queries load the same size of snapshot. */
  public equals(other: SnapshotLimitedQuery): boolean {
    return other && this._snapshotLimit.equals(other._snapshotLimit);
  }

  public toString(): string {
    return this._snapshotLimit.toString();
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      snapshot_limit: this._snapshotLimit.toJson()
    };
  }

  private _snapshotLimit: SnapshotLimit;
}
