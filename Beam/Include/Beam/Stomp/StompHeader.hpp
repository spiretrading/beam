#ifndef BEAM_STOMPHEADER_HPP
#define BEAM_STOMPHEADER_HPP
#include <string>
#include <boost/lexical_cast.hpp>
#include "Beam/Stomp/Stomp.hpp"

namespace Beam {
namespace Stomp {

  /*! \class StompHeader
      \brief Stores a single header in a STOMP frame.
   */
  class StompHeader {
    public:

      //! Constructs a StompHeader.
      /*!
        \param header The header name.
        \param value The value.
      */
      StompHeader(std::string name, std::string value);

      //! Constructs a StompHeader.
      /*!
        \param header The header name.
        \param value The value.
      */
      template<typename T>
      StompHeader(std::string name, const T& value);

      //! Returns the header name.
      const std::string& GetName() const;

      //! Returns the value.
      const std::string& GetValue() const;

      //! Returns the value.
      template<typename T>
      T GetValue() const;

    private:
      std::string m_name;
      std::string m_value;
  };

  inline StompHeader::StompHeader(std::string name, std::string value)
      : m_name{std::move(name)},
        m_value{std::move(value)} {}

  template<typename T>
  StompHeader::StompHeader(std::string name, const T& value)
      : StompHeader{std::move(name), boost::lexical_cast<std::string>(value)} {}

  inline const std::string& StompHeader::GetName() const {
    return m_name;
  }

  inline const std::string& StompHeader::GetValue() const {
    return m_value;
  }

  template<typename T>
  T StompHeader::GetValue() const {
    return boost::lexical_cast<T>(GetValue());
  }
}
}

#endif
