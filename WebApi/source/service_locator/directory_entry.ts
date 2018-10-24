/** Represents a directory entry in Beam's account database. */
export class DirectoryEntry {

  /** Represents an invalid directory entry. */
  public static readonly INVALID = new DirectoryEntry(-1, -1, '');

  /** Constructs a DirectoryEntry from a JSON object. */
  public static fromJson(value: any): DirectoryEntry {
    if(value.type == DirectoryEntry.Type.NONE) {
      return DirectoryEntry.INVALID;
    }
    return new DirectoryEntry(value.type, value.id, value.name);
  }

  /** Constructs a DirectoryEntry representing an account.
   * @param id - The account's unique id.
   * @param name - The name of the account.
   */
  public static makeAccount(id: number, name: string): DirectoryEntry {
    return new DirectoryEntry(DirectoryEntry.Type.ACCOUNT, id, name);
  }

  /** Constructs a DirectoryEntry representing a directory.
   * @param id - The directory's unique id.
   * @param name - The name of the directory.
   */
  public static makeDirectory(id: number, name: string): DirectoryEntry {
    return new DirectoryEntry(DirectoryEntry.Type.DIRECTORY, id, name);
  }

  /** Constructs a DirectoryEntry.
   * @param type - The type of directory entry to represent.
   * @param id - The entry's unique id.
   * @param name - The name of the directory entry.
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

  /** Returns true if two directory entries refer to the same id. */
  public equals(other: DirectoryEntry): boolean {
    return other && this._id === other._id;
  }

  /** Converts this object to JSON. */
  public toJson() {
    return {
      type: this._type,
      id: this._id,
      name: this._name
    };
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
