import { describe, it } from 'node:test';
import * as assert from 'node:assert';
import { DateTime } from '../../source/definitions';
import { AndExpression } from '../../source/queries/and_expression';
import { ConstantExpression } from
  '../../source/queries/constant_expression';
import { checkBoolean, Expression } from '../../source/queries/expression';
import { FilteredQuery } from '../../source/queries/filtered_query';
import { MemberAccessExpression } from
  '../../source/queries/member_access_expression';
import { NotExpression } from '../../source/queries/not_expression';
import { OrExpression } from '../../source/queries/or_expression';
import { ParameterExpression } from
  '../../source/queries/parameter_expression';
import { QueryType } from '../../source/queries/query_type';
import { makeEquals, makeGreaterEquals } from
  '../../source/queries/standard_function_expressions';
import { DateTimeValue, IntValue } from
  '../../source/queries/standard_values';

/**
 * The JSON that Beam's C++ JsonSender produces for the filter built by
 * makeFilter, with the optional __version members removed. JsonReceiver
 * defaults a missing __version to 0, so this is what the web API must send.
 */
const CPP_FILTER = {
  filter: {
    expression: {
      __type: 'Beam.Queries.AndExpression',
      left: {
        expression: {
          __type: 'Beam.Queries.OrExpression',
          left: {
            expression: {
              __type: 'Beam.Queries.FunctionExpression',
              name: '==',
              type: 'bool',
              parameters: [{
                expression: {
                  __type: 'Beam.Queries.MemberAccessExpression',
                  name: 'id',
                  type: 'int',
                  expression: {
                    expression: {
                      __type: 'Beam.Queries.ParameterExpression',
                      index: 0,
                      type: 'int'
                    }
                  }
                }
              }, {
                expression: {
                  __type: 'Beam.Queries.ConstantExpression',
                  value: {
                    value: {__type: 'Beam.Queries.IntValue', value: 5}
                  }
                }
              }]
            }
          },
          right: {
            expression: {
              __type: 'Beam.Queries.FunctionExpression',
              name: '>=',
              type: 'bool',
              parameters: [{
                expression: {
                  __type: 'Beam.Queries.ParameterExpression',
                  index: 1,
                  type: 'boost.posix_time.ptime'
                }
              }, {
                expression: {
                  __type: 'Beam.Queries.ConstantExpression',
                  value: {
                    value: {
                      __type: 'Beam.Queries.DateTimeValue',
                      value: '20200102T030405'
                    }
                  }
                }
              }]
            }
          }
        }
      },
      right: {
        expression: {
          __type: 'Beam.Queries.NotExpression',
          operand: {
            expression: {
              __type: 'Beam.Queries.ConstantExpression',
              value: {
                value: {__type: 'Beam.Queries.BoolValue', value: false}
              }
            }
          }
        }
      }
    }
  }
};

function makeFilter(): FilteredQuery {
  const request = new ParameterExpression(0, QueryType.INT);
  return new FilteredQuery(new AndExpression(
    new OrExpression(
      makeEquals(new MemberAccessExpression('id', QueryType.INT, request),
        new ConstantExpression(new IntValue(5))),
      makeGreaterEquals(new ParameterExpression(1, QueryType.DATE_TIME),
        new ConstantExpression(
          new DateTimeValue(DateTime.fromJson('20200102T030405'))))),
    new NotExpression(ConstantExpression.FALSE)));
}

describe('Expression', () => {
  it('nested_json', () => {
    const nested = Expression.nestedToJson(ConstantExpression.TRUE);
    assert.deepStrictEqual(nested,
      {expression: ConstantExpression.TRUE.toJson()});
    assert.ok(
      Expression.nestedFromJson(nested).equals(ConstantExpression.TRUE));
  });

  it('check_boolean', () => {
    assert.strictEqual(
      checkBoolean(ConstantExpression.TRUE), ConstantExpression.TRUE);
    assert.throws(
      () => checkBoolean(new ConstantExpression(new IntValue(1))), TypeError);
  });

  it('matches_the_cpp_wire_format', () => {
    const json = JSON.parse(JSON.stringify(makeFilter().toJson()));
    assert.deepStrictEqual(json, CPP_FILTER);
  });

  it('parses_the_cpp_wire_format', () => {
    const restored = FilteredQuery.fromJson(CPP_FILTER);
    assert.ok(restored.filter.equals(makeFilter().filter));
  });

  it('tolerates_a_received_version', () => {
    const json = {
      __version: 0,
      filter: {
        __version: 0,
        expression: {
          __type: 'Beam.Queries.ConstantExpression',
          __version: 0,
          value: {
            __version: 0,
            value: {
              __type: 'Beam.Queries.BoolValue', __version: 0, value: true
            }
          }
        }
      }
    };
    assert.ok(
      FilteredQuery.fromJson(json).filter.equals(ConstantExpression.TRUE));
  });
});
