import { EvaluatorNode } from './evaluator_node';
import { QueryType } from './query_type';

/** Evaluates logical and with short-circuit evaluation. */
export class AndEvaluatorNode extends EvaluatorNode<boolean> {

  /**
   * Constructs a logical and evaluator.
   * @param left - The left operand.
   * @param right - The right operand.
   */
  constructor(left: EvaluatorNode<boolean>, right: EvaluatorNode<boolean>) {
    super();
    this._left = left;
    this._right = right;
  }

  public get type(): QueryType {
    return QueryType.BOOL;
  }

  public eval(): boolean {
    return this._left.eval() && this._right.eval();
  }

  private _left: EvaluatorNode<boolean>;
  private _right: EvaluatorNode<boolean>;
}
