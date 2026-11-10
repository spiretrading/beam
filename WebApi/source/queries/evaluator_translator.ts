import { AndEvaluatorNode } from './and_evaluator_node';
import { AndExpression } from './and_expression';
import { ConstantEvaluatorNode } from './constant_evaluator_node';
import { ConstantExpression } from './constant_expression';
import { EvaluatorNode } from './evaluator_node';
import { Expression } from './expression';
import { ExpressionTranslationError } from './expression_translation_error';
import { FunctionExpression } from './function_expression';
import { NotEvaluatorNode } from './not_evaluator_node';
import { NotExpression } from './not_expression';
import { OrEvaluatorNode } from './or_evaluator_node';
import { OrExpression } from './or_expression';
import { ParameterEvaluatorNode } from './parameter_evaluator_node';
import { ParameterExpression } from './parameter_expression';
import { QueryType } from './query_type';
import { matchesQueryType } from './query_value_check';
import { translateStandardFunction } from './standard_function_evaluator';
import { NativeValue } from './value';

/** The maximum number of parameters supported by an evaluator. */
export const MAX_EVALUATOR_PARAMETERS = 2;

/** Translates Beam expressions into evaluator nodes. */
export class EvaluatorTranslator {

  /** Constructs a translator for expressions without parameters. */
  constructor();

  /**
   * Constructs a translator for expressions with parameters.
   * @param type - The query type to which parameters are bound.
   */
  constructor(type: QueryType);
  constructor(type?: QueryType) {
    this._type = type ?? null;
    this._evaluator = null;
    this._parameters = [];
  }

  /**
   * Translates an expression, replacing the previous translation.
   * @param expression - The expression to translate.
   * @throws ExpressionTranslationError If the expression cannot be translated.
   */
  public translate(expression: Expression): void {
    this._evaluator = null;
    this._parameters = [];
    try {
      const evaluator = this.translateOperand(expression);
      const types = new Map<number, QueryType>();
      for(const parameter of this._parameters) {
        if(types.has(parameter.index) &&
            types.get(parameter.index) !== parameter.type) {
          throw new ExpressionTranslationError('Parameter type mismatch.');
        }
        types.set(parameter.index, parameter.type);
      }
      for(let i = 0; i < types.size; ++i) {
        if(!types.has(i)) {
          throw new ExpressionTranslationError('Missing parameter.');
        }
      }
      this._evaluator = evaluator;
    } catch(error) {
      this._parameters = [];
      throw error;
    }
  }

  /** Takes the most recently translated root node, or null if none exists. */
  public takeEvaluator(): EvaluatorNode<unknown> {
    const evaluator = this._evaluator;
    this._evaluator = null;
    return evaluator;
  }

  /** Returns the parameter nodes used by the current translation. */
  public getParameters(): ParameterEvaluatorNode<unknown>[] {
    return this._parameters.slice();
  }

  /**
   * Creates a translator for a sub-expression.
   * @param type - The query type to which parameters are bound.
   */
  public makeTranslator(type: QueryType): EvaluatorTranslator {
    return new EvaluatorTranslator(type);
  }

  /**
   * Translates an operand and checks its declared result type.
   * @param expression - The operand to translate.
   */
  protected translateOperand(expression: Expression): EvaluatorNode<unknown> {
    const result = this.translateExpression(expression);
    if(!result || result.type !== expression.type) {
      throw new ExpressionTranslationError('Expression type mismatch.');
    }
    return result;
  }

  /**
   * Translates a Beam expression. Override to support domain expressions.
   * @param expression - The expression to translate.
   */
  protected translateExpression(expression: Expression):
      EvaluatorNode<unknown> {
    if(expression instanceof ConstantExpression) {
      const value = expression.value;
      if(!(value instanceof NativeValue) ||
          !matchesQueryType(value.type, value.value)) {
        throw new ExpressionTranslationError('Constant type not supported.');
      }
      return new ConstantEvaluatorNode(value.type, value.value);
    }
    if(expression instanceof ParameterExpression) {
      if(!Number.isInteger(expression.index) || expression.index < 0 ||
          expression.index >= MAX_EVALUATOR_PARAMETERS) {
        throw new ExpressionTranslationError('Invalid parameter index.');
      }
      if(expression.type !== this._type ||
          !Object.values(QueryType).includes(expression.type)) {
        throw new ExpressionTranslationError('Parameter type mismatch.');
      }
      const parameter = new ParameterEvaluatorNode(
        expression.index, expression.type);
      this._parameters.push(parameter);
      return parameter;
    }
    if(expression instanceof AndExpression) {
      return new AndEvaluatorNode(
        this.translateOperand(expression.left) as EvaluatorNode<boolean>,
        this.translateOperand(expression.right) as EvaluatorNode<boolean>);
    }
    if(expression instanceof OrExpression) {
      return new OrEvaluatorNode(
        this.translateOperand(expression.left) as EvaluatorNode<boolean>,
        this.translateOperand(expression.right) as EvaluatorNode<boolean>);
    }
    if(expression instanceof NotExpression) {
      return new NotEvaluatorNode(
        this.translateOperand(expression.operand) as EvaluatorNode<boolean>);
    }
    if(expression instanceof FunctionExpression) {
      return translateStandardFunction(expression.name,
        expression.parameters.map(operand => this.translateOperand(operand)));
    }
    throw new ExpressionTranslationError('Expression not supported.');
  }

  private _type: QueryType;
  private _evaluator: EvaluatorNode<unknown>;
  private _parameters: ParameterEvaluatorNode<unknown>[];
}
