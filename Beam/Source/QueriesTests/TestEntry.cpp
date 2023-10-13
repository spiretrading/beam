#include "Beam/QueriesTests/TestEntry.hpp"
#include <boost/date_time/posix_time/posix_time_io.hpp>

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;

std::ostream& Beam::Queries::Tests::operator <<(
    std::ostream& out, const TestEntry& entry) {
  return out << '(' << entry.m_value << ' ' << entry.m_timestamp << ')';
}
