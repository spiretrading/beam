#include "Beam/Python/Queries.hpp"
#include <sstream>
#include <Aspen/Conversions.hpp>
#include <Aspen/Python/Box.hpp>
#include <Aspen/Python/Reactor.hpp>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include "Beam/Python/Collections.hpp"
#include "Beam/Python/DateTime.hpp"
#include "Beam/Python/Queues.hpp"
#include "Beam/Python/Variant.hpp"
#include "Beam/Queries/AndExpression.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionQuery.hpp"
#include "Beam/Queries/ExpressionTranslationException.hpp"
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/FunctionExpression.hpp"
#include "Beam/Queries/InterruptableQuery.hpp"
#include "Beam/Queries/InterruptionPolicy.hpp"
#include "Beam/Queries/NotExpression.hpp"
#include "Beam/Queries/OrExpression.hpp"
#include "Beam/Queries/QueryInterruptedException.hpp"
#include "Beam/Queries/QueryReactor.hpp"
#include "Beam/Queries/Range.hpp"
#include "Beam/Queries/RangedQuery.hpp"
#include "Beam/Queries/Sequence.hpp"
#include "Beam/Queries/SnapshotLimit.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"
#include "Beam/Queries/StandardFunctionExpressions.hpp"
#include "Beam/Queries/StandardValues.hpp"
#include "Beam/Queries/TypeCompatibilityException.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace pybind11;

namespace {
  auto value = std::unique_ptr<class_<Value>>();

  struct FromPythonExpression : VirtualExpression {
    const VirtualExpression* m_expression;

    FromPythonExpression(const VirtualExpression& expression)
      : m_expression(&expression) {
    }

    std::type_index get_type() const override {
      return m_expression->get_type();
    }

    void apply(ExpressionVisitor& visitor) const override {
      m_expression->apply(visitor);
    }

    std::ostream& to_stream(std::ostream& out) const override {
      return (out << *m_expression);
    }
  };
}

class_<Value>& Beam::Python::get_exported_value() {
  return *value;
}

void Beam::Python::export_and_expression(pybind11::module& module) {
  class_<AndExpression, VirtualExpression>(module, "AndExpression").
    def(pybind11::init<Expression, Expression>()).
    def(pybind11::init<const AndExpression&>()).
    def_property_readonly("left", &AndExpression::get_left).
    def_property_readonly("right", &AndExpression::get_right);
}

void Beam::Python::export_constant_expression(pybind11::module& module) {
  class_<ConstantExpression, VirtualExpression>(module, "ConstantExpression").
    def(pybind11::init<const Value&>()).
    def(pybind11::init<const ConstantExpression&>()).
    def_property_readonly("value", &ConstantExpression::get_value);
}

void Beam::Python::export_expression(pybind11::module& module) {
  auto virtual_expression =
    class_<VirtualExpression>(module, "VirtualExpression").
      def_property_readonly("data_type", &VirtualExpression::get_type).
      def("apply", &VirtualExpression::apply).
      def("__lt__", [] (const Expression& left, const Expression& right) {
        return left < right;
      }).
      def("__le__", [] (const Expression& left, const Expression& right) {
        return left <= right;
      }).
      def("__eq__", [] (const Expression& left, const Expression& right) {
        return left == right;
      }).
      def("__ne__", [] (const Expression& left, const Expression& right) {
        return left != right;
      }).
      def("__ge__", [] (const Expression& left, const Expression& right) {
        return left >= right;
      }).
      def("__gt__", [] (const Expression& left, const Expression& right) {
        return left > right;
      }).
      def("__add__", [] (const Expression& left, const Expression& right) {
        return left + right;
      }).
      def("__sub__", [] (const Expression& left, const Expression& right) {
        return left - right;
      }).
      def("__mul__", [] (const Expression& left, const Expression& right) {
        return left * right;
      }).
      def("__truediv__", [] (const Expression& left, const Expression& right) {
        return left / right;
      });
  export_default_methods(virtual_expression);
  auto expression = class_<Expression>(module, "Expression").
    def(pybind11::init([] (const VirtualExpression& expression) {
      return Expression(FromPythonExpression(expression));
    }), keep_alive<1, 2>()).
    def_property_readonly("data_type", &Expression::get_type).
    def("apply", &Expression::apply);
  export_default_methods(expression);
  implicitly_convertible<VirtualExpression, Expression>();
}

