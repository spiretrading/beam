#ifndef BEAM_CONVERTER_HPP
#define BEAM_CONVERTER_HPP
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \struct StaticCastConverter
      \brief Wraps a static_cast into a function object.
   */
  template<typename Destination>
  struct StaticCastConverter {
    Destination& operator ()(Destination& value) const {
      return value;
    }

    const Destination& operator ()(const Destination& value) const {
      return value;
    }

    template<typename T>
    Destination operator ()(T&& value) const {
      return Destination(std::forward<T>(value));
    }
  };

  template<typename Source, typename Destination, typename Enabled = void>
  struct Converter {};

  //! Converts a value from one representation to another.
  /*!
    \param source The source value to convert.
    \return The conversion.
  */
  template<typename Destination, typename Source>
  Destination Convert(const Source& source) {
    return Converter<Source, Destination>()(source);
  }

  template<>
  inline std::string Convert(const bool& source) {
    if(source) {
      return "true";
    } else {
      return "false";
    }
  }

  template<>
  inline std::string Convert(const short& source) {
    char buffer[16];
    std::sprintf(buffer, "%hd", source);
    return buffer;
  }

  template<>
  inline short Convert(const std::string& source) {
    short value;
    std::sscanf(source.c_str(), "%hd", &value);
    return value;
  }

  template<>
  inline std::string Convert(const unsigned short& source) {
    char buffer[16];
    std::sprintf(buffer, "%hu", source);
    return buffer;
  }

  template<>
  inline std::string Convert(const unsigned char& source) {
    std::string value;
    value += source;
    return value;
  }

  template<>
  inline std::string Convert(const signed char& source) {
    std::string value;
    value += source;
    return value;
  }

  template<>
  inline std::string Convert(const char& source) {
    std::string value;
    value += source;
    return value;
  }

  template<>
  inline unsigned short Convert(const std::string& source) {
    unsigned short value;
    std::sscanf(source.c_str(), "%hu", &value);
    return value;
  }

  template<>
  inline std::string Convert(const int& source) {
    char buffer[16];
    std::sprintf(buffer, "%d", source);
    return buffer;
  }

  template<>
  inline std::string Convert(const unsigned int& source) {
    char buffer[16];
    std::sprintf(buffer, "%u", source);
    return buffer;
  }

  template<>
  inline std::string Convert(const long long int& source) {
    char buffer[16];
    std::sprintf(buffer, "%lld", source);
    return buffer;
  }

  template<>
  inline std::string Convert(const unsigned long long int& source) {
    char buffer[16];
    std::sprintf(buffer, "%llu", source);
    return buffer;
  }

  template<>
  inline std::string Convert(const long unsigned int& source) {
    char buffer[16];
    std::sprintf(buffer, "%lu", source);
    return buffer;
  }

  template<>
  inline std::string Convert(const long int& source) {
    char buffer[16];
    std::sprintf(buffer, "%ld", source);
    return buffer;
  }

  template<>
  inline std::string Convert(const float& source) {
    char buffer[32];
    std::sprintf(buffer, "%f", source);
    return buffer;
  }

  template<>
  inline std::string Convert(const double& source) {
    char buffer[32];
    std::sprintf(buffer, "%f", source);
    return buffer;
  }

  template<>
  inline bool Convert(const char* const& source) {
    return source[0] == 't' || source[0] == '1' || source[0] == 'T';
  }

  template<>
  inline bool Convert(char* const& source) {
    return source[0] == 't' || source[0] == '1' || source[0] == 'T';
  }

  template<>
  inline int Convert(const char* const& source) {
    return std::atoi(source);
  }

  template<>
  inline std::int64_t Convert(const char* const& source) {
#ifdef _MSC_VER
    return ::_strtoi64(source, nullptr, 10);
#else
    return std::strtoll(source, nullptr, 10);
#endif
  }

  template<>
  inline std::uint64_t Convert(const char* const& source) {
#ifdef _MSC_VER
    return ::_strtoui64(source, nullptr, 10);
#else
    return std::strtoull(source, nullptr, 10);
#endif
  }

  template<>
  inline int Convert(char* const& source) {
    return std::atoi(source);
  }

  template<>
  inline std::int64_t Convert(char* const& source) {
#ifdef _MSC_VER
    return ::_strtoi64(source, nullptr, 10);
#else
    return std::strtoll(source, nullptr, 10);
#endif
  }

  template<>
  inline std::uint64_t Convert(char* const& source) {
#ifdef _MSC_VER
    return ::_strtoui64(source, nullptr, 10);
#else
    return std::strtoull(source, nullptr, 10);
#endif
  }

  template<typename VectorSource, typename VectorAllocator>
  struct Converter<std::vector<VectorSource, VectorAllocator>, std::string> {
    std::string operator ()(
        const std::vector<VectorSource, VectorAllocator>& source) const {
      std::string conversion = "[";
      for(auto i = source.begin(); i != source.end(); ++i) {
        conversion += Convert<std::string>(*i);
        if(i != source.end() - 1) {
          conversion += ", ";
        }
      }
      conversion += "]";
      return conversion;
    }
  };

  template<typename VectorSource, typename VectorAllocator>
  struct Converter<std::string, std::vector<VectorSource, VectorAllocator>> {
    std::vector<VectorSource, VectorAllocator> operator ()(
        const std::string& source) const {
      std::vector<VectorSource, VectorAllocator> conversion;
      std::string::size_type i = 0;
      while(true) {
        ++i;
        std::string::size_type start = i;
        while(source[i] != ',' && source[i] != ']') {
          ++i;
        }
        std::string item(source.begin() + start, source.begin() + i);
        conversion.emplace_back(Convert<VectorSource>(item));
        if(source[i] == ']') {
          break;
        } else {
          ++i;
        }
      };
      return conversion;
    }
  };
}

#endif
