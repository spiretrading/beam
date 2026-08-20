import { Expression } from './expression';
import { QueryType } from './query_type';

/** An Expression that evaluates to a member of another Expression. */
export class MemberAccessExpression extends Expression {

  /** Constructs a MemberAccessExpression from a JSON object. */
  public static fromJson(value: any): MemberAccessExpression {
    return new MemberAccessExpression(
      value.name, value.type, Expression.nestedFromJson(value.expression));
  }

  /**
   * Constructs a MemberAccessExpression.
   * @param name - The name of the member to access.
   * @param type - The type the member evaluates to.
   * @param expression - The Expression whose member is accessed.
   */
  constructor(name: string, type: QueryType, expression: Expression) {
    super();
    this._name = name;
    this._type = type;
    this._expression = expression;
  }

  /** Returns the name of the member to access. */
  public get name(): string {
    return this._name;
  }

  /** Returns the Expression whose member is accessed. */
  public get expression(): Expression {
    return this._expression;
  }

  public get type(): QueryType {
    return this._type;
  }

  public equals(other: Expression): boolean {
    return other instanceof MemberAccessExpression &&
      this._name === other._name && this._type === other._type &&
      this._expression.equals(other._expression);
  }

  public toString(): string {
    return `${this._expression}.${this._name}`;
  }

  public toJson(): any {
    return {
      __type: NAME,
      name: this._name,
      type: this._type,
      expression: Expression.nestedToJson(this._expression)
    };
  }

  private _name: string;
  private _type: QueryType;
  private _expression: Expression;
}

const NAME = 'Beam.Queries.MemberAccessExpression';

Expression.register(NAME, MemberAccessExpression.fromJson);
