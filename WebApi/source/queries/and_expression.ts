import { ConstantExpression } from './constant_expression';
import { checkBoolean, Expression } from './expression';
import { QueryType } from './query_type';

/** An Expression that evaluates to the logical and of its operands. */
export class AndExpression extends Expression {

  /** Constructs an AndExpression from a JSON object. */
  public static fromJson(value: any): AndExpression {
    return new AndExpression(Expression.nestedFromJson(value.left),
      Expression.nestedFromJson(value.right));
  }

  /**
   * Constructs an AndExpression.
   * @param left - The left hand side of the expression.
   * @param right - The right hand side of the expression.
   */
  constructor(left: Expression, right: Expression) {
    super();
    this._left = checkBoolean(left);
    this._right = checkBoolean(right);
  }

  /** Returns the left hand side of the expression. */
  public get left(): Expression {
    return this._left;
  }

  /** Returns the right hand side of the expression. */
  public get right(): Expression {
    return this._right;
  }

  public get type(): QueryType {
    return QueryType.BOOL;
  }

  public equals(other: Expression): boolean {
    return other instanceof AndExpression && this._left.equals(other._left) &&
      this._right.equals(other._right);
  }

  public toString(): string {
    return `(and ${this._left} ${this._right})`;
  }

  public toJson(): any {
    return {
      __type: NAME,
      left: Expression.nestedToJson(this._left),
      right: Expression.nestedToJson(this._right)
    };
  }

  private _left: Expression;
  private _right: Expression;
}

/**
 * Returns an Expression representing the logical and over a sequence of
 * sub-expressions.
 * @param expressions - The sub-expressions to combine.
 */
export function conjunction(expressions: Expression[]): Expression {
  if(expressions.length === 0) {
    return ConstantExpression.FALSE;
  }
  let result = checkBoolean(expressions[expressions.length - 1]);
  for(let i = expressions.length - 2; i >= 0; --i) {
    result = new AndExpression(expressions[i], result);
  }
  return result;
}

const NAME = 'Beam.Queries.AndExpression';

Expression.register(NAME, AndExpression.fromJson);
