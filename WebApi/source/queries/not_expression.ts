import { checkBoolean, Expression } from './expression';
import { QueryType } from './query_type';

/** An Expression that evaluates to the negation of its operand. */
export class NotExpression extends Expression {

  /** Constructs a NotExpression from a JSON object. */
  public static fromJson(value: any): NotExpression {
    return new NotExpression(Expression.nestedFromJson(value.operand));
  }

  /**
   * Constructs a NotExpression.
   * @param operand - The Expression to negate.
   */
  constructor(operand: Expression) {
    super();
    this._operand = checkBoolean(operand);
  }

  /** Returns the operand. */
  public get operand(): Expression {
    return this._operand;
  }

  public get type(): QueryType {
    return QueryType.BOOL;
  }

  public equals(other: Expression): boolean {
    return other instanceof NotExpression &&
      this._operand.equals(other._operand);
  }

  public toString(): string {
    return `(not ${this._operand})`;
  }

  public toJson(): any {
    return {
      __type: NAME,
      operand: Expression.nestedToJson(this._operand)
    };
  }

  private _operand: Expression;
}

const NAME = 'Beam.Queries.NotExpression';

Expression.register(NAME, NotExpression.fromJson);
