#include "Beam/Python/Queries.hpp"
#include <sstream>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include "Beam/Python/DateTime.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/Queues.hpp"
#include "Beam/Python/Variant.hpp"
#include "Beam/Queries/AndExpression.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/DataType.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionQuery.hpp"
#include "Beam/Queries/ExpressionTranslationException.hpp"
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/FunctionExpression.hpp"
#include "Beam/Queries/IndexListQuery.hpp"
#include "Beam/Queries/InterruptableQuery.hpp"
#include "Beam/Queries/InterruptionPolicy.hpp"
#include "Beam/Queries/NotExpression.hpp"
#include "Beam/Queries/OrExpression.hpp"
#include "Beam/Queries/QueryInterruptedException.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/RangedQuery.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Queries/SnapshotLimit.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"
#include "Beam/Queries/StandardValues.hpp"
#include "Beam/Queries/TypeCompatibilityException.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Queries;
using namespace boost;
using namespace boost::posix_time;
using namespace pybind11;

void Beam::Python::ExportAndExpression(pybind11::module& module) {
  class_<AndExpression, VirtualExpression>(module, "AndExpression").
    def(init<Expression, Expression>()).
    def(init<const AndExpression&>()).
    def_property_readonly("left_expression", &AndExpression::GetLeftExpression).
    def_property_readonly("right_expression",
      &AndExpression::GetRightExpression);
  implicitly_convertible<AndExpression, Expression>();
}

void Beam::Python::ExportExpression(pybind11::module& module) {
  class_<VirtualExpression>(module, "Expression").
    def_property_readonly("data_type", &VirtualExpression::GetType).
    def("apply", &VirtualExpression::Apply).
    def(self < self).
    def(self <= self).
    def(self == self).
    def(self != self).
    def(self >= self).
    def(self > self).
    def(self + self).
    def(self - self).
    def(self * self).
    def(self / self);
  class_<Expression>(module, "CloneableExpression").
    def(init<const VirtualExpression&>());
  implicitly_convertible<VirtualExpression, Expression>();
}

void Beam::Python::ExportConstantExpression(pybind11::module& module) {
  class_<ConstantExpression, VirtualExpression>(module, "ConstantExpression").
    def(init<const Value&>()).
    def_property_readonly("value", &ConstantExpression::GetValue);
  implicitly_convertible<ConstantExpression, Expression>();
}

void Beam::Python::ExportDataType(pybind11::module& module) {
  class_<VirtualDataType>(module, "DataType").
    def("__eq__",
      [] (const VirtualDataType& self, const VirtualDataType& rhs) {
        return self == rhs;
      }).
    def("__ne__",
      [] (const VirtualDataType& self, const VirtualDataType& rhs) {
        return self != rhs;
      });
  class_<DataType>(module, "CloneableDataType").
    def(init<const VirtualDataType&>());
  implicitly_convertible<VirtualDataType, DataType>();
  ExportNativeDataType<BoolType>(module, "BoolType");
  ExportNativeDataType<CharType>(module, "CharType");
  ExportNativeDataType<IntType>(module, "IntType");
  ExportNativeDataType<DecimalType>(module, "DecimalType");
  ExportNativeDataType<IdType>(module, "IdType");
  ExportNativeDataType<StringType>(module, "StringType");
  ExportNativeDataType<DateTimeType>(module, "DateTimeType");
  ExportNativeDataType<DurationType>(module, "DurationType");
}

void Beam::Python::ExportExpressionQuery(module& module) {
  auto outer = class_<ExpressionQuery>(module, "ExpressionQuery").
    def(init()).
    def(init<const Expression&>()).
    def(init<const ExpressionQuery&>()).
    def_property("update_policy", &ExpressionQuery::GetUpdatePolicy,
      &ExpressionQuery::SetUpdatePolicy).
    def_property("expression", &ExpressionQuery::GetExpression,
      &ExpressionQuery::SetExpression).
    def("__str__", &lexical_cast<std::string, ExpressionQuery>);
  enum_<ExpressionQuery::UpdatePolicy>(outer, "UpdatePolicy").
    value("ALL", ExpressionQuery::UpdatePolicy::ALL).
    value("CHANGE", ExpressionQuery::UpdatePolicy::CHANGE);
}

void Beam::Python::ExportFilteredQuery(pybind11::module& module) {
  class_<FilteredQuery>(module, "FilteredQuery").
    def(init()).
    def(init<const FilteredQuery&>()).
    def(init<const Expression&>()).
    def_property("filter", &FilteredQuery::GetFilter,
      &FilteredQuery::SetFilter).
    def("__str__", &lexical_cast<std::string, FilteredQuery>);
}

