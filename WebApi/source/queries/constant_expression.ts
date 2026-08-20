import { Expression } from './expression';
import { QueryType } from './query_type';
import { BoolValue } from './standard_values';
import { Value } from './value';

/** An Expression that evaluates to a constant. */
export class ConstantExpression extends Expression {

  /** An Expression that evaluates to true. */
  public static readonly TRUE = new ConstantExpression(new BoolValue(true));

  /** An Expression that evaluates to false. */
  public static readonly FALSE = new ConstantExpression(new BoolValue(false));

  /** Constructs a ConstantExpression from a JSON object. */
  public static fromJson(value: any): ConstantExpression {
    return new ConstantExpression(Value.fromJson(value.value.value));
  }

  /**
   * Constructs a ConstantExpression.
   * @param value - The value to evaluate to.
   */
  constructor(value: Value) {
    super();
    this._value = value;
  }

  /** Returns the value to evaluate to. */
  public get value(): Value {
    return this._value;
  }

  public get type(): QueryType {
    return this._value.type;
  }

  public equals(other: Expression): boolean {
    return other instanceof ConstantExpression &&
      this._value.equals(other._value);
  }

  public toString(): string {
    return this._value.toString();
  }

  public toJson(): any {
    return {
      __type: NAME,
      value: {
        value: this._value.toJson()
      }
    };
  }

  private _value: Value;
}

const NAME = 'Beam.Queries.ConstantExpression';

Expression.register(NAME, ConstantExpression.fromJson);
