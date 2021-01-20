#ifndef BEAM_SHUTTLE_QUERY_TYPES_HPP
#define BEAM_SHUTTLE_QUERY_TYPES_HPP
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/FunctionExpression.hpp"
#include "Beam/Queries/GlobalVariableDeclarationExpression.hpp"
#include "Beam/Queries/MemberAccessExpression.hpp"
#include "Beam/Queries/NotExpression.hpp"
#include "Beam/Queries/OrExpression.hpp"
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/ReduceExpression.hpp"
#include "Beam/Queries/SetVariableExpression.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Queries/StandardValues.hpp"
#include "Beam/Queries/VariableExpression.hpp"
#include "Beam/Serialization/TypeRegistry.hpp"

namespace Beam::Queries {
  BEAM_REGISTER_TYPES(RegisterDataTypes,
    (BoolType, "Beam.Queries.BoolType"),
    (CharType, "Beam.Queries.CharType"),
    (IntType, "Beam.Queries.IntType"),
    (DecimalType, "Beam.Queries.DecimalType"),
    (IdType, "Beam.Queries.IdType"),
    (StringType, "Beam.Queries.StringType"),
    (DateTimeType, "Beam.Queries.DateTimeType"));

  BEAM_REGISTER_TYPES(RegisterValueTypes,
    (BoolValue, "Beam.Queries.BoolValue"),
    (CharValue, "Beam.Queries.CharValue"),
    (IntValue, "Beam.Queries.IntValue"),
    (DecimalValue, "Beam.Queries.DecimalValue"),
    (IdValue, "Beam.Queries.IdValue"),
    (StringValue, "Beam.Queries.StringValue"),
    (DateTimeValue, "Beam.Queries.DateTimeValue"));

  BEAM_REGISTER_TYPES(RegisterExpressionTypes,
    (ConstantExpression, "Beam.Queries.ConstantExpression"),
    (FunctionExpression, "Beam.Queries.FunctionExpression"),
    (GlobalVariableDeclarationExpression,
      "Beam.Queries.GlobalVariableDeclarationExpression"),
    (MemberAccessExpression, "Beam.Queries.MemberAccessExpression"),
    (NotExpression, "Beam.Queries.NotExpression"),
    (OrExpression, "Beam.Queries.OrExpression"),
    (ParameterExpression, "Beam.Queries.ParameterExpression"),
    (ReduceExpression, "Beam.Queries.ReduceExpression"),
    (SetVariableExpression, "Beam.Queries.SetVariableExpression"),
    (VariableExpression, "Beam.Queries.VariableExpression"));

  template<typename SenderType>
  void RegisterQueryTypes(
      Out<Serialization::TypeRegistry<SenderType>> registry) {
    RegisterDataTypes(Store(registry));
    RegisterValueTypes(Store(registry));
    RegisterExpressionTypes(Store(registry));
  }
}

#endif
