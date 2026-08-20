import { QueryType } from './query_type';

/** Base class for an expression used in a query. */
export abstract class Expression {

  /**
   * Registers a type of Expression so that it can be parsed from JSON.
   * @param name - The name identifying the type.
   * @param parser - Constructs an Expression from its JSON representation.
   */
  public static register(name: string, parser: Expression.Parser): void {
    PARSERS.set(name, parser);
  }

  /** Constructs an Expression from a JSON object. */
  public static fromJson(value: any): Expression {
    const parser = PARSERS.get(value.__type);
    if(parser === undefined) {
      throw new TypeError(`Unregistered expression type: ${value.__type}`);
    }
    return parser(value);
  }

  /** Constructs an Expression stored within another object from JSON. */
  public static nestedFromJson(value: any): Expression {
    return Expression.fromJson(value.expression);
  }

  /** Converts an Expression stored within another object to JSON. */
  public static nestedToJson(expression: Expression): any {
    return {
      expression: expression.toJson()
    };
  }

  /** Returns the type that this Expression evaluates to. */
  public abstract get type(): QueryType;

  /** Tests if two expressions evaluate the same way. */
  public abstract equals(other: Expression): boolean;

  public abstract toString(): string;

  /** Converts this object to JSON. */
  public abstract toJson(): any;
}

export namespace Expression {

  /** Constructs an Expression from its JSON representation. */
  export type Parser = (value: any) => Expression;
}

/**
 * Tests that an Expression evaluates to a boolean.
 * @param expression - The Expression to test.
 * @return The expression that was tested.
 */
export function checkBoolean(expression: Expression): Expression {
  if(expression.type !== QueryType.BOOL) {
    throw new TypeError('Expression must be bool.');
  }
  return expression;
}

const PARSERS = new Map<string, Expression.Parser>();
