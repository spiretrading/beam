import { Date } from '../definitions/date';
import { DateTime } from '../definitions/date_time';
import { Duration } from '../definitions/duration';
import { EvaluatorNode } from './evaluator_node';
import { ExpressionTranslationError } from './expression_translation_error';
import { FunctionEvaluatorNode } from './function_evaluator_node';
import { QueryType } from './query_type';
import { getStandardFunctionType } from './standard_function_types';

/**
 * Translates a standard function applied to two operand nodes.
 * @param name - The standard function's name.
 * @param operands - The nodes supplying the function's arguments.
 */
export function translateStandardFunction(name: string,
    operands: EvaluatorNode<unknown>[]): EvaluatorNode<unknown> {
  if(operands.length !== 2) {
    throw new ExpressionTranslationError('Invalid parameter count.');
  }
  const [left, right] = operands;
  const type = getStandardFunctionType(name, left.type, right.type);
  if(!type) {
    throw new ExpressionTranslationError(
      'Function or operand types unsupported.');
  }
  return new FunctionEvaluatorNode(type,
    (a, b) => evaluate(name, type, left.type, right.type, a, b), operands);
}

function evaluate(name: string, type: QueryType, left: QueryType,
    right: QueryType, a: any, b: any): unknown {
  if(left === QueryType.INT && right === QueryType.ID) {
    a = BigInt(a);
  } else if(left === QueryType.ID && right === QueryType.INT) {
    b = BigInt(b);
  }
  if(['==', '!=', '<', '<=', '>', '>=', 'min', 'max'].includes(name)) {
    let x = a;
    let y = b;
    if(a instanceof DateTime || a instanceof Duration) {
      x = a.compare(b);
      if(a.equals(b)) {
        x = 0;
      }
      y = 0;
    }
    switch(name) {
      case '==': return x === y;
      case '!=': return x !== y;
      case '<': return x < y;
      case '<=': return x <= y;
      case '>': return x > y;
      case '>=': return x >= y;
      case 'min':
        if(y < x) {
          return b;
        }
        return a;
      case 'max':
        if(x < y) {
          return b;
        }
        return a;
    }
  }
  if(a instanceof DateTime) {
    if(b instanceof DateTime) {
      return new Duration(dateTicks(a.date) - dateTicks(b.date) +
        a.timeOfDay.ticks - b.timeOfDay.ticks);
    }
    let ticks = b.ticks;
    if(name === '-') {
      ticks = -ticks;
    }
    return shift(a, ticks);
  }
  if(a instanceof Duration) {
    if(name === '+') {
      return a.add(b);
    }
    return a.subtract(b);
  }
  if(left === QueryType.CHAR) {
    a = a.charCodeAt(0);
    b = b.charCodeAt(0);
  } else if(left === QueryType.BOOL) {
    a = Number(a);
    b = Number(b);
  }
  let result: any;
  switch(name) {
    case '+': result = a + b; break;
    case '-': result = a - b; break;
    case '*': result = a * b; break;
    case '/':
      if(type !== QueryType.DECIMAL && (b === 0 || b === 0n)) {
        throw new RangeError('Division by zero.');
      }
      result = a / b;
      if(type === QueryType.INT) {
        result = Math.trunc(result);
      }
      break;
  }
  if(type === QueryType.ID) {
    return BigInt.asUintN(64, result);
  }
  return result;
}

function dateTicks(date: Date): number {
  if(date.equals(Date.NOT_A_DATE)) {
    return NaN;
  }
  if(date.equals(Date.POS_INFIN)) {
    return Infinity;
  }
  if(date.equals(Date.NEG_INFIN)) {
    return -Infinity;
  }
  const value = new globalThis.Date(0);
  value.setUTCFullYear(date.year, date.month - 1, date.day);
  return value.getTime();
}

function shift(value: DateTime, ticks: number): DateTime {
  const date = dateTicks(value.date);
  const time = value.timeOfDay.ticks + ticks;
  const total = date + time;
  if(Number.isNaN(total)) {
    return DateTime.NOT_A_DATE_TIME;
  }
  if(total === Infinity) {
    return DateTime.POS_INFIN;
  }
  if(total === -Infinity) {
    return DateTime.NEG_INFIN;
  }
  const TICKS_PER_DAY = 86400000;
  const days = Math.floor(time / TICKS_PER_DAY);
  const next = new globalThis.Date(date + days * TICKS_PER_DAY);
  return new DateTime(new Date(next.getUTCFullYear(), next.getUTCMonth() + 1,
    next.getUTCDate()), new Duration(time - days * TICKS_PER_DAY));
}
