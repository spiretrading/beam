import { InterruptionPolicy } from './interruption_policy';

/** A query with a policy for recovering from an interruption or failure. */
export class InterruptableQuery {

  /** Constructs an InterruptableQuery from a JSON object. */
  public static fromJson(value: any): InterruptableQuery {
    return new InterruptableQuery(value.interruption_policy);
  }

  /**
   * Constructs an InterruptableQuery.
   * @param policy - The policy used to recover from an interruption.
   */
  constructor(policy: InterruptionPolicy = InterruptionPolicy.BREAK_QUERY) {
    this._interruptionPolicy = policy;
  }

  /** Returns the InterruptionPolicy. */
  public get interruptionPolicy(): InterruptionPolicy {
    return this._interruptionPolicy;
  }

  public set interruptionPolicy(value: InterruptionPolicy) {
    this._interruptionPolicy = value;
  }

  /** Tests if two queries recover from an interruption the same way. */
  public equals(other: InterruptableQuery): boolean {
    return other && this._interruptionPolicy === other._interruptionPolicy;
  }

  public toString(): string {
    return InterruptionPolicy[this._interruptionPolicy];
  }

  /** Converts this object to JSON. */
  public toJson(): any {
    return {
      interruption_policy: this._interruptionPolicy
    };
  }

  private _interruptionPolicy: InterruptionPolicy;
}
