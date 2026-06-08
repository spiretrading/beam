#include <doctest/doctest.h>
#include "Beam/Utilities/Tracer.hpp"

using namespace Beam;

namespace {
  struct Point {
    int m_x;
    int m_y;
  };

  std::ostream& operator <<(std::ostream& out, const Point& point) {
    return out << '(' << point.m_x << ", " << point.m_y << ')';
  }

  struct SinkGuard {
    std::ostringstream m_capture;

    SinkGuard() {
      Tracer::set_sink(m_capture);
    }

    ~SinkGuard() {
      Tracer::set_sink(std::cerr);
      Tracer::set_default_options(Tracer::Options());
    }

    std::vector<std::string> lines() const {
      auto result = std::vector<std::string>();
      auto stream = std::istringstream(m_capture.str());
      auto line = std::string();
      while(std::getline(stream, line)) {
        result.push_back(line);
      }
      return result;
    }

    SinkGuard(const SinkGuard&) = delete;
    SinkGuard& operator =(const SinkGuard&) = delete;
  };

  auto make_quiet_options() {
    auto options = Tracer::Options();
    options.m_has_wall_clock = false;
    options.m_has_routine_id = false;
    options.m_has_thread_id = false;
    options.m_has_scope_bounds = false;
    return options;
  }
}

TEST_SUITE("Tracer") {
  TEST_CASE("mark_outputs_id_scope_and_label") {
    auto guard = SinkGuard();
    {
      auto tracer = Tracer("example", make_quiet_options());
      tracer.mark("step");
    }
    auto lines = guard.lines();
    REQUIRE(lines.size() == 1);
    auto& line = lines.front();
    REQUIRE(line.starts_with("[id="));
    REQUIRE(line.find(" scope=example]") != std::string::npos);
    REQUIRE(line.ends_with("| step"));
  }

  TEST_CASE("scope_bounds_emit_enter_and_exit") {
    auto guard = SinkGuard();
    {
      auto options = make_quiet_options();
      options.m_has_scope_bounds = true;
      auto tracer = Tracer("example", options);
      tracer.mark("middle");
    }
    auto lines = guard.lines();
    REQUIRE(lines.size() == 3);
    REQUIRE(lines[0].ends_with("| > enter"));
    REQUIRE(lines[1].ends_with("| middle"));
    REQUIRE(lines[2].ends_with("| < exit"));
  }

  TEST_CASE("ids_are_sequential") {
    auto guard = SinkGuard();
    auto first = Tracer("first", make_quiet_options());
    auto second = Tracer("second", make_quiet_options());
    REQUIRE(second.get_id() == first.get_id() + 1);
  }

  TEST_CASE("streaming_combines_into_one_line") {
    auto guard = SinkGuard();
    {
      auto tracer = Tracer("example", make_quiet_options());
      tracer << "x=" << 42 << " y=" << 7;
    }
    auto first = guard.lines();
    REQUIRE(first.size() == 1);
    REQUIRE(first.front().ends_with("| x=42 y=7"));
    guard.m_capture.str("");
    {
      auto tracer = Tracer("example", make_quiet_options());
      tracer << Point(3, 4);
    }
    auto second = guard.lines();
    REQUIRE(second.size() == 1);
    REQUIRE(second.front().ends_with("| (3, 4)"));
  }

  TEST_CASE("options_control_emitted_fields") {
    auto guard = SinkGuard();
    {
      auto options = Tracer::Options();
      options.m_has_scope_bounds = false;
      options.m_has_thread_id = true;
      auto tracer = Tracer("example", options);
      tracer.mark("step");
    }
    auto verbose = guard.lines();
    REQUIRE(verbose.size() == 1);
    REQUIRE(verbose.front().find("ts=") != std::string::npos);
    REQUIRE(verbose.front().find("rtn=") != std::string::npos);
    REQUIRE(verbose.front().find("thr=") != std::string::npos);
    guard.m_capture.str("");
    {
      auto tracer = Tracer("example", make_quiet_options());
      tracer.mark("step");
    }
    auto bare = guard.lines();
    REQUIRE(bare.size() == 1);
    REQUIRE(bare.front().find("ts=") == std::string::npos);
    REQUIRE(bare.front().find("rtn=") == std::string::npos);
    REQUIRE(bare.front().find("thr=") == std::string::npos);
  }

  TEST_CASE("default_options_constructor") {
    auto guard = SinkGuard();
    {
      auto tracer = Tracer("example");
      tracer.mark("step");
    }
    auto lines = guard.lines();
    REQUIRE(lines.size() == 3);
    REQUIRE(lines[1].find("ts=") != std::string::npos);
    REQUIRE(lines[1].find("rtn=") != std::string::npos);
    REQUIRE(lines[1].find("thr=") == std::string::npos);
  }

  TEST_CASE("concurrent_lines_are_not_interleaved") {
    auto guard = SinkGuard();
    auto thread_count = 4;
    auto mark_count = 200;
    {
      auto threads = std::vector<std::thread>();
      for(auto i = 0; i < thread_count; ++i) {
        threads.emplace_back([mark_count] {
          auto tracer = Tracer("worker", make_quiet_options());
          for(auto j = 0; j < mark_count; ++j) {
            tracer.mark("tick");
          }
        });
      }
      for(auto& thread : threads) {
        thread.join();
      }
    }
    auto lines = guard.lines();
    REQUIRE(lines.size() ==
      static_cast<std::size_t>(thread_count * mark_count));
    for(auto& line : lines) {
      REQUIRE(line.starts_with("[id="));
      REQUIRE(line.ends_with("| tick"));
    }
  }
}
