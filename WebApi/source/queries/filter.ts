/**
 * Returns the JSON representation of a filter that accepts every value.
 * Expressions are not yet modelled by this API, so every query sends this
 * filter and ignores whatever filter it receives.
 */
export function makeAllValuesFilter(): any {
  return {
    expression: {
      __type: 'Beam.Queries.ConstantExpression',
      value: {
        value: {
          __type: 'Beam.Queries.BoolValue',
          value: true
        }
      }
    }
  };
}