void Beam::Python::ExportFunctionExpression(pybind11::module& module) {
  class_<FunctionExpression, VirtualExpression>(module, "FunctionExpression").
    def(init<std::string, const DataType&, std::vector<Expression>>()).
    def_property_readonly("name", &FunctionExpression::GetName).
    def_property_readonly("parameters", &FunctionExpression::GetParameters);
  implicitly_convertible<FunctionExpression, Expression>();
}

void Beam::Python::ExportIndexedValue(pybind11::module& module) {
  class_<IndexedValue<object, object>>(module, "IndexedValue").
    def(init()).
    def(init<const object&, const object&>()).
    def(init<const IndexedValue<object, object>&>()).
    def_property_readonly("value", static_cast<
      const object& (IndexedValue<object, object>::*)() const>(
      &IndexedValue<object, object>::GetValue)).
    def_property_readonly("index", static_cast<
      const object& (IndexedValue<object, object>::*)() const>(
      &IndexedValue<object, object>::GetIndex)).
    def("__eq__",
      [] (IndexedValue<object, object>& self,
          const IndexedValue<object, object>& other) {
        return self.GetIndex().attr("__eq__")(other.GetIndex()).cast<bool>() &&
          self.GetValue().attr("__eq__")(other.GetValue()).cast<bool>();
      }).
    def("__ne__",
      [] (IndexedValue<object, object>& self,
          const IndexedValue<object, object>& other) {
        return self.GetIndex().attr("__ne__")(other.GetIndex()).cast<bool>() ||
          self.GetValue().attr("__ne__")(other.GetValue()).cast<bool>();
      }).
    def("__str__", &lexical_cast<std::string, IndexedValue<object, object>>);
}

void Beam::Python::ExportIndexListQuery(pybind11::module& module) {
  class_<IndexListQuery, SnapshotLimitedQuery, FilteredQuery>(module,
      "IndexListQuery").
    def(init()).
    def(init<const IndexListQuery&>()).
    def("__str__", &lexical_cast<std::string, IndexListQuery>);
}

void Beam::Python::ExportInterruptableQuery(pybind11::module& module) {
  class_<InterruptableQuery>(module, "InterruptableQuery").
    def(init()).
    def(init<const InterruptableQuery&>()).
    def(init<InterruptionPolicy>()).
    def_property("interruption_policy",
      &InterruptableQuery::GetInterruptionPolicy,
      &InterruptableQuery::SetInterruptionPolicy).
    def("__str__", &lexical_cast<std::string, InterruptableQuery>);
}

void Beam::Python::ExportInterruptionPolicy(pybind11::module& module) {
  enum_<InterruptionPolicy>(module, "InterruptionPolicy").
    value("BREAK_QUERY", InterruptionPolicy::BREAK_QUERY).
    value("RECOVER_DATA", InterruptionPolicy::RECOVER_DATA).
    value("IGNORE_CONTINUE", InterruptionPolicy::IGNORE_CONTINUE);
}

void Beam::Python::ExportMemberAccessExpression(pybind11::module& module) {
  class_<MemberAccessExpression, VirtualExpression>(module,
    "MemberAccessExpression").
    def(init<std::string, const DataType&, const Expression&>()).
    def_property_readonly("name", &MemberAccessExpression::GetName).
    def_property_readonly("expression",
      &MemberAccessExpression::GetExpression);
  implicitly_convertible<MemberAccessExpression, Expression>();
}

void Beam::Python::ExportNotExpression(pybind11::module& module) {
  class_<NotExpression, VirtualExpression>(module, "NotExpression").
    def(init<Expression>()).
    def(init<const NotExpression&>()).
    def_property_readonly("operand", &NotExpression::GetOperand);
  implicitly_convertible<NotExpression, Expression>();
}

void Beam::Python::ExportOrExpression(pybind11::module& module) {
  class_<OrExpression, VirtualExpression>(module, "OrExpression").
    def(init<Expression, Expression>()).
    def(init<const OrExpression&>()).
    def_property_readonly("left_expression", &OrExpression::GetLeftExpression).
    def_property_readonly("right_expression",
      &OrExpression::GetRightExpression);
  implicitly_convertible<OrExpression, Expression>();
}