void Beam::Python::export_expression_query(module& module) {
  auto outer = class_<ExpressionQuery>(module, "ExpressionQuery").
    def(pybind11::init<const Expression&>()).
    def_property("update_policy", &ExpressionQuery::get_update_policy,
      &ExpressionQuery::set_update_policy).
    def_property("expression", &ExpressionQuery::get_expression,
      &ExpressionQuery::set_expression);
  export_default_methods(outer);
  enum_<ExpressionQuery::UpdatePolicy>(outer, "UpdatePolicy").
    value("ALL", ExpressionQuery::UpdatePolicy::ALL).
    value("CHANGE", ExpressionQuery::UpdatePolicy::CHANGE);
}

void Beam::Python::export_filtered_query(pybind11::module& module) {
  auto query = class_<FilteredQuery>(module, "FilteredQuery").
    def(pybind11::init<const Expression&>()).
    def_property(
      "filter", &FilteredQuery::get_filter, &FilteredQuery::set_filter);
  export_default_methods(query);
}

void Beam::Python::export_function_expression(pybind11::module& module) {
  class_<FunctionExpression, VirtualExpression>(module, "FunctionExpression").
    def(
      pybind11::init<std::string, std::type_index, std::vector<Expression>>()).
    def_property_readonly("name", &FunctionExpression::get_name).
    def_property_readonly("parameters", &FunctionExpression::get_parameters);
  module.def("max", overload_cast<const Expression&, const Expression&>(&max));
  module.def("min", overload_cast<const Expression&, const Expression&>(&min));
}

void Beam::Python::export_indexed_value(pybind11::module& module) {
  class_<IndexedValue<object, object>>(module, "IndexedValue").
    def(pybind11::init()).
    def(pybind11::init<const object&, const object&>()).
    def(pybind11::init<const IndexedValue<object, object>&>()).
    def_property_readonly("value",
      overload_cast<>(&IndexedValue<object, object>::get_value, const_)).
    def_property_readonly("index",
      overload_cast<>(&IndexedValue<object, object>::get_index, const_)).
    def("__eq__",
      [] (IndexedValue<object, object>& self,
          const IndexedValue<object, object>& other) {
        return self.get_index().attr("__eq__")(
          other.get_index()).cast<bool>() &&
            self.get_value().attr("__eq__")(other.get_value()).cast<bool>();
      }).
    def("__ne__",
      [] (IndexedValue<object, object>& self,
          const IndexedValue<object, object>& other) {
        return self.get_index().attr("__ne__")(
          other.get_index()).cast<bool>() ||
            self.get_value().attr("__ne__")(other.get_value()).cast<bool>();
      }).
    def("__str__", &lexical_cast<std::string, IndexedValue<object, object>>);
}

void Beam::Python::export_interruptable_query(pybind11::module& module) {
  auto query = class_<InterruptableQuery>(module, "InterruptableQuery").
    def(pybind11::init<InterruptionPolicy>()).
    def_property("interruption_policy",
      &InterruptableQuery::get_interruption_policy,
      &InterruptableQuery::set_interruption_policy);
  export_default_methods(query);
}

void Beam::Python::export_interruption_policy(pybind11::module& module) {
  enum_<InterruptionPolicy::Type>(module, "InterruptionPolicy").
    value("BREAK_QUERY", InterruptionPolicy::BREAK_QUERY).
    value("RECOVER_DATA", InterruptionPolicy::RECOVER_DATA).
    value("IGNORE_CONTINUE", InterruptionPolicy::IGNORE_CONTINUE);
}

void Beam::Python::export_member_access_expression(pybind11::module& module) {
  class_<MemberAccessExpression, VirtualExpression>(
    module, "MemberAccessExpression").
    def(pybind11::init<std::string, std::type_index, const Expression&>()).
    def_property_readonly("name", &MemberAccessExpression::get_name).
    def_property_readonly(
      "expression", &MemberAccessExpression::get_expression);
}

