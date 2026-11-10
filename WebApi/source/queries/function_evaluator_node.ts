import { EvaluatorNode } from './evaluator_node';
import { QueryType } from './query_type';

/** Evaluates a function applied to operand nodes. */
export class FunctionEvaluatorNode<T> extends EvaluatorNode<T> {

  /**
   * Constructs a function evaluator.
   * @param type - The function's result type.
   * @param operation - The function to evaluate.
   * @param operands - The nodes supplying the function's arguments.
   */
  constructor(type: QueryType, operation: (...values: any[]) => T,
      operands: EvaluatorNode<unknown>[]) {
    super();
    this._type = type;
    this._operation = operation;
    this._operands = operands.slice();
  }

  public get type(): QueryType {
    return this._type;
  }

  public eval(): T {
    return this._operation(...this._operands.map(operand => operand.eval()));
  }

  private _type: QueryType;
  private _operation: (...values: any[]) => T;
  private _operands: EvaluatorNode<unknown>[];
}
