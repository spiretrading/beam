import { EvaluatorNode } from './evaluator_node';
import { QueryType } from './query_type';

/** Evaluates the logical negation of an operand. */
export class NotEvaluatorNode extends EvaluatorNode<boolean> {

  /**
   * Constructs a logical negation evaluator.
   * @param operand - The operand to negate.
   */
  constructor(operand: EvaluatorNode<boolean>) {
    super();
    this._operand = operand;
  }

  public get type(): QueryType {
    return QueryType.BOOL;
  }

  public eval(): boolean {
    return !this._operand.eval();
  }

  private _operand: EvaluatorNode<boolean>;
}
