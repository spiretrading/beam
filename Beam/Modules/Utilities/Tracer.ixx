module;
#include "Prelude.hpp"

export module Beam:Tracer;

export namespace Beam {

  /** A timestamped logging utility that allows tracing through a code path. */
  class Tracer {
    public:

      /** Configures the fields a Tracer emits on each line. */
      struct Options {

        /** Whether to emit a wall clock stamp in epoch microseconds. */
        bool m_has_wall_clock = true;

        /** Whether to emit the id of the routine that produced the line. */
        bool m_has_routine_id = true;

        /** Whether to emit a hash of the operating system thread. */
        bool m_has_thread_id = false;

        /** Whether to emit a line on entry to and exit from the scope. */
        bool m_has_scope_bounds = true;
      };

      /**
       * Accumulates one streamed line and emits it as a single atomic write.
       */
      class Event {
        public:
          Event(Event&& other);
          ~Event();

          /**
           * Appends a value to the pending line.
           * @param value The value to append using its operator <<.
           */
          Event& operator <<(const IsStreamable auto& value);

        private:
          friend class Tracer;
          Tracer* m_tracer;
          std::chrono::steady_clock::time_point m_timestamp;
          std::chrono::system_clock::time_point m_wall_clock;
          std::ostringstream m_body;
          bool m_is_active;

          explicit Event(Tracer& tracer);
          Event(const Event&) = delete;
          Event& operator =(const Event&) = delete;
      };

      /**
       * Sets the default options applied to Tracers.
       * @param options The options to apply by default.
       */
      static void set_default_options(const Options& options);

      /**
       * Specifies the stream used to output all logging.
       * @param stream The stream to write to.
       */
      static void set_sink(std::ostream& stream);

      /**
       * Constructs a Tracer using the default options.
       * @param scope An name used to identify a series of log entries.
       */
      explicit Tracer(std::string_view scope);

      /**
       * Constructs a Tracer.
       * @param scope An name used to identify a series of log entries.
       * @param options The fields to emit.
       */
      Tracer(std::string_view scope, Options options);

      ~Tracer();

      /** Returns this Tracer's unique id. */
      std::uint64_t get_id() const;

      /** Returns the time elapsed since this Tracer was constructed. */
      std::chrono::steady_clock::duration get_elapsed() const;

      /**
       * Emits one line with a preformatted label.
       * @param label The text of the line.
       */
      void mark(std::string_view label);

      /** Begins a streamed line. */
      Event line();

      /**
       * Begins a streamed line seeded with a value.
       * @param tracer The Tracer to emit from.
       * @param value The first value of the line.
       */
      friend Event operator <<(Tracer& tracer, const IsStreamable auto& value) {
        auto event = tracer.line();
        event << value;
        return event;
      }

    private:
      inline static std::mutex m_mutex;
      inline static std::ostream* m_stream = &std::cerr;
      inline static std::atomic_uint64_t m_next_id = 0;
      static Options m_default_options;
      std::uint64_t m_id;
      std::string_view m_scope;
      Options m_options;
      std::chrono::steady_clock::time_point m_start;
      std::chrono::steady_clock::time_point m_last;

      static Options get_default_options();
      Tracer(const Tracer&) = delete;
      Tracer& operator =(const Tracer&) = delete;
      void write(std::chrono::steady_clock::time_point timestamp,
        std::chrono::system_clock::time_point wall_clock,
        std::string_view body);
  };

  inline Tracer::Options Tracer::m_default_options;

  inline void Tracer::set_default_options(const Options& options) {
    auto lock = std::lock_guard(m_mutex);
    m_default_options = options;
  }

  inline void Tracer::set_sink(std::ostream& stream) {
    auto lock = std::lock_guard(m_mutex);
    m_stream = &stream;
  }

  inline Tracer::Options Tracer::get_default_options() {
    auto lock = std::lock_guard(m_mutex);
    return m_default_options;
  }