void Beam::Python::export_not_expression(pybind11::module& module) {
  class_<NotExpression, VirtualExpression>(module, "NotExpression").
    def(pybind11::init<Expression>()).
    def(pybind11::init<const NotExpression&>()).
    def_property_readonly("operand", &NotExpression::get_operand);
}

void Beam::Python::export_or_expression(pybind11::module& module) {
  class_<OrExpression, VirtualExpression>(module, "OrExpression").
    def(pybind11::init<Expression, Expression>()).
    def(pybind11::init<const OrExpression&>()).
    def_property_readonly("left", &OrExpression::get_left).
    def_property_readonly("right", &OrExpression::get_right);
}

void Beam::Python::export_parameter_expression(pybind11::module& module) {
  class_<ParameterExpression, VirtualExpression>(module, "ParameterExpression").
    def(pybind11::init<int, std::type_index>()).
    def_property_readonly("index", &ParameterExpression::get_index);
}

void Beam::Python::export_queries(pybind11::module& module) {
  export_expression(module);
  export_constant_expression(module);
  export_interruptable_query(module);
  export_interruption_policy(module);
  export_expression_query(module);
  export_filtered_query(module);
  export_function_expression(module);
  export_indexed_query<object>(module, "IndexedQuery");
  export_indexed_value(module);
  export_member_access_expression(module);
  export_and_expression(module);
  export_not_expression(module);
  export_or_expression(module);
  export_parameter_expression(module);
  export_range(module);
  export_ranged_query(module);
  export_sequence(module);
  export_sequenced_value(module);
  export_snapshot_limit(module);
  export_snapshot_limited_query(module);
  export_value(module);
  export_basic_query<object>(module, "Query");
  export_paged_query<object, object>(module, "PagedQuery");
  export_queue_suite<QueryVariant>(module, "QueryVariant");
  export_queue_suite<bool>(module, "Bool");
  export_queue_suite<char>(module, "Char");
  export_queue_suite<int>(module, "Int");
  export_queue_suite<double>(module, "Double");
  export_queue_suite<std::uint64_t>(module, "UInt64");
  export_queue_suite<std::string>(module, "String");
  export_queue_suite<ptime>(module, "DateTime");
  export_queue_suite<time_duration>(module, "TimeDuration");
  module.def("make_current_query", &make_current_query<object>);
  module.def("make_latest_query", &make_real_time_query<object>);
  module.def("make_real_time_query", &make_real_time_query<object>);
  register_exception<ExpressionTranslationException>(
    module, "ExpressionTranslationException");
  register_exception<QueryInterruptedException>(
    module, "QueryInterruptedException");
  register_exception<TypeCompatibilityException>(
    module, "TypeCompatibilityException");
}

void Beam::Python::export_query_reactor(pybind11::module& module) {
  module.def("query", [] (std::function<
      void (const BasicQuery<object>&, std::shared_ptr<QueueWriter<object>>)>
        submission_function, SharedBox<BasicQuery<object>> query) {
      return shared_box(query_reactor<object>(
        std::move(submission_function), std::move(query)));
    });
}

void Beam::Python::export_range(pybind11::module& module) {
  auto range = class_<Range>(module, "Range").
    def(pybind11::init<const Range::Point&, const Range::Point&>()).
    def_readonly_static("EMPTY", &Range::EMPTY).
    def_readonly_static("HISTORICAL", &Range::HISTORICAL).
    def_readonly_static("TOTAL", &Range::TOTAL).
    def_readonly_static("REAL_TIME", &Range::REAL_TIME).
    def_property_readonly("start", &Range::get_start).
    def_property_readonly("end", &Range::get_end);
  export_default_methods(range);
}

void Beam::Python::export_ranged_query(pybind11::module& module) {
  auto query = class_<RangedQuery>(module, "RangedQuery").
    def_property("range", &RangedQuery::get_range,
      overload_cast<const Range&>(&RangedQuery::set_range)).
    def("set_range", overload_cast<const Range::Point&, const Range::Point&>(
      &RangedQuery::set_range));
  export_default_methods(query);
}

