import { Expression } from './expression';
import { QueryType } from './query_type';

/** An Expression that evaluates to the result of applying a function. */
export class FunctionExpression extends Expression {

  /** Constructs a FunctionExpression from a JSON object. */
  public static fromJson(value: any): FunctionExpression {
    return new FunctionExpression(
      value.name, value.type, value.parameters.map(
        (parameter: any) => Expression.nestedFromJson(parameter)));
  }

  /**
   * Constructs a FunctionExpression.
   * @param name - The name of the function to apply.
   * @param type - The type the function evaluates to.
   * @param parameters - The parameters to apply the function to.
   */
  constructor(name: string, type: QueryType, parameters: Expression[]) {
    super();
    this._name = name;
    this._type = type;
    this._parameters = parameters.slice();
  }

  /** Returns the name of the function to apply. */
  public get name(): string {
    return this._name;
  }

  /** Returns the parameters to apply the function to. */
  public get parameters(): Expression[] {
    return this._parameters.slice();
  }

  public get type(): QueryType {
    return this._type;
  }

  public equals(other: Expression): boolean {
    if(!(other instanceof FunctionExpression) || this._name !== other._name ||
        this._type !== other._type ||
        this._parameters.length !== other._parameters.length) {
      return false;
    }
    return this._parameters.every(
      (parameter, index) => parameter.equals(other._parameters[index]));
  }

  public toString(): string {
    const parameters = this._parameters.map(
      (parameter) => ` ${parameter}`).join('');
    return `(${this._name}${parameters})`;
  }

  public toJson(): any {
    return {
      __type: NAME,
      name: this._name,
      type: this._type,
      parameters: this._parameters.map(
        (parameter) => Expression.nestedToJson(parameter))
    };
  }

  private _name: string;
  private _type: QueryType;
  private _parameters: Expression[];
}

const NAME = 'Beam.Queries.FunctionExpression';

Expression.register(NAME, FunctionExpression.fromJson);
