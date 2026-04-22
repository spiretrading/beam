/** Indicates that a Queue's pipe has been broken. */
export class PipeBrokenError extends Error {
  constructor() {
    super('Pipe broken.');
  }
}