void Beam::Python::ExportParameterExpression(pybind11::module& module) {
  class_<ParameterExpression, VirtualExpression>(module, "ParameterExpression").
    def(init<int, const DataType&>()).
    def_property_readonly("index", &ParameterExpression::GetIndex);
  implicitly_convertible<ParameterExpression, Expression>();
}

void Beam::Python::ExportQueries(pybind11::module& module) {
  auto submodule = module.def_submodule("queries");
  ExportExpression(submodule);
  ExportConstantExpression(submodule);
  ExportDataType(submodule);
  ExportInterruptableQuery(submodule);
  ExportInterruptionPolicy(submodule);
  ExportExpressionQuery(submodule);
  ExportFilteredQuery(submodule);
  ExportFunctionExpression(submodule);
  ExportIndexedQuery<object>(submodule, "IndexedQuery");
  ExportIndexedValue(submodule);
  ExportMemberAccessExpression(submodule);
  ExportAndExpression(submodule);
  ExportNotExpression(submodule);
  ExportOrExpression(submodule);
  ExportParameterExpression(submodule);
  ExportRange(submodule);
  ExportRangedQuery(submodule);
  ExportSequence(submodule);
  ExportSequencedValue(submodule);
  ExportSnapshotLimit(submodule);
  ExportSnapshotLimitedQuery(submodule);
  ExportValue(submodule);
  ExportIndexListQuery(submodule);
  ExportBasicQuery<object>(submodule, "Query");
  ExportPagedQuery<object, object>(submodule, "PagedQuery");
  ExportQueueSuite<QueryVariant>(submodule, "QueryVariant");
  ExportQueueSuite<bool>(submodule, "Bool");
  ExportQueueSuite<char>(submodule, "Char");
  ExportQueueSuite<int>(submodule, "Int");
  ExportQueueSuite<double>(submodule, "Double");
  ExportQueueSuite<std::uint64_t>(submodule, "UInt64");
  ExportQueueSuite<std::string>(submodule, "String");
  ExportQueueSuite<ptime>(submodule, "DateTime");
  ExportQueueSuite<time_duration>(submodule, "TimeDuration");
  submodule.def("make_current_query", &MakeCurrentQuery<object>);
  submodule.def("make_latest_query", &MakeRealTimeQuery<object>);
  submodule.def("make_real_time_query", &MakeRealTimeQuery<object>);
  register_exception<ExpressionTranslationException>(submodule,
    "ExpressionTranslationException");
  register_exception<QueryInterruptedException>(submodule,
    "QueryInterruptedException");
  register_exception<TypeCompatibilityException>(submodule,
    "TypeCompatibilityException");
}

void Beam::Python::ExportRange(pybind11::module& module) {
  class_<Range>(module, "Range").
    def(init()).
    def(init<const Range::Point&, const Range::Point&>()).
    def_property_readonly_static("EMPTY",
      [] (const object&) { return Range::Empty(); }).
    def_property_readonly_static("HISTORICAL",
      [] (const object&) { return Range::Historical(); }).
    def_property_readonly_static("TOTAL",
      [] (const object&) { return Range::Total(); }).
    def_property_readonly_static("REAL_TIME",
      [] (const object&) { return Range::RealTime(); }).
    def_property_readonly("start", &Range::GetStart).
    def_property_readonly("end", &Range::GetEnd).
    def(self == self).
    def(self != self).
    def("__hash__", [] (Queries::Range self) {
      return std::hash<Queries::Range>()(self);
    }).
    def("__str__", &lexical_cast<std::string, Range>);
}

void Beam::Python::ExportRangedQuery(pybind11::module& module) {
  class_<RangedQuery>(module, "RangedQuery").
    def(init()).
    def(init<const RangedQuery&>()).
    def_property("range", &RangedQuery::GetRange,
      static_cast<void (RangedQuery::*)(const Range&)>(&RangedQuery::SetRange)).
    def("set_range", static_cast<
      void (RangedQuery::*)(const Range::Point&, const Range::Point&)>(
      &RangedQuery::SetRange<const Range::Point&, const Range::Point&>)).
    def("__str__", &lexical_cast<std::string, RangedQuery>);
}

