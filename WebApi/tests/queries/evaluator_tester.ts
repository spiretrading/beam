import * as assert from 'node:assert';
import { describe, it } from 'node:test';
import { AndExpression, BoolValue, CharValue, ConstantEvaluatorNode,
  ConstantExpression, DateTime, DateTimeValue, DecimalValue, Duration,
  DurationValue, Evaluator, EvaluatorNode, EvaluatorTranslator, Expression,
  ExpressionTranslationError, FunctionEvaluatorNode, FunctionExpression,
  IdValue, IntValue, makeAddition, makeDivision, makeEquals, makeGreater,
  makeGreaterEquals, makeLess, makeLessEquals, makeMax, makeMin,
  makeMultiplication, makeNotEquals, makeSubtraction, MemberAccessEvaluatorNode,
  MemberAccessExpression, NotExpression, OrExpression, ParameterEvaluatorNode,
  ParameterExpression, QueryType, StringValue, testFilter, translate,
  TypeCompatibilityError, Value } from '../../source';

function constant(value: Value): ConstantExpression {
  return new ConstantExpression(value);
}

class TimeTranslator extends EvaluatorTranslator {
  protected translateExpression(expression: Expression):
      EvaluatorNode<unknown> {
    if(expression instanceof MemberAccessExpression &&
        expression.expression.type === QueryType.DATE_TIME &&
        expression.name === 'time_of_day') {
      return new MemberAccessEvaluatorNode(QueryType.DURATION,
        this.translateOperand(expression.expression) as EvaluatorNode<DateTime>,
        value => value.timeOfDay);
    }
    return super.translateExpression(expression);
  }
}

