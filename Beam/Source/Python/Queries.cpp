#include "Beam/Python/Queries.hpp"
#include <sstream>
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/Variant.hpp"
#include "Beam/Python/Vector.hpp"
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/DataType.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/FunctionExpression.hpp"
#include "Beam/Queries/InterruptableQuery.hpp"
#include "Beam/Queries/InterruptionPolicy.hpp"
#include "Beam/Queries/ExpressionTranslationException.hpp"
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
using namespace boost::python;
using namespace std;

namespace Beam {
  bool operator ==(const Expression& lhs, const Expression& rhs) {
    return &*lhs == &*rhs;
  }
}

namespace {
  string SnapshotLimitToString(const SnapshotLimit& snapshotLimit) {
    stringstream ss;
    ss << snapshotLimit;
    return ss.str();
  }

  Expression MakeCloneableExpression(const VirtualExpression& expression) {
    return Expression(expression);
  }

  DataType MakeCloneableDataType(const VirtualDataType& dataType) {
    return DataType(dataType);
  }

  Value MakeCloneableValue(const VirtualValue& value) {
    return Value(value);
  }

  FunctionExpression* MakeFunctionExpression(string name,
      const VirtualDataType& dataType, const boost::python::list& parameters) {
    vector<Expression> expressionParameters;
    for(int i = 0; i < boost::python::len(parameters); ++i) {
      expressionParameters.push_back(
        *boost::python::extract<VirtualExpression*>(parameters[i]));
    }
    return new FunctionExpression(std::move(name), dataType,
      std::move(expressionParameters));
  }
}

void Beam::Python::ExportExpression() {
  class_<VirtualExpression, noncopyable>("Expression", no_init)
    .add_property("data_type", make_function(&VirtualExpression::GetType,
      return_value_policy<copy_const_reference>()))
    .def("apply", &VirtualExpression::Apply);
  class_<Expression>("CloneableExpression", no_init)
    .def("__init__", &MakeCloneableExpression);
  ExportVector<vector<Expression>>("VectorExpression");
}

void Beam::Python::ExportConstantExpression() {
  class_<ConstantExpression, bases<VirtualExpression>>("ConstantExpression",
    init<const Value&>())
    .add_property("value", make_function(&ConstantExpression::GetValue,
      return_value_policy<copy_const_reference>()));
  implicitly_convertible<ConstantExpression, Expression>();
}

void Beam::Python::ExportDataType() {
  class_<VirtualDataType, noncopyable>("DataType", no_init)
    .def(self == self)
    .def(self != self);
  class_<DataType>("CloneableDataType", no_init)
    .def("__init__", &MakeCloneableDataType);
  ExportNativeDataType<BoolType>("BoolType");
  ExportNativeDataType<CharType>("CharType");
  ExportNativeDataType<IntType>("IntType");
  ExportNativeDataType<DecimalType>("DecimalType");
  ExportNativeDataType<IdType>("IdType");
  ExportNativeDataType<StringType>("StringType");
  ExportNativeDataType<DateTimeType>("DateTimeType");
  ExportNativeDataType<DurationType>("DurationType");
}

void Beam::Python::ExportFilteredQuery() {
  class_<FilteredQuery>("FilteredQuery", boost::python::init<>())
    .def(init<const Expression&>())
    .add_property("filter", make_function(&FilteredQuery::GetFilter,
      return_value_policy<copy_const_reference>()), &FilteredQuery::SetFilter);
}

void Beam::Python::ExportFunctionExpression() {
  class_<FunctionExpression, bases<VirtualExpression>>("FunctionExpression",
    init<string, const DataType&, vector<Expression>>())
    .def("__init__", make_constructor(&MakeFunctionExpression))
    .add_property("name", make_function(&FunctionExpression::GetName,
      return_value_policy<copy_const_reference>()))
    .add_property("parameters",
      make_function(&FunctionExpression::GetParameters,
      return_value_policy<copy_const_reference>()));
  implicitly_convertible<FunctionExpression, Expression>();
}

void Beam::Python::ExportInterruptableQuery() {
  class_<InterruptableQuery>("InterruptableQuery", boost::python::init<>())
    .def(init<InterruptionPolicy>())
    .add_property("interruption_policy",
      &InterruptableQuery::GetInterruptionPolicy,
      &InterruptableQuery::SetInterruptionPolicy);
}

void Beam::Python::ExportInterruptionPolicy() {
  enum_<InterruptionPolicy>("InterruptionPolicy")
    .value("BREAK_QUERY", InterruptionPolicy::BREAK_QUERY)
    .value("RECOVER_DATA", InterruptionPolicy::RECOVER_DATA)
    .value("IGNORE_CONTINUE", InterruptionPolicy::IGNORE_CONTINUE);
}

void Beam::Python::ExportMemberAccessExpression() {
  class_<MemberAccessExpression, bases<VirtualExpression>>(
    "MemberAccessExpression",
    init<string, const DataType&, const Expression&>())
    .add_property("name", make_function(&MemberAccessExpression::GetName,
      return_value_policy<copy_const_reference>()))
    .add_property("expression",
      make_function(&MemberAccessExpression::GetExpression,
      return_value_policy<copy_const_reference>()));
  implicitly_convertible<MemberAccessExpression, Expression>();
}

void Beam::Python::ExportParameterExpression() {
  class_<ParameterExpression, bases<VirtualExpression>>("ParameterExpression",
    init<int, const DataType&>())
    .add_property("index", &ParameterExpression::GetIndex);
  implicitly_convertible<ParameterExpression, Expression>();
}

