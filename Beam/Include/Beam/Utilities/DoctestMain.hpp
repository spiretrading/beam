#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <print>
#include <string_view>
#include <system_error>
#include <thread>
#ifdef _WIN32
  #include <io.h>
#else
  #include <unistd.h>
#endif
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

namespace Beam::Details {
  class DoctestStreamBuffer : public std::streambuf {
    public:
      explicit DoctestStreamBuffer(std::FILE* stream)
          : m_stream(stream),
            m_file(nullptr) {
#ifdef _WIN32
        auto descriptor = _dup(_fileno(stream));
#else
        auto descriptor = dup(fileno(stream));
#endif
        if(descriptor == -1) {
          throw std::system_error(errno, std::generic_category(),
            "Unable to preserve test output");
        }
#ifdef _WIN32
        m_file = _fdopen(descriptor, "w");
#else
        m_file = fdopen(descriptor, "w");
#endif
        if(!m_file) {
          auto error = errno;
#ifdef _WIN32
          _close(descriptor);
#else
          ::close(descriptor);
#endif
          throw std::system_error(error, std::generic_category(),
            "Unable to open test output");
        }
      }

      ~DoctestStreamBuffer() {
        redirect(m_file);
        std::fclose(m_file);
      }

      std::FILE* get_file() const {
        return m_file;
      }

      bool redirect(std::FILE* file) {
        std::fflush(m_stream);
#ifdef _WIN32
        return _dup2(_fileno(file), _fileno(m_stream)) != -1;
#else
        return dup2(fileno(file), fileno(m_stream)) != -1;
#endif
      }

    private:
      std::FILE* m_stream;
      std::FILE* m_file;

      DoctestStreamBuffer(const DoctestStreamBuffer&) = delete;
      DoctestStreamBuffer& operator =(const DoctestStreamBuffer&) = delete;

      std::streamsize xsputn(const char* data, std::streamsize size) override {
        return static_cast<std::streamsize>(
          std::fwrite(data, 1, static_cast<std::size_t>(size), m_file));
      }

      int_type overflow(int_type value) override {
        if(traits_type::eq_int_type(value, traits_type::eof())) {
          return traits_type::not_eof(value);
        }
        if(std::fputc(traits_type::to_char_type(value), m_file) == EOF) {
          return traits_type::eof();
        }
        return value;
      }

      int sync() override {
        return std::fflush(m_file);
      }
  };

  inline int run_doctest(int argc, char** argv, int timeout) {
    auto output = DoctestStreamBuffer(stdout);
    auto errors = DoctestStreamBuffer(stderr);
    auto stream = std::ostream(&output);
    auto context = doctest::Context(argc, argv);
    context.setAsDefaultForAssertsOutOfTestCases();
    auto& options = *doctest::getContextOptions();
    if(!options.quiet && options.out.size() == 0) {
      context.setCout(&stream);
    }
    auto has_log_output = false;
    auto has_timeout = true;
    for(auto i = 1; i < argc; ++i) {
      if(std::string_view(argv[i]) == "--beam-log-output") {
        has_log_output = true;
      } else if(std::string_view(argv[i]) == "--notimeout") {
        has_timeout = false;
      }
    }
    if(!has_log_output) {
      std::cout.flush();
      std::cerr.flush();
      std::clog.flush();
#ifdef _WIN32
      auto file = std::fopen("NUL", "w");
#else
      auto file = std::fopen("/dev/null", "w");
#endif
      if(!file) {
        std::println(output.get_file(), "Unable to suppress test output.");
        return 1;
      }
      auto is_redirected = output.redirect(file) && errors.redirect(file);
      std::fclose(file);
      if(!is_redirected) {
        std::println(output.get_file(), "Unable to suppress test output.");
        return 1;
      }
    }
    auto code = 0;
    auto is_done = false;
    auto is_done_mutex = std::mutex();
    auto is_done_condition = std::condition_variable();
    auto lock = std::unique_lock(is_done_mutex);
    auto thread = std::thread([&] {
      auto result = context.run();
      auto lock = std::unique_lock(is_done_mutex);
      code = result;
      is_done = true;
      is_done_condition.notify_one();
    });
    if(!has_timeout) {
      is_done_condition.wait(lock, [&] { return is_done; });
    } else if(!is_done_condition.wait_for(
        lock, std::chrono::seconds(timeout), [&] { return is_done; })) {
      std::println(output.get_file(), "Status: TIMEOUT ({})", argv[0]);
      std::fflush(output.get_file());
      std::quick_exit(1);
    }
    thread.join();
    stream.flush();
    std::cout.flush();
    std::cerr.flush();
    std::clog.flush();
    return code;
  }
}

#define DOCTEST_MAIN_TIMEOUT(timeout)                                          \
  int main(int argc, char** argv) {                                            \
    return Beam::Details::run_doctest(argc, argv, timeout);                    \
  }

#define DOCTEST_MAIN() DOCTEST_MAIN_TIMEOUT(600)