  inline Tracer::Tracer(std::string_view scope)
    : Tracer(scope, get_default_options()) {}

  inline Tracer::Tracer(std::string_view scope, Options options)
      : m_id(++m_next_id),
        m_scope(scope),
        m_options(options),
        m_start(std::chrono::steady_clock::now()),
        m_last(m_start) {
    if(m_options.m_has_scope_bounds) {
      write(m_start, std::chrono::system_clock::now(), "> enter");
    }
  }

  inline Tracer::~Tracer() {
    if(m_options.m_has_scope_bounds) {
      write(std::chrono::steady_clock::now(),
        std::chrono::system_clock::now(), "< exit");
    }
  }

  inline std::uint64_t Tracer::get_id() const {
    return m_id;
  }

  inline std::chrono::steady_clock::duration Tracer::get_elapsed() const {
    return std::chrono::steady_clock::now() - m_start;
  }

  inline void Tracer::mark(std::string_view label) {
    write(std::chrono::steady_clock::now(),
      std::chrono::system_clock::now(), label);
  }

  inline Tracer::Event Tracer::line() {
    return Event(*this);
  }

  inline void Tracer::write(std::chrono::steady_clock::time_point timestamp,
      std::chrono::system_clock::time_point wall_clock, std::string_view body) {
    auto to_microseconds = [] (auto duration) {
      return static_cast<long long>(std::chrono::duration_cast<
        std::chrono::microseconds>(duration).count());
    };
    auto total = to_microseconds(timestamp - m_start);
    auto delta = to_microseconds(timestamp - m_last);
    m_last = timestamp;
    auto buffer = std::array<char, 512>();
    auto position = std::size_t(0);
    auto append_text = [&] (std::string_view text) {
      auto count = std::min(text.size(), buffer.size() - position);
      std::memcpy(buffer.data() + position, text.data(), count);
      position += count;
    };
    auto append_number = [&] (auto value, auto base) {
      auto result = std::to_chars(buffer.data() + position,
        buffer.data() + buffer.size(), value, base);
      if(result.ec == std::errc()) {
        position = static_cast<std::size_t>(result.ptr - buffer.data());
      }
    };
    append_text("[id=");
    append_number(m_id, 10);
    if(m_options.m_has_wall_clock) {
      append_text(" ts=");
      append_number(to_microseconds(wall_clock.time_since_epoch()), 10);
    }
    if(m_options.m_has_routine_id) {
      append_text(" rtn=");
      append_number(get_current_routine().get_id(), 10);
    }
    if(m_options.m_has_thread_id) {
      append_text(" thr=");
      append_number(
        std::hash<std::thread::id>()(std::this_thread::get_id()), 16);
    }
    append_text(" scope=");
    append_text(m_scope);
    append_text("] t=");
    append_number(total, 10);
    append_text("us d=");
    append_number(delta, 10);
    append_text("us | ");
    append_text(body);
    append_text("\n");
    try {
      auto lock = std::lock_guard(m_mutex);
      m_stream->write(buffer.data(), static_cast<std::streamsize>(position));
      m_stream->flush();
    } catch(...) {}
  }

  inline Tracer::Event::Event(Tracer& tracer)
    : m_tracer(&tracer),
      m_timestamp(std::chrono::steady_clock::now()),
      m_wall_clock(std::chrono::system_clock::now()),
      m_is_active(true) {}

  inline Tracer::Event::Event(Event&& other)
      : m_tracer(other.m_tracer),
        m_timestamp(other.m_timestamp),
        m_wall_clock(other.m_wall_clock),
        m_body(std::move(other.m_body)),
        m_is_active(other.m_is_active) {
    other.m_is_active = false;
  }

  inline Tracer::Event::~Event() {
    if(m_is_active) {
      m_tracer->write(m_timestamp, m_wall_clock, m_body.str());
    }
  }

  Tracer::Event& Tracer::Event::operator <<(const IsStreamable auto& value) {
    if(m_is_active) {
      m_body << value;
    }
    return *this;
  }
}