void Beam::Python::export_sequence(pybind11::module& module) {
  auto sequence = class_<Sequence>(module, "Sequence").
    def(pybind11::init<Sequence::Ordinal>()).
    def_readonly_static("FIRST", &Sequence::FIRST).
    def_readonly_static("LAST", &Sequence::LAST).
    def_readonly_static("PRESENT", &Sequence::PRESENT).
    def_property_readonly("ordinal", &Sequence::get_ordinal);
  export_default_methods(sequence);
  module.def("increment", &increment);
  module.def("decrement", &decrement);
}

void Beam::Python::export_sequenced_value(pybind11::module& module) {
  class_<SequencedValue<object>>(module, "SequencedValue").
    def(pybind11::init()).
    def(pybind11::init<object, Sequence>()).
    def(pybind11::init<const SequencedValue<object>&>()).
    def_property_readonly(
      "value", overload_cast<>(&SequencedValue<object>::get_value, const_)).
    def_property_readonly("sequence",
      overload_cast<>(&SequencedValue<object>::get_sequence, const_)).
    def("__eq__",
      [] (SequencedValue<object>& self, const SequencedValue<object>& other) {
        return self.get_sequence() == other.get_sequence() &&
          self.get_value().attr("__eq__")(other.get_value()).cast<bool>();
      }).
    def("__ne__",
      [] (SequencedValue<object>& self, const SequencedValue<object>& other) {
        return self.get_sequence() != other.get_sequence() ||
          self.get_value().attr("__ne__")(other.get_value()).cast<bool>();
      }).
    def("__str__", &lexical_cast<std::string, SequencedValue<object>>);
}

void Beam::Python::export_snapshot_limit(pybind11::module& module) {
  auto outer = class_<SnapshotLimit>(module, "SnapshotLimit").
    def(pybind11::init<SnapshotLimit::Type, int>()).
    def_readonly_static("NONE", &SnapshotLimit::NONE).
    def_readonly_static("UNLIMITED", &SnapshotLimit::UNLIMITED).
    def_property_readonly("type", &SnapshotLimit::get_type).
    def_property_readonly("size", &SnapshotLimit::get_size).
    def_static("from_head", &SnapshotLimit::from_head).
    def_static("from_tail", &SnapshotLimit::from_tail);
  export_default_methods(outer);
  enum_<SnapshotLimit::Type>(outer, "Type").
    value("HEAD", SnapshotLimit::Type::HEAD).
    value("TAIL", SnapshotLimit::Type::TAIL);
}

void Beam::Python::export_snapshot_limited_query(pybind11::module& module) {
  auto query = class_<SnapshotLimitedQuery>(module, "SnapshotLimitedQuery").
    def(pybind11::init<SnapshotLimit>()).
    def_property("snapshot_limit", &SnapshotLimitedQuery::get_snapshot_limit,
      overload_cast<const SnapshotLimit&>(
        &SnapshotLimitedQuery::set_snapshot_limit)).
    def("set_snapshot_limit", overload_cast<SnapshotLimit::Type, int>(
      &SnapshotLimitedQuery::set_snapshot_limit));
  export_default_methods(query);
}

void Beam::Python::export_value(pybind11::module& module) {
  auto virtual_value = class_<VirtualValue>(module, "VirtualValue").
    def_property_readonly("data_type", &VirtualValue::get_type);
  export_default_methods(virtual_value);
  value = std::make_unique<class_<Value>>(module, "Value");
  value->def_property_readonly("data_type", &Value::get_type);
  export_default_methods(*value);
  export_native_value<BoolValue>(module, "BoolValue");
  export_native_value<CharValue>(module, "CharValue");
  export_native_value<IntValue>(module, "IntValue");
  export_native_value<DecimalValue>(module, "DecimalValue");
  export_native_value<IdValue>(module, "IdValue");
  export_native_value<StringValue>(module, "StringValue");
  export_native_value<DateTimeValue>(module, "DateTimeValue");
  export_native_value<DurationValue>(module, "DurationValue");
}