void Beam::Python::ExportSequence(pybind11::module& module) {
  class_<Queries::Sequence>(module, "Sequence").
    def(init()).
    def(init<Queries::Sequence::Ordinal>()).
    def_property_readonly_static("FIRST",
      [] (const object&) { return Queries::Sequence::First(); }).
    def_property_readonly_static("LAST",
      [] (const object&) { return Queries::Sequence::Last(); }).
    def_property_readonly_static("PRESENT",
      [] (const object&) { return Queries::Sequence::Present(); }).
    def_property_readonly("ordinal", &Queries::Sequence::GetOrdinal).
    def(self < self).
    def(self <= self).
    def(self == self).
    def(self != self).
    def(self >= self).
    def(self > self).
    def("__hash__", [] (Queries::Sequence self) {
      return std::hash<Queries::Sequence>()(self);
    }).
    def("__str__", &lexical_cast<std::string, Queries::Sequence>);
  module.def("increment", &Increment);
  module.def("decrement", &Decrement);
}

void Beam::Python::ExportSequencedValue(pybind11::module& module) {
  class_<SequencedValue<object>>(module, "SequencedValue").
    def(init()).
    def(init<object, Queries::Sequence>()).
    def_property_readonly("value",
      static_cast<const object& (SequencedValue<object>::*)() const>(
      &SequencedValue<object>::GetValue)).
    def_property_readonly("sequence", static_cast<Queries::Sequence (
      SequencedValue<object>::*)() const>(
        &SequencedValue<object>::GetSequence)).
    def("__eq__",
      [] (SequencedValue<object>& self,
          const SequencedValue<object>& other) {
        return self.GetSequence() == other.GetSequence() &&
          self.GetValue().attr("__eq__")(other.GetValue()).cast<bool>();
      }).
    def("__ne__",
      [] (SequencedValue<object>& self, const SequencedValue<object>& other) {
        return self.GetSequence() != other.GetSequence() ||
          self.GetValue().attr("__ne__")(other.GetValue()).cast<bool>();
      }).
    def("__str__", &lexical_cast<std::string, SequencedValue<object>>);
}

void Beam::Python::ExportSnapshotLimit(pybind11::module& module) {
  auto outer = class_<SnapshotLimit>(module, "SnapshotLimit").
    def(init()).
    def(init<SnapshotLimit::Type, int>()).
    def_property_readonly_static("NONE",
      [] (const object&) { return SnapshotLimit::None(); }).
    def_property_readonly_static("UNLIMITED",
      [] (const object&) { return SnapshotLimit::Unlimited(); }).
    def_property_readonly("type", &SnapshotLimit::GetType).
    def_property_readonly("size", &SnapshotLimit::GetSize).
    def_static("from_head", &SnapshotLimit::FromHead).
    def_static("from_tail", &SnapshotLimit::FromTail).
    def("__str__", &lexical_cast<std::string, SnapshotLimit>).
    def(self == self).
    def(self != self);
  enum_<SnapshotLimit::Type>(outer, "Type").
    value("HEAD", SnapshotLimit::Type::HEAD).
    value("TAIL", SnapshotLimit::Type::TAIL);
}

void Beam::Python::ExportSnapshotLimitedQuery(pybind11::module& module) {
  class_<SnapshotLimitedQuery>(module, "SnapshotLimitedQuery").
    def(init()).
    def(init<const SnapshotLimitedQuery&>()).
    def(init<SnapshotLimit>()).
    def_property("snapshot_limit", &SnapshotLimitedQuery::GetSnapshotLimit,
      static_cast<void (SnapshotLimitedQuery::*)(const SnapshotLimit&)>(
        &SnapshotLimitedQuery::SetSnapshotLimit)).
    def("set_snapshot_limit", static_cast<
      void (SnapshotLimitedQuery::*)(SnapshotLimit::Type, int)>(
        &SnapshotLimitedQuery::SetSnapshotLimit)).
    def("__str__", &lexical_cast<std::string, SnapshotLimitedQuery>);
}

void Beam::Python::ExportValue(pybind11::module& module) {
  class_<VirtualValue>(module, "Value").
    def_property_readonly("data_type", &VirtualValue::GetType);
  class_<Value>(module, "CloneableValue").
    def(init<const VirtualValue&>());
  ExportNativeValue<BoolValue>(module, "BoolValue");
  ExportNativeValue<CharValue>(module, "CharValue");
  ExportNativeValue<IntValue>(module, "IntValue");
  ExportNativeValue<DecimalValue>(module, "DecimalValue");
  ExportNativeValue<IdValue>(module, "IdValue");
  ExportNativeValue<StringValue>(module, "StringValue");
  ExportNativeValue<DateTimeValue>(module, "DateTimeValue");
  ExportNativeValue<DurationValue>(module, "DurationValue");
}
