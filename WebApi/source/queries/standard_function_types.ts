import { QueryType } from './query_type';

/**
 * Returns a standard function's result type, or null for unsupported types.
 * @param name - The standard function's name.
 * @param left - The left operand's type.
 * @param right - The right operand's type.
 */
export function getStandardFunctionType(
    name: string, left: QueryType, right: QueryType): QueryType {
  if(!Object.values(QueryType).includes(left) ||
      !Object.values(QueryType).includes(right)) {
    return null;
  }
  const mixedDecimal = left === QueryType.INT && right === QueryType.DECIMAL ||
    left === QueryType.DECIMAL && right === QueryType.INT;
  const mixedInteger = left === QueryType.INT && right === QueryType.ID ||
    left === QueryType.ID && right === QueryType.INT;
  if(['==', '!=', '<', '<=', '>', '>='].includes(name)) {
    if(left !== right && !mixedDecimal && !mixedInteger) {
      return null;
    }
    if(left === QueryType.BOOL && name !== '==' && name !== '!=') {
      return null;
    }
    return QueryType.BOOL;
  }
  if(name === 'min' || name === 'max') {
    if(mixedDecimal) {
      return QueryType.DECIMAL;
    }
    if(left === right) {
      return left;
    }
    return null;
  }
  if(!['+', '-', '*', '/'].includes(name)) {
    return null;
  }
  if(left === QueryType.DATE_TIME && right === QueryType.DURATION &&
      (name === '+' || name === '-')) {
    return QueryType.DATE_TIME;
  }
  if(mixedDecimal) {
    return QueryType.DECIMAL;
  }
  if(left !== right) {
    return null;
  }
  if(left === QueryType.INT || left === QueryType.DECIMAL ||
      left === QueryType.ID) {
    return left;
  }
  if(left === QueryType.CHAR ||
      left === QueryType.BOOL && name !== '/') {
    return QueryType.INT;
  }
  if(left === QueryType.STRING && name === '+') {
    return QueryType.STRING;
  }
  if(left === QueryType.DURATION && (name === '+' || name === '-')) {
    return QueryType.DURATION;
  }
  if(left === QueryType.DATE_TIME && name === '-') {
    return QueryType.DURATION;
  }
  return null;
}
