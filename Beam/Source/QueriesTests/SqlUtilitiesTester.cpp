#include <doctest/doctest.h>
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/SqlUtilities.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace Viper;

namespace {
  struct SequenceSequenceConnection {
    template<typename T, typename D>
    void execute(const SelectStatement<T, D>& statement) {
      REQUIRE(false);
    }
  };

  struct DateSequenceConnection {
    int m_call_count = 0;

    template<typename T, typename D>
    void execute(const SelectStatement<T, D>& statement) {
      ++m_call_count;
      REQUIRE(m_call_count == 1);
      *statement.get_first() = 500;
    }
  };

  struct SequenceDateConnection {
    int m_call_count = 0;

    template<typename T, typename D>
    void execute(const SelectStatement<T, D>& statement) {
      ++m_call_count;
      REQUIRE(m_call_count == 1);
      *statement.get_first() = 600;
    }
  };

  struct DateDateConnection {
    int m_call_count = 0;

    template<typename T, typename D>
    void execute(const SelectStatement<T, D>& statement) {
      ++m_call_count;
      if(m_call_count == 1) {
        *statement.get_first() = 500;
        return;
      } else if(m_call_count == 2) {
        *statement.get_first() = 600;
        return;
      } else {
        REQUIRE(false);
      }
    }
  };
}

TEST_SUITE("SqlUtilities") {
  TEST_CASE("sanitize_sequence_sequence") {
    auto pool = DatabaseConnectionPool<SequenceSequenceConnection>(1, [] {
      return std::make_unique<SequenceSequenceConnection>();
    });
    auto query = BasicQuery<int>();
    query.set_range(Beam::Sequence(500), Beam::Sequence(600));
    auto sanitized_query = sanitize(query, "table", sym("id"), pool);
    auto start = get<Beam::Sequence>(sanitized_query.get_range().get_start());
    REQUIRE(start == Beam::Sequence(500));
    auto end = get<Beam::Sequence>(sanitized_query.get_range().get_end());
    REQUIRE(end == Beam::Sequence(600));
  }

  TEST_CASE("sanitize_date_sequence") {
    auto pool = DatabaseConnectionPool<DateSequenceConnection>(1, [] {
      return std::make_unique<DateSequenceConnection>();
    });
    auto query = BasicQuery<int>();
    query.set_range(
      time_from_string("2020-05-06 12:05:01:00"), Beam::Sequence(600));
    auto sanitized_query = sanitize(query, "table", sym("id"), pool);
    auto start = get<Beam::Sequence>(sanitized_query.get_range().get_start());
    REQUIRE(start == Beam::Sequence(500));
    auto end = get<Beam::Sequence>(sanitized_query.get_range().get_end());
    REQUIRE(end == Beam::Sequence(600));
  }

  TEST_CASE("sanitize_sequence_date") {
    auto pool = DatabaseConnectionPool<SequenceDateConnection>(1, [] {
      return std::make_unique<SequenceDateConnection>();
    });
    auto query = BasicQuery<int>();
    query.set_range(
      Beam::Sequence(200), time_from_string("2020-05-06 12:15:01:00"));
    auto sanitized_query = sanitize(query, "table", sym("id"), pool);
    auto start = get<Beam::Sequence>(sanitized_query.get_range().get_start());
    REQUIRE(start == Beam::Sequence(200));
    auto end = get<Beam::Sequence>(sanitized_query.get_range().get_end());
    REQUIRE(end == Beam::Sequence(600));
  }

  TEST_CASE("sanitize_date_date") {
    auto pool = DatabaseConnectionPool<DateDateConnection>(1, [] {
      return std::make_unique<DateDateConnection>();
    });
    auto query = BasicQuery<int>();
    query.set_range(time_from_string("2020-05-06 12:05:01:00"),
      time_from_string("2020-05-06 12:15:01:00"));
    auto sanitized_query = sanitize(query, "table", sym("id"), pool);
    auto start = get<Beam::Sequence>(sanitized_query.get_range().get_start());
    REQUIRE(start == Beam::Sequence(500));
    auto end = get<Beam::Sequence>(sanitized_query.get_range().get_end());
    REQUIRE(end == Beam::Sequence(600));
  }
}
