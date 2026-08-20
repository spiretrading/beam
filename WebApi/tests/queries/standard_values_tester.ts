import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { DateTime } from '../../source/definitions';
import { QueryType } from '../../source/queries/query_type';
import { BoolValue, CharValue, DateTimeValue, DecimalValue, IdValue,
  IntValue, StringValue } from '../../source/queries/standard_values';
import { Value } from '../../source/queries/value';

describe('StandardValues', () => {
  it('type', () => {
    assert.strictEqual(new BoolValue(true).type, QueryType.BOOL);
    assert.strictEqual(new CharValue('a').type, QueryType.CHAR);
    assert.strictEqual(new IntValue(1).type, QueryType.INT);
    assert.strictEqual(new DecimalValue(1.5).type, QueryType.DECIMAL);
    assert.strictEqual(new IdValue(1n).type, QueryType.ID);
    assert.strictEqual(new StringValue('a').type, QueryType.STRING);
    assert.strictEqual(
      new DateTimeValue(DateTime.NOT_A_DATE_TIME).type, QueryType.DATE_TIME);
  });

  it('to_string', () => {
    assert.strictEqual(new BoolValue(true).toString(), 'true');
    assert.strictEqual(new BoolValue(false).toString(), 'false');
    assert.strictEqual(new CharValue('a').toString(), 'a');
    assert.strictEqual(new IntValue(-5).toString(), '-5');
    assert.strictEqual(new DecimalValue(1.5).toString(), '1.5');
    assert.strictEqual(new IdValue(18446744073709551615n).toString(),
      '18446744073709551615');
    assert.strictEqual(new StringValue('hello').toString(), '"hello"');
  });

  it('equals', () => {
    assert.ok(new IntValue(1).equals(new IntValue(1)));
    assert.ok(!new IntValue(1).equals(new IntValue(2)));
    assert.ok(!new IntValue(1).equals(new DecimalValue(1)));
    assert.ok(new IdValue(1n).equals(new IdValue(1n)));
    assert.ok(new DateTimeValue(DateTime.POS_INFIN).equals(
      new DateTimeValue(DateTime.POS_INFIN)));
    assert.ok(!new DateTimeValue(DateTime.POS_INFIN).equals(
      new DateTimeValue(DateTime.NEG_INFIN)));
  });

  it('round_trip', () => {
    const values = [new BoolValue(true), new BoolValue(false),
      new CharValue('a'), new IntValue(-5), new DecimalValue(1.5),
      new IdValue(18446744073709551615n), new StringValue('hello'),
      new DateTimeValue(DateTime.fromJson('20200102T030405'))];
    for(const value of values) {
      const json = JSON.parse(JSON.stringify(value.toJson()));
      const restored = Value.fromJson(json);
      assert.strictEqual(restored.type, value.type);
      assert.ok(restored.equals(value), `${value} did not round trip.`);
    }
  });

  it('sends_the_registered_type_names', () => {
    assert.strictEqual(
      new BoolValue(true).toJson().__type, 'Beam.Queries.BoolValue');
    assert.strictEqual(
      new CharValue('a').toJson().__type, 'Beam.Queries.CharValue');
    assert.strictEqual(
      new IntValue(1).toJson().__type, 'Beam.Queries.IntValue');
    assert.strictEqual(
      new DecimalValue(1).toJson().__type, 'Beam.Queries.DecimalValue');
    assert.strictEqual(
      new IdValue(1n).toJson().__type, 'Beam.Queries.IdValue');
    assert.strictEqual(
      new StringValue('a').toJson().__type, 'Beam.Queries.StringValue');
    assert.strictEqual(new DateTimeValue(DateTime.POS_INFIN).toJson().__type,
      'Beam.Queries.DateTimeValue');
  });

  it('sends_wide_integers_as_strings', () => {
    assert.strictEqual(
      new IdValue(18446744073709551615n).toJson().value,
      '18446744073709551615');
    assert.strictEqual(new IntValue(5).toJson().value, 5);
  });

  it('sends_a_null_character_as_an_integer', () => {
    assert.strictEqual(new CharValue('\0').toJson().value, 0);
    assert.strictEqual(new CharValue('a').toJson().value, 'a');
    assert.strictEqual(CharValue.fromJson({value: 0}).value, '\0');
  });

  it('rejects_an_unregistered_value', () => {
    assert.throws(
      () => Value.fromJson({__type: 'Beam.Queries.DurationValue', value: '1'}),
      TypeError);
  });
});