describe('Evaluator', () => {
  it('constants_and_json', () => {
    const values = [new BoolValue(true), new CharValue('x'), new IntValue(-3),
      new DecimalValue(1.25), new IdValue(9007199254740993n),
      new StringValue('hello'),
      new DateTimeValue(DateTime.fromJson('20261109T123000')),
      new DurationValue(new Duration(2500))];
    for(const value of values) {
      const expression = Expression.fromJson(constant(value).toJson());
      assert.deepStrictEqual(translate(expression).eval(), value.value);
    }
  });

  it('boolean_short_circuit', () => {
    const failing = makeEquals(makeDivision(constant(new IntValue(1)),
      constant(new IntValue(0))), constant(new IntValue(0)));
    assert.ok(!translate(new AndExpression(ConstantExpression.FALSE,
      failing)).eval());
    assert.ok(translate(new OrExpression(ConstantExpression.TRUE,
      failing)).eval());
    assert.throws(() => translate(new AndExpression(ConstantExpression.TRUE,
      failing)).eval(), RangeError);
    assert.throws(() => translate(new OrExpression(ConstantExpression.FALSE,
      failing)).eval(), RangeError);
    assert.ok(translate(new NotExpression(ConstantExpression.FALSE)).eval());
    assert.ok(!translate(new NotExpression(ConstantExpression.TRUE)).eval());
  });

  it('arithmetic_and_promotions', () => {
    const cases: [Expression, unknown, QueryType][] = [
      [makeAddition(constant(new IntValue(2)), constant(new IntValue(3))),
        5, QueryType.INT],
      [makeSubtraction(constant(new IntValue(2)), constant(new IntValue(3))),
        -1, QueryType.INT],
      [makeMultiplication(constant(new IntValue(2)), constant(new IntValue(3))),
        6, QueryType.INT],
      [makeDivision(constant(new IntValue(-7)), constant(new IntValue(2))),
        -3, QueryType.INT],
      [makeDivision(constant(new IntValue(7)), constant(new DecimalValue(2))),
        3.5, QueryType.DECIMAL],
      [makeAddition(constant(new DecimalValue(2.5)), constant(new IntValue(1))),
        3.5, QueryType.DECIMAL],
      [makeMax(constant(new IntValue(2)), constant(new DecimalValue(2.5))),
        2.5, QueryType.DECIMAL],
      [makeMin(constant(new DecimalValue(2.5)), constant(new IntValue(2))),
        2, QueryType.DECIMAL],
      [makeAddition(constant(new StringValue('a')),
        constant(new StringValue('b'))), 'ab', QueryType.STRING],
      [makeAddition(constant(new CharValue('a')), constant(new CharValue('b'))),
        195, QueryType.INT],
      [makeAddition(ConstantExpression.TRUE, ConstantExpression.TRUE),
        2, QueryType.INT],
      [makeMin(ConstantExpression.TRUE, ConstantExpression.FALSE),
        false, QueryType.BOOL],
      [makeAddition(constant(new IdValue(9007199254740993n)),
        constant(new IdValue(2n))), 9007199254740995n, QueryType.ID],
      [makeSubtraction(constant(new IdValue(0n)), constant(new IdValue(1n))),
        18446744073709551615n, QueryType.ID],
      [makeDivision(constant(new IdValue(7n)), constant(new IdValue(2n))),
        3n, QueryType.ID]
    ];
    for(const [expression, expected, type] of cases) {
      assert.strictEqual(expression.type, type);
      assert.strictEqual(translate(expression).eval(), expected);
    }
  });

  it('comparisons', () => {
    const pairs = [
      [new IntValue(-1), new IdValue(0n)],
      [new IdValue(9007199254740993n), new IdValue(9007199254740994n)],
      [new IntValue(2), new DecimalValue(2.5)],
      [new CharValue('a'), new CharValue('b')],
      [new StringValue('first'), new StringValue('second')],
      [new DurationValue(new Duration(1)), new DurationValue(new Duration(2))],
      [new DateTimeValue(DateTime.fromJson('20261109T000000')),
        new DateTimeValue(DateTime.fromJson('20261110T000000'))]
    ];
    for(const [a, b] of pairs) {
      const left = constant(a);
      const right = constant(b);
      assert.ok(translate(makeLess(left, right)).eval());
      assert.ok(translate(makeLessEquals(left, right)).eval());
      assert.ok(translate(makeGreater(right, left)).eval());
      assert.ok(translate(makeGreaterEquals(right, left)).eval());
      assert.ok(translate(makeNotEquals(left, right)).eval());
      assert.ok(!translate(makeEquals(left, right)).eval());
      assert.ok(translate(makeEquals(left, left)).eval());
      assert.ok(translate(makeLessEquals(left, left)).eval());
    }
    assert.ok(translate(makeEquals(constant(new IntValue(1)),
      constant(new IdValue(1n)))).eval());
    assert.ok(!translate(makeEquals(constant(new DecimalValue(NaN)),
      constant(new DecimalValue(NaN)))).eval());
    assert.ok(!translate(makeLess(constant(new DecimalValue(NaN)),
      constant(new DecimalValue(1)))).eval());
  });

  it('time_arithmetic', () => {
    const start = constant(new DateTimeValue(
      DateTime.fromJson('20260228T235959.5')));
    const second = constant(new DurationValue(new Duration(1000)));
    const sum = makeAddition(start, second);
    assert.ok(translate<DateTime>(sum).eval().equals(
      DateTime.fromJson('20260301T000000.5')));
    assert.ok(translate<DateTime>(makeSubtraction(sum, second)).eval().equals(
      (start.value as DateTimeValue).value));
    const difference = makeSubtraction(sum, start);
    assert.strictEqual(difference.type, QueryType.DURATION);
    assert.ok(translate<Duration>(difference).eval().equals(
      new Duration(1000)));
    assert.ok(translate<Duration>(makeAddition(second, second)).eval().equals(
      new Duration(2000)));
    assert.ok(translate<Duration>(makeSubtraction(second, second)).eval().
      equals(Duration.ZERO));
    const microsecond = constant(new DurationValue(new Duration(0.001)));
    const midnight = constant(new DateTimeValue(
      DateTime.fromJson('20261109T000000')));
    assert.ok(translate<Duration>(makeSubtraction(
      makeAddition(midnight, microsecond), midnight)).eval().equals(
        new Duration(0.001)));
  });

  it('parameter_binding_and_validation', () => {
    const first = new ParameterExpression(0, QueryType.INT);
    const second = new ParameterExpression(1, QueryType.INT);
    const evaluator = translate<number>(makeSubtraction(first, second),
      QueryType.INT);
    assert.strictEqual(evaluator.eval(7, 2), 5);
    assert.strictEqual(evaluator.eval(2, 7), -5);
    assert.throws(() => evaluator.eval(), TypeCompatibilityError);
    assert.throws(() => evaluator.eval(1), TypeCompatibilityError);
    assert.throws(() => evaluator.eval('7', 2), TypeCompatibilityError);
    assert.throws(() => evaluator.eval(1.5, 2), TypeCompatibilityError);
    assert.throws(() => evaluator.eval(1, 2, 3), TypeCompatibilityError);
    assert.strictEqual(evaluator.eval(10, 5), 5);
    assert.throws(() => translate(second, QueryType.INT),
      ExpressionTranslationError);
    for(const index of [-1, 2, 0.5]) {
      assert.throws(() => translate(new ParameterExpression(index,
        QueryType.INT), QueryType.INT), ExpressionTranslationError);
    }
    assert.throws(() => translate(first), ExpressionTranslationError);
    assert.throws(() => translate(first, QueryType.STRING),
      ExpressionTranslationError);
    const translator = new EvaluatorTranslator(QueryType.INT);
    const previous = translate<number>(makeAddition(first, first), translator);
    assert.strictEqual(previous.eval(4), 8);
    assert.throws(() => translator.translate(second),
      ExpressionTranslationError);
    assert.strictEqual(translator.takeEvaluator(), null);
    assert.strictEqual(translator.getParameters().length, 0);
    assert.strictEqual(
      translate(constant(new IntValue(3)), translator).eval(), 3);
    assert.strictEqual(previous.eval(5), 10);
  });

  it('parameter_value_holder', () => {
    const node = new ParameterEvaluatorNode<number>(0, QueryType.INT);
    const parameter = {value: 3};
    node.setParameter(parameter);
    assert.strictEqual(node.eval(), 3);
    parameter.value = 7;
    assert.strictEqual(node.eval(), 7);
    node.setParameter({value: 11});
    parameter.value = 13;
    assert.strictEqual(node.eval(), 11);
  });

  it('shared_parameter_binding', () => {
    const first = new ParameterEvaluatorNode<number>(0, QueryType.INT);
    const second = new ParameterEvaluatorNode<number>(0, QueryType.INT);
    const third = new ParameterEvaluatorNode<number>(1, QueryType.INT);
    const root = new FunctionEvaluatorNode(QueryType.INT,
      (a, b, c) => a + b - c, [first, second, third]);
    const evaluator = new Evaluator(root, [first, second, third]);
    assert.strictEqual(evaluator.eval(7, 2), 12);
    assert.strictEqual(first.eval(), 7);
    assert.strictEqual(second.eval(), 7);
    assert.throws(() => evaluator.eval(10, '2'), TypeCompatibilityError);
    assert.strictEqual(root.eval(), 12);
    assert.strictEqual(evaluator.eval(3, 5), 1);
    assert.strictEqual(first.eval(), 3);
    assert.strictEqual(second.eval(), 3);
  });

  it('independent_parameter_bindings', () => {
    const parameter = new ParameterExpression(0, QueryType.INT);
    const translator = new EvaluatorTranslator(QueryType.INT);
    const first = translate<number>(makeAddition(parameter, parameter),
      translator);
    const second = translate<number>(makeMultiplication(parameter, parameter),
      translator);
    assert.strictEqual(first.eval(3), 6);
    assert.strictEqual(second.eval(5), 25);
    assert.strictEqual(first.eval(7), 14);
    assert.strictEqual(second.eval(2), 4);
  });

  it('translation_errors', () => {
    const one = constant(new IntValue(1));
    const unsupported = [
      new FunctionExpression('unknown', QueryType.INT, [one, one]),
      new FunctionExpression('+', QueryType.INT, [one]),
      new FunctionExpression('+', QueryType.BOOL, [one, one]),
      makeAddition(one, constant(new IdValue(1n))),
      makeMax(one, constant(new IdValue(1n))),
      makeLess(one, constant(new StringValue('1'))),
      makeLess(ConstantExpression.TRUE, ConstantExpression.FALSE),
      makeDivision(ConstantExpression.TRUE, ConstantExpression.TRUE),
      makeMultiplication(constant(new DurationValue(Duration.SECOND)), one),
      new MemberAccessExpression('unknown', QueryType.INT, one),
      constant(new IntValue(1.5))
    ];
    for(const expression of unsupported) {
      assert.throws(() => translate(expression), ExpressionTranslationError);
    }
  });

  it('explicit_member_translation', () => {
    const parameter = new ParameterExpression(0, QueryType.DATE_TIME);
    const member = new MemberAccessExpression('time_of_day', QueryType.DURATION,
      parameter);
    const expression = makeGreater(member,
      constant(new DurationValue(Duration.HOUR)));
    const translator = new TimeTranslator(QueryType.DATE_TIME);
    const evaluator = translate<boolean>(expression, translator);
    assert.ok(evaluator.eval(DateTime.fromJson('20261109T020000')));
    assert.ok(!evaluator.eval(DateTime.fromJson('20261109T003000')));
    assert.throws(() => translate(expression, QueryType.DATE_TIME),
      ExpressionTranslationError);
    assert.throws(() => translate(new MemberAccessExpression('time_of_day',
      QueryType.INT, parameter), translator), ExpressionTranslationError);
  });

  it('filter_errors_and_repeated_evaluation', () => {
    const parameter = new ParameterExpression(0, QueryType.INT);
    const filter = translate<boolean>(makeGreater(
      makeDivision(constant(new IntValue(10)), parameter),
      constant(new IntValue(1))), QueryType.INT);
    assert.ok(testFilter(filter, 2));
    assert.ok(!testFilter(filter, 10));
    assert.ok(!testFilter(filter, 0));
    assert.ok(!testFilter(filter, '2'));
    assert.ok(testFilter(filter, 5));
    const node = new FunctionEvaluatorNode(QueryType.BOOL, () => {
      throw new Error('Evaluation failed.');
    }, [new ConstantEvaluatorNode(QueryType.INT, 1)]);
    assert.ok(!testFilter(new Evaluator<boolean>(node, []), 1));
  });
});
