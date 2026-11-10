import { QueryType } from './query_type';

/** Evaluates an expression to a value of type T. */
export abstract class EvaluatorNode<T> {

  /** Returns the type evaluated by this node. */
  public abstract get type(): QueryType;

  /** Evaluates this node using its bound parameters. */
  public abstract eval(): T;
}
