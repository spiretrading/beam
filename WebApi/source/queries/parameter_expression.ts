import { Expression } from './expression';
import { QueryType } from './query_type';

/** An Expression that evaluates to one of a query's parameters. */
export class ParameterExpression extends Expression {

  /** Constructs a ParameterExpression from a JSON object. */
  public static fromJson(value: any): ParameterExpression {
    return new ParameterExpression(value.index, value.type);
  }

  /**
   * Constructs a ParameterExpression.
   * @param index - The index of the parameter.
   * @param type - The type the parameter evaluates to.
   */
  constructor(index: number, type: QueryType) {
    super();
    this._index = index;
    this._type = type;
  }

  /** Returns the index of the parameter. */
  public get index(): number {
    return this._index;
  }

  public get type(): QueryType {
    return this._type;
  }

  public equals(other: Expression): boolean {
    return other instanceof ParameterExpression &&
      this._index === other._index && this._type === other._type;
  }

  public toString(): string {
    return `(parameter ${this._index})`;
  }

  public toJson(): any {
    return {
      __type: NAME,
      index: this._index,
      type: this._type
    };
  }

  private _index: number;
  private _type: QueryType;
}

const NAME = 'Beam.Queries.ParameterExpression';

Expression.register(NAME, ParameterExpression.fromJson);
