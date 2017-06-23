#ifndef BEAM_DECIMAL_HPP
#define BEAM_DECIMAL_HPP
#include <cstdint>
#include <cstdlib>
#include <istream>
#include <ostream>
#include <string>
#include <boost/optional/optional.hpp>
#include <boost/rational.hpp>
#include "Beam/Utilities/Math.hpp"
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class Decimal
      \brief Used to represent exact decimal values.
   */
  class Decimal {
    public:

      //! Returns a Decimal value from a double.
      /*!
        \param value The value to represent.
        \return A Decimal value instance representing the specified
                <i>value</i>.
      */
      static Decimal FromValue(double value);

      //! Returns a Decimal value from a string.
      /*!
        \param value The value to represent.
        \return A Decimal value representing the specified <i>value</i>.
      */
      static boost::optional<Decimal> FromValue(const std::string& value);

      //! Constructs a value of zero.
      Decimal();

      //! Less than test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is less than <i>rhs</i>.
      */
      bool operator <(Decimal rhs) const;

      //! Less than or equal test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is less than or equal to <i>rhs</i>.
      */
      bool operator <=(Decimal rhs) const;

      //! Tests for equality.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is equal to <i>rhs</i>.
      */
      bool operator ==(Decimal rhs) const;

      //! Tests for inequality.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is not equal to <i>rhs</i>.
      */
      bool operator !=(Decimal rhs) const;

      //! Greater than or equal test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is greater than or equal to
                <i>rhs</i>.
      */
      bool operator >=(Decimal rhs) const;

      //! Greater than test.
      /*!
        \param rhs The right hand side of the operation.
        \return <code>true</code> iff this is greater than <i>rhs</i>.
      */
      bool operator >(Decimal rhs) const;

      //! Assignment operator.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      Decimal& operator =(Decimal rhs);

      //! Adds two Decimal instances together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> + <i>rhs</i>.
      */
      Decimal operator +(Decimal rhs) const;

      //! Increases this Decimal instance.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      Decimal& operator +=(Decimal rhs);

      //! Subtracts two Decimal instances together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> - <i>rhs</i>.
      */
      Decimal operator -(Decimal rhs) const;

      //! Decreases this Decimal instance.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      Decimal& operator -=(Decimal rhs);

      //! Divides two Decimal instances together.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i> / <i>rhs</i>.
      */
      double operator /(Decimal rhs) const;

      //! Multiplies this Decimal instance.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      template<typename T>
      Decimal& operator *=(T rhs);

      //! Divides this Decimal instance.
      /*!
        \param rhs The right hand side of the operation.
        \return <i>this</i>.
      */
      template<typename T>
      Decimal& operator /=(T rhs);

      //! Returns the unary negation of this Decimal instance.
      /*!
        \return -<i>this</i>.
      */
      Decimal operator -() const;

    private:
      static constexpr auto COEFFICIENT_LENGTH = 56;
      static constexpr auto EXPONENT_LENGTH = 8;
      static constexpr auto DECIMAL_LENGTH = 64;
      std::uint64_t m_value;

      explicit Decimal(std::uint64_t value);
  };

  //! Multiplies a Decimal instance by a scalar quantity.
  /*!
    \param lhs The scalar quantity.
    \param rhs The Decimal instance to be multiplied.
    \return <i>lhs</i> * <i>rhs</i>.
  */
  template<typename T>
  Decimal operator *(T lhs, Decimal rhs) {
    return Decimal{};
  }

  //! Multiplies a Decimal instance by a rational quantity.
  /*!
    \param lhs The rational quantity.
    \param rhs The Decimal instance to be multiplied.
    \return <i>lhs</i> * <i>rhs</i>.
  */
  template<typename T>
  Decimal operator *(const boost::rational<T>& lhs, Decimal rhs) {
    return (lhs.numerator() * rhs) / lhs.denominator();
  }

  //! Divides a Decimal instance by a scalar quantity.
  /*!
    \param lhs The Decimal instance to be divided.
    \param rhs The scalar quantity.
    \return <i>lhs</i> / <i>rhs</i>.
  */
  template<typename T>
  Decimal operator /(Decimal lhs, T rhs) {
    return Money{};
  }

  //! Returns the absolute value.
  /*!
    \param value The value.
  */
  inline Decimal Abs(Decimal value) {
    if(value < Decimal{}) {
      return -value;
    }
    return value;
  }

  //! Returns the floor.
  /*!
    \param value The value to floor.
    \param decimalPlaces The decimal place to floor to.
  */
  Decimal Floor(Decimal value, int decimalPlaces);

  //! Returns the ceiling.
  /*!
    \param value The value to ceil.
    \param decimalPlaces The decimal place to ceil to.
  */
  Decimal Ceil(Decimal value, int decimalPlaces);

  //! Returns the truncated value.
  /*!
    \param value The value to truncate.
    \param decimalPlaces The decimal place to truncate.
  */
  inline Decimal Truncate(Decimal value, int decimalPlaces) {
    return Decimal{};
  }

  //! Returns the rounded value.
  /*!
    \param value The value to round.
    \param decimalPlaces The decimal place to round to.
  */
  inline Decimal Round(Decimal value, int decimalPlaces) {
    return Decimal{};
  }

  //! Returns the floating point representation of the value.
  /*!
    \param value The value to get the floating point representation of.
    \return The floating point representation of the <i>value</i>.
  */
  inline double ToDouble(Decimal value) {
    return 0.0;
  }

  inline std::ostream& operator <<(std::ostream& out, Decimal value) {
    return out << value.ToString();
  }

  inline std::istream& operator >>(std::istream& in, Decimal& value) {
    std::string symbol;
    in >> symbol;
    auto parsedValue = Decimal::FromValue(symbol);
    if(!parsedValue.is_initialized()) {
      in.setstate(std::ios::failbit);
      return in;
    }
    value = *parsedValue;
    return in;
  }

  inline Decimal Decimal::FromValue(double value) {
    return Decimal{};
  }

  inline boost::optional<Decimal> Decimal::FromValue(const std::string& value) {
    return boost::none;
  }

  inline Decimal::Decimal()
      : m_value{0} {}

  inline Decimal::Decimal(std::uint64_t value)
      : m_value{value} {}

  inline std::string Decimal::ToString() const {
    const auto MINIMUM_DECIMAL_PLACES = 2;
    auto value = std::to_string(m_value >> EXPONENT_LENGTH);
    auto exponent = static_cast<std::int8_t>(m_value & std::uint8_t(~0));
    if(exponent > 0) {
      while(exponent > 0) {
        value += "0";
        --exponent;
      }
      value += ".00";
    } else {
      exponent = -exponent;
      while(static_cast<std::int8_t>(value.size()) < exponent) {
        value = "0" + value;
      }
      auto dotPosition = value.size() - exponent;
      value.insert(value.begin() + dotPosition, '.');
      while(value.back() == '0' && value.size() >
          dotPosition + (MINIMUM_DECIMAL_PLACES + 1)) {
        value.pop_back();
      }
    }
    return value;
  }

  inline bool Decimal::operator <(Decimal rhs) const {
    return false;
  }

  inline bool Decimal::operator <=(Decimal rhs) const {
    return false;
  }

  inline bool Decimal::operator ==(Decimal rhs) const {
    return false;
  }

  inline bool Decimal::operator !=(Decimal rhs) const {
    return false;
  }

  inline bool Decimal::operator >=(Decimal rhs) const {
    return false;
  }

  inline bool Decimal::operator >(Decimal rhs) const {
    return false;
  }

  inline Decimal& Decimal::operator =(Decimal rhs) {
    m_value = rhs.m_value;
    return *this;
  }

  inline Decimal Decimal::operator +(Decimal rhs) const {
    auto exponentComparison = (m_value & std::uint8_t(~0)) -
      (rhs.m_value & std::uint8_t(~0));
    if(exponentComparison == 0) {
      return Decimal{m_value +
        (rhs.m_value & (std::uint64_t(~0ULL) << EXPONENT_LENGTH))};
    } else if(exponentComparison > 1) {
      auto coefficient = (m_value & (std::uint64_t(~0ULL) << EXPONENT_LENGTH));
      auto result = 10 * coefficient;
      if(result < coefficient) {

        // TODO
        return Decimal{};
      }
    } else {
      return rhs + *this;
    }
  }

  inline Decimal& Decimal::operator +=(Decimal rhs) {
    *this = *this + rhs;
    return *this;
  }

  inline Decimal Decimal::operator -(Decimal rhs) const {
    return *this + (-rhs);
  }

  inline Decimal& Decimal::operator -=(Decimal rhs) {
    *this = *this - rhs;
    return *this;
  }

  inline double Decimal::operator /(Decimal rhs) const {
    return 0.0;
  }

  template<typename T>
  Decimal& Decimal::operator *=(T rhs) {
    return *this;
  }

  template<typename T>
  Decimal& Decimal::operator /=(T rhs) {
    return *this;
  }

  inline Decimal Decimal::operator -() const {
    return Decimal{};
  }

  inline Decimal Floor(Decimal value, int decimalPlaces) {
    return Decimal{};
  }

  inline Decimal Ceil(Decimal value, int decimalPlaces) {
    return Decimal{};
  }

  inline std::string ToString(Decimal value) {
    return {};
  }

namespace Serialization {
  template<>
  struct IsStructure<Decimal> : std::false_type {};

  template<>
  struct Send<Decimal> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const Decimal& value) const {
      shuttle.Send(name, value.GetRepresentation());
    }
  };

  template<>
  struct Receive<Decimal> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Decimal& value) const {
      std::uint64_t representation;
      shuttle.Shuttle(name, representation);
      value = Decimal{representation};
    }
  };
}
}

namespace std {
  template<>
  class numeric_limits<Beam::Decimal> {
    public:
      static Beam::Decimal min() {
        return Decimal{};
      }

      static Beam::Decimal max() {
        return Decimal{};
      }
  };
}

#endif