void Beam::Python::ExportQueries() {
  string nestedName = extract<string>(scope().attr("__name__") + ".queries");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("queries") = nestedModule;
  scope parent = nestedModule;
  ExportExpression();
  ExportConstantExpression();
  ExportDataType();
  ExportInterruptableQuery();
  ExportInterruptionPolicy();
  ExportFilteredQuery();
  ExportFunctionExpression();
  ExportMemberAccessExpression();
  ExportParameterExpression();
  ExportRange();
  ExportRangedQuery();
  ExportSequence();
  ExportSnapshotLimit();
  ExportSnapshotLimitedQuery();
  ExportValue();
  ExportIndexedQuery<boost::python::object>("");
  ExportBasicQuery<boost::python::object>("");
  ExportException<ExpressionTranslationException, std::runtime_error>(
    "ExpressionTranslationException")
    .def(init<const string&>());
  ExportException<QueryInterruptedException, std::runtime_error>(
    "QueryInterruptedException")
    .def(init<>())
    .def(init<const string&>());
  ExportException<TypeCompatibilityException, std::runtime_error>(
    "TypeCompatibilityException")
    .def(init<>())
    .def(init<const string&>());
  def("build_current_query", &BuildCurrentQuery<boost::python::object>);
  def("build_real_time_query", &BuildRealTimeQuery<boost::python::object>);
}

void Beam::Python::ExportRange() {
  ExportVariant<Range::Point>();
  class_<Range>("Range", init<>())
    .def(init<const Range::Point&, const Range::Point&>())
    .add_static_property("EMPTY", &Range::Empty)
    .add_static_property("TOTAL", &Range::Total)
    .add_static_property("REAL_TIME", &Range::RealTime)
    .add_property("start", make_function(&Range::GetStart,
      return_value_policy<copy_const_reference>()))
    .add_property("end", make_function(&Range::GetEnd,
      return_value_policy<copy_const_reference>()))
    .def(self == self)
    .def(self != self);
}

void Beam::Python::ExportRangedQuery() {
  class_<RangedQuery>("RangedQuery", boost::python::init<>())
    .add_property("range", boost::python::make_function(&RangedQuery::GetRange,
      return_value_policy<copy_const_reference>()),
      static_cast<void (RangedQuery::*)(const Range&)>(&RangedQuery::SetRange))
    .def("set_range", static_cast<
      void (RangedQuery::*)(const Range::Point&, const Range::Point&)>(
      &RangedQuery::SetRange<const Range::Point&, const Range::Point&>));
}

void Beam::Python::ExportSequence() {
  class_<Queries::Sequence>("Sequence", boost::python::init<>())
    .def(init<Queries::Sequence::Ordinal>())
    .add_static_property("FIRST", &Queries::Sequence::First)
    .add_static_property("LAST", &Queries::Sequence::Last)
    .add_static_property("PRESENT", &Queries::Sequence::Present)
    .add_property("ordinal", &Queries::Sequence::GetOrdinal)
    .def(self < self)
    .def(self <= self)
    .def(self == self)
    .def(self != self)
    .def(self >= self)
    .def(self > self);
  def("increment", &Increment);
  def("decrement", &Decrement);
}

void Beam::Python::ExportSnapshotLimit() {
  scope outer =
    class_<SnapshotLimit>("SnapshotLimit", boost::python::init<>())
      .def(init<SnapshotLimit::Type, int>())
      .add_static_property("NONE", &SnapshotLimit::None)
      .add_static_property("UNLIMITED", &SnapshotLimit::Unlimited)
      .add_property("type", &SnapshotLimit::GetType)
      .add_property("size", &SnapshotLimit::GetSize)
      .def("__str__", &SnapshotLimitToString)
      .def(self == self)
      .def(self != self);
  enum_<SnapshotLimit::Type>("Type")
    .value("HEAD", SnapshotLimit::Type::HEAD)
    .value("TAIL", SnapshotLimit::Type::TAIL);
}

void Beam::Python::ExportSnapshotLimitedQuery() {
  class_<SnapshotLimitedQuery>("SnapshotLimitedQuery", boost::python::init<>())
    .def(init<SnapshotLimit>())
    .add_property("snapshot_limit",
      boost::python::make_function(&SnapshotLimitedQuery::GetSnapshotLimit,
      return_value_policy<copy_const_reference>()),
      static_cast<void (SnapshotLimitedQuery::*)(const SnapshotLimit&)>(
      &SnapshotLimitedQuery::SetSnapshotLimit))
    .def("set_snapshot_limit", static_cast<
      void (SnapshotLimitedQuery::*)(SnapshotLimit::Type, int)>(
      &SnapshotLimitedQuery::SetSnapshotLimit));
}

void Beam::Python::ExportValue() {
  class_<VirtualValue, noncopyable>("Value", no_init)
    .add_property("data_type", make_function(&VirtualValue::GetType,
      return_value_policy<copy_const_reference>()));
  class_<Value>("CloneableValue", no_init)
    .def("__init__", &MakeCloneableValue);
  ExportNativeValue<BoolValue>("BoolValue");
  ExportNativeValue<CharValue>("CharValue");
  ExportNativeValue<IntValue>("IntValue");
  ExportNativeValue<DecimalValue>("DecimalValue");
  ExportNativeValue<IdValue>("IdValue");
  ExportNativeValue<StringValue>("StringValue");
  ExportNativeValue<DateTimeValue>("DateTimeValue");
  ExportNativeValue<DurationValue>("DurationValue");
}
