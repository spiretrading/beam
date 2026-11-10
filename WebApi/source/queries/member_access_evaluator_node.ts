import { EvaluatorNode } from './evaluator_node';
import { QueryType } from './query_type';

/** Evaluates a member of an object using an explicit accessor. */
export class MemberAccessEvaluatorNode<T, M> extends EvaluatorNode<M> {

  /**
   * Constructs a member evaluator.
   * @param type - The member's query type.
   * @param evaluator - The node producing the object.
   * @param accessor - The function accessing the object's member.
   */
  constructor(type: QueryType, evaluator: EvaluatorNode<T>,
      accessor: (value: T) => M) {
    super();
    this._type = type;
    this._evaluator = evaluator;
    this._accessor = accessor;
  }

  public get type(): QueryType {
    return this._type;
  }

  public eval(): M {
    return this._accessor(this._evaluator.eval());
  }

  private _type: QueryType;
  private _evaluator: EvaluatorNode<T>;
  private _accessor: (value: T) => M;
}
