/** Represents a directory entry in Beam's account database. */
export class DirectoryEntry {

  /** Constructs a DirectoryEntry.
   * @param type - The type of directory entry to represent.
   * @param id - The entry's unique id.
   * @param name - The name of the of the directory entry.
   */
  public constructor(type: DirectoryEntry.Type, id: number, name: string) {
    this._type = type;
    this._id = id;
    this._name = name;
  }

  /** Returns the type of directory entry. */
  public get type(): DirectoryEntry.Type {
    return this._type;
  }

  /** Returns the entry's unique id. */
  public get id(): number {
    return this._id;
  }

  /** Returns the name of the directory entry. */
  public get name(): string {
    return this._name;
  }

  private _type: DirectoryEntry.Type;
  private _id: number;
  private _name: string;
}

export module DirectoryEntry {

  /** Enumerates the types of directory entries. */
  export enum Type {

    /** An invalid directory entry. */
    NONE = -1,

    /** An account. */
    ACCOUNT = 0,

    /** A directory. */
    DIRECTORY = 1
  }
}
