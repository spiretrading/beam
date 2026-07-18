module;
#include "Prelude.hpp"

export module Beam:ShuttleQueryTypes;

import :StandardValues;

export namespace Beam {
  BEAM_REGISTER_TYPES(register_value_types,
    (BoolValue, "Beam.Queries.BoolValue"),
    (CharValue, "Beam.Queries.CharValue"),
    (IntValue, "Beam.Queries.IntValue"),
    (DecimalValue, "Beam.Queries.DecimalValue"),
    (IdValue, "Beam.Queries.IdValue"),
    (StringValue, "Beam.Queries.StringValue"),
    (DateTimeValue, "Beam.Queries.DateTimeValue"));

  BEAM_REGISTER_TYPES(register_expression_types,
    (AndExpression, "Beam.Queries.AndExpression"),
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

  template<IsSender S>
  void register_query_types(Out<TypeRegistry<S>> registry) {
    registry->add(typeid(bool), "bool");
    registry->add(typeid(char), "char");
    registry->add(typeid(int), "int");
    registry->add(typeid(double), "double");
    registry->add(typeid(std::uint64_t), "uint64");
    registry->add(typeid(std::string), "string");
    registry->add(typeid(boost::posix_time::ptime), "boost.posix_time.ptime");
    registry->add(typeid(boost::posix_time::time_duration),
      "boost.posix_time.time_duration");
    register_value_types(out(registry));
    register_expression_types(out(registry));
  }
}

