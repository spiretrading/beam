#include <memory>
#include <doctest/doctest.h>
#include "Beam/Utilities/Instantiate.hpp"

using namespace Beam;

namespace {
  struct MetaClass1D {
    using type = boost::mp11::mp_list<int, double, char>;

    template<typename T>
    bool operator ()(const T& a) const {
      return a < 5;
    }
  };

  struct MetaClassMoveOnly1D {
    using type = boost::mp11::mp_list<int, double, char>;

    template<typename T>
    bool operator ()(std::unique_ptr<T> a) const {
      return true;
    }
  };

  struct MetaClass2D {
    using type = boost::mp11::mp_list<
      boost::mp11::mp_list<int, double>,
      boost::mp11::mp_list<char, float>,
      boost::mp11::mp_list<int, int>>;

    template<typename T1, typename T2>
    bool operator ()(const T1& a, const T2& b) const {
      return a < b;
    }
  };

  struct MetaClassMoveOnly2D {
    using type = boost::mp11::mp_list<
      boost::mp11::mp_list<int, double>>;

    template<typename T1, typename T2>
    bool operator ()(std::unique_ptr<T1> a, std::unique_ptr<T2> b) const {
      return true;
    }
  };
}

TEST_SUITE("Instantiate") {
  TEST_CASE("1d") {
    REQUIRE(instantiate<MetaClass1D>(typeid(int))(3));
    REQUIRE(instantiate<MetaClass1D>(typeid(double))(3.14));
    REQUIRE(!instantiate<MetaClass1D>(typeid(char))('a'));
  }

  TEST_CASE("1d_move_only") {
    instantiate<MetaClassMoveOnly1D>(typeid(int))(std::make_unique<int>(3));
    instantiate<MetaClassMoveOnly1D>(typeid(double))(
      std::make_unique<double>(3.14));
    instantiate<MetaClassMoveOnly1D>(typeid(char))(std::make_unique<char>('a'));
  }

  TEST_CASE("2d") {
    REQUIRE(instantiate<MetaClass2D>(typeid(int), typeid(double))(3, 4.4));
  }

  TEST_CASE("2d_move_only") {
    REQUIRE(instantiate<MetaClassMoveOnly2D>(typeid(int), typeid(double))(
      std::make_unique<int>(3), std::make_unique<double>(4.4)));
  }
}
