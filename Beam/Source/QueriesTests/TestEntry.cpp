#include "Beam/QueriesTests/TestEntry.hpp"
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include "Beam/Queries/MemberAccessEvaluatorNode.hpp"

using namespace Beam;
using namespace Beam::Tests;

std::ostream& Beam::Tests::operator <<(
    std::ostream& out, const TestEntry& entry) {
  return out << '(' << entry.m_value << ' ' << entry.m_timestamp << ')';
}

TestTranslator::TestTranslator()
  : EvaluatorTranslator<TestQueryTypes>(typeid(TestEntry)) {}

std::unique_ptr<EvaluatorTranslator<TestQueryTypes>>
    TestTranslator::make_translator(std::type_index type) const {
  return std::make_unique<TestTranslator>(type);
}

void TestTranslator::visit(const MemberAccessExpression& expression) {
  if(expression.get_expression().get_type() == typeid(TestEntry)) {
    if(expression.get_name() == "value" &&
        expression.get_type() == typeid(int)) {
      auto instance = translate_operand<TestEntry>(expression.get_expression());
      set_evaluator(std::make_unique<MemberAccessEvaluatorNode<TestEntry, int>>(
        std::move(instance), &TestEntry::m_value));
    }
  } else {
    EvaluatorTranslator<TestQueryTypes>::visit(expression);
  }
}
