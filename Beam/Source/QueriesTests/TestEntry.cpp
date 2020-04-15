#include "Beam/QueriesTests/TestEntry.hpp"
#include <boost/date_time/posix_time/posix_time_io.hpp>

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Queries::Tests;
using namespace boost;
using namespace boost::posix_time;

bool TestEntry::operator ==(const TestEntry& rhs) const {
  return m_value == rhs.m_value && m_timestamp == rhs.m_timestamp;
}

bool TestEntry::operator !=(const TestEntry& rhs) const {
  return !(*this == rhs);
}

std::ostream& Beam::Queries::Tests::operator <<(
    std::ostream& out, const TestEntry& entry) {
  return out << '(' << entry.m_value << ' ' << entry.m_timestamp << ')';
}
