import { EvaluatorNode } from './evaluator_node';
import { QueryType } from './query_type';

/** Evaluates to a constant value. */
export class ConstantEvaluatorNode<T> extends EvaluatorNode<T> {

  /**
   * Constructs a constant evaluator.
   * @param type - The query type of the value.
   * @param value - The value to return.
   */
  constructor(type: QueryType, value: T) {
    super();
    this._type = type;
    this._value = value;
  }

  public get type(): QueryType {
    return this._type;
  }

  public eval(): T {
    return this._value;
  }

  private _type: QueryType;
  private _value: T;
}
