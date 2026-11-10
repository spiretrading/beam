import { EvaluatorNode } from './evaluator_node';
import { QueryType } from './query_type';

/** Evaluates to a supplied parameter. */
export class ParameterEvaluatorNode<T> extends EvaluatorNode<T> {

  /**
   * Constructs a parameter evaluator.
   * @param index - The zero-based parameter index.
   * @param type - The query type of the parameter.
   */
  constructor(index: number, type: QueryType) {
    super();
    this._index = index;
    this._type = type;
    this._parameter = null;
  }

  /** Returns the parameter index. */
  public get index(): number {
    return this._index;
  }

  public get type(): QueryType {
    return this._type;
  }

  /**
   * Binds the parameter to use when evaluating this node.
   * @param parameter - The value holder, which must be bound before evaluation.
   */
  public setParameter(parameter: { value: T }): void {
    this._parameter = parameter;
  }

  public eval(): T {
    return this._parameter.value;
  }

  private _index: number;
  private _type: QueryType;
  private _parameter: { value: T };
}
