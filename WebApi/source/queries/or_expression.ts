import { ConstantExpression } from './constant_expression';
import { checkBoolean, Expression } from './expression';
import { QueryType } from './query_type';

/** An Expression that evaluates to the logical or of its operands. */
export class OrExpression extends Expression {

  /** Constructs an OrExpression from a JSON object. */
  public static fromJson(value: any): OrExpression {
    return new OrExpression(Expression.nestedFromJson(value.left),
      Expression.nestedFromJson(value.right));
  }

  /**
   * Constructs an OrExpression.
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
    return other instanceof OrExpression && this._left.equals(other._left) &&
      this._right.equals(other._right);
  }

  public toString(): string {
    return `(or ${this._left} ${this._right})`;
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
 * Returns an Expression representing the logical or over a sequence of
 * sub-expressions.
 * @param expressions - The sub-expressions to combine.
 */
export function disjunction(expressions: Expression[]): Expression {
  if(expressions.length === 0) {
    return ConstantExpression.FALSE;
  }
  let result = checkBoolean(expressions[expressions.length - 1]);
  for(let i = expressions.length - 2; i >= 0; --i) {
    result = new OrExpression(expressions[i], result);
  }
  return result;
}

const NAME = 'Beam.Queries.OrExpression';

Expression.register(NAME, OrExpression.fromJson);
