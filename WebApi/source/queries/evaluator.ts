import { EvaluatorNode } from './evaluator_node';
import { EvaluatorTranslator, MAX_EVALUATOR_PARAMETERS } from
  './evaluator_translator';
import { Expression } from './expression';
import { ParameterEvaluatorNode } from './parameter_evaluator_node';
import { QueryType } from './query_type';
import { matchesQueryType } from './query_value_check';
import { TypeCompatibilityError } from './type_compatibility_error';

/** Evaluates a translated expression. Recursive evaluation is not supported. */
export class Evaluator<T> {

  /**
   * Constructs an evaluator.
   * @param evaluator - The root evaluation node.
   * @param parameters - The parameter nodes used by the expression.
   */
  constructor(evaluator: EvaluatorNode<T>,
      parameters: ParameterEvaluatorNode<unknown>[]) {
    this._evaluator = evaluator;
    this._parameters = [];
    for(let i = 0; i < MAX_EVALUATOR_PARAMETERS; ++i) {
      this._parameters.push({type: null, value: undefined});
    }
    for(const parameter of parameters) {
      const entry = this._parameters[parameter.index];
      entry.type = parameter.type;
      parameter.setParameter(entry);
    }
  }

  /**
   * Evaluates the expression.
   * @param parameters - Up to two values bound to the expression's parameters.
   * @return The expression's result.
   * @throws TypeCompatibilityError If a parameter is absent or incompatible.
   */
  public eval(...parameters: unknown[]): T {
    if(parameters.length > MAX_EVALUATOR_PARAMETERS) {
      throw new TypeCompatibilityError('Too many parameters.');
    }
    for(let i = 0; i < this._parameters.length; ++i) {
      const entry = this._parameters[i];
      if(entry.type !== null && !matchesQueryType(entry.type, parameters[i])) {
        throw new TypeCompatibilityError('Parameter type mismatch.');
      }
    }
    for(let i = 0; i < this._parameters.length; ++i) {
      this._parameters[i].value = parameters[i];
    }
    return this._evaluator.eval();
  }

  private _evaluator: EvaluatorNode<T>;
  private _parameters: { type: QueryType, value: unknown }[];
}

/**
 * Translates an expression without parameters.
 * @param expression - The expression to translate.
 */
export function translate<T>(expression: Expression): Evaluator<T>;

/**
 * Translates an expression with parameters of a given type.
 * @param expression - The expression to translate.
 * @param type - The query type to which parameters are bound.
 */
export function translate<T>(
  expression: Expression, type: QueryType): Evaluator<T>;

/**
 * Translates an expression using a supplied translator.
 * @param expression - The expression to translate.
 * @param translator - The translator to use.
 */
export function translate<T>(
  expression: Expression, translator: EvaluatorTranslator): Evaluator<T>;
export function translate<T>(expression: Expression,
    typeOrTranslator?: QueryType | EvaluatorTranslator): Evaluator<T> {
  let translator: EvaluatorTranslator;
  if(typeOrTranslator instanceof EvaluatorTranslator) {
    translator = typeOrTranslator;
  } else if(typeOrTranslator === undefined) {
    translator = new EvaluatorTranslator();
  } else {
    translator = new EvaluatorTranslator(typeOrTranslator);
  }
  translator.translate(expression);
  return new Evaluator(translator.takeEvaluator() as EvaluatorNode<T>,
    translator.getParameters());
}
