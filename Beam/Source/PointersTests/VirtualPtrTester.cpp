#include <array>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"

using namespace Beam;

namespace {
  struct SmallObject {
    int m_value;
  };

  struct LargeObject {
    std::array<std::byte, 100> m_data;
    int m_value;

    LargeObject()
      : m_value(0) {}

    LargeObject(int value)
      : m_value(value) {}
  };

  template<typename T>
  concept IsAnimal = requires(const T& animal) {
    { animal.speak() } -> std::convertible_to<std::string>;
    { animal.feed() } -> std::convertible_to<std::string>;
  };

  class Animal {
    public:
      template<IsAnimal T, typename... Args>
      explicit Animal(std::in_place_type_t<T>, Args&&... args)
        : m_animal(
            make_virtual_ptr<WrappedAnimal<T>>(std::forward<Args>(args)...)) {}

      template<DisableCopy<Animal> T> requires IsAnimal<dereference_t<T>>
      Animal(T&& animal)
        : m_animal(make_virtual_ptr<WrappedAnimal<std::remove_cvref_t<T>>>(
            std::forward<T>(animal))) {}

      Animal(const Animal& other) = default;

      std::string speak() const {
        return m_animal->speak();
      }

      std::string feed() const {
        return m_animal->feed();
      }

    private:
      struct VirtualAnimal {
        virtual ~VirtualAnimal() = default;
        virtual std::string speak() const = 0;
        virtual std::string feed() const = 0;
      };
      template<typename T>
      struct WrappedAnimal : VirtualAnimal {
        using Type = T;
        local_ptr_t<T> m_animal;

        template<typename... Args>
        WrappedAnimal(Args&&... args)
          : m_animal(std::forward<Args>(args)...) {}

        std::string speak() const override {
          return m_animal->speak();
        }

        std::string feed() const override {
          return m_animal->feed();
        }
      };
      VirtualPtr<VirtualAnimal> m_animal;
  };

  struct Dog {
    std::string speak() const {
      return "Woof!";
    }

    std::string feed() const {
      return "Kibble";
    }
  };

  struct Cow {
    LargeObject m_data;

    std::string speak() const {
      return "Mooo!";
    }

    std::string feed() const {
      return "Grass";
    }
  };
}

TEST_SUITE("VirtualPtr") {
  TEST_CASE("default_constructor") {
    auto ptr = VirtualPtr<int>();
    REQUIRE(!ptr);
    REQUIRE(!ptr.get());
  }

  TEST_CASE("nullptr_constructor") {
    auto ptr = VirtualPtr<int>(nullptr);
    REQUIRE(!ptr);
    REQUIRE(!ptr.get());
  }

  TEST_CASE("make_virtual_ptr_small_object") {
    auto ptr = make_virtual_ptr<SmallObject>(42);
    REQUIRE(ptr);
    REQUIRE(ptr.get());
    REQUIRE(ptr->m_value == 42);
    REQUIRE((*ptr).m_value == 42);
  }

  TEST_CASE("make_virtual_ptr_large_object") {
    auto ptr = make_virtual_ptr<LargeObject>(99);
    REQUIRE(ptr);
    REQUIRE(ptr.get());
    REQUIRE(ptr->m_value == 99);
    REQUIRE((*ptr).m_value == 99);
  }

  TEST_CASE("make_virtual_ptr_default_construction") {
    auto ptr = make_virtual_ptr<SmallObject>();
    REQUIRE(ptr);
    REQUIRE(ptr->m_value == 0);
  }

  TEST_CASE("copy_constructor_empty") {
    auto ptr1 = VirtualPtr<int>();
    auto ptr2 = ptr1;
    REQUIRE(!ptr1);
    REQUIRE(!ptr2);
  }

  TEST_CASE("copy_constructor_small_object") {
    auto ptr1 = make_virtual_ptr<SmallObject>(42);
    auto ptr2 = ptr1;
    REQUIRE(ptr1);
    REQUIRE(ptr2);
    REQUIRE(ptr1->m_value == 42);
    REQUIRE(ptr2->m_value == 42);
    REQUIRE(ptr1.get() == ptr2.get());
  }

  TEST_CASE("copy_constructor_large_object") {
    auto ptr1 = make_virtual_ptr<LargeObject>(99);
    auto ptr2 = ptr1;
    REQUIRE(ptr1);
    REQUIRE(ptr2);
    REQUIRE(ptr1->m_value == 99);
    REQUIRE(ptr2->m_value == 99);
  }

  TEST_CASE("move_constructor_empty") {
    auto ptr1 = VirtualPtr<int>();
    auto ptr2 = std::move(ptr1);
    REQUIRE(!ptr2);
  }

  TEST_CASE("move_constructor_small_object") {
    auto ptr1 = make_virtual_ptr<SmallObject>(42);
    auto ptr2 = std::move(ptr1);
    REQUIRE(!ptr1);
    REQUIRE(ptr2);
    REQUIRE(ptr2->m_value == 42);
  }

  TEST_CASE("move_constructor_large_object") {
    auto ptr1 = make_virtual_ptr<LargeObject>(99);
    auto ptr2 = std::move(ptr1);
    REQUIRE(!ptr1);
    REQUIRE(ptr2);
    REQUIRE(ptr2->m_value == 99);
  }

  TEST_CASE("copy_assignment_empty_to_empty") {
    auto ptr1 = VirtualPtr<int>();
    auto ptr2 = VirtualPtr<int>();
    ptr2 = ptr1;
    REQUIRE(!ptr1);
    REQUIRE(!ptr2);
  }

  TEST_CASE("copy_assignment_value_to_empty") {
    auto ptr1 = make_virtual_ptr<SmallObject>(42);
    auto ptr2 = VirtualPtr<SmallObject>();
    ptr2 = ptr1;
    REQUIRE(ptr1);
    REQUIRE(ptr2);
    REQUIRE(ptr1->m_value == 42);
    REQUIRE(ptr2->m_value == 42);
  }

  TEST_CASE("copy_assignment_empty_to_value") {
    auto ptr1 = VirtualPtr<SmallObject>();
    auto ptr2 = make_virtual_ptr<SmallObject>(42);
    ptr2 = ptr1;
    REQUIRE(!ptr1);
    REQUIRE(!ptr2);
  }

  TEST_CASE("copy_assignment_value_to_value") {
    auto ptr1 = make_virtual_ptr<SmallObject>(42);
    auto ptr2 = make_virtual_ptr<SmallObject>(99);
    ptr2 = ptr1;
    REQUIRE(ptr1);
    REQUIRE(ptr2);
    REQUIRE(ptr1->m_value == 42);
    REQUIRE(ptr2->m_value == 42);
  }

  TEST_CASE("copy_assignment_self") {
    auto ptr = make_virtual_ptr<SmallObject>(42);
    ptr = ptr;
    REQUIRE(ptr);
    REQUIRE(ptr->m_value == 42);
  }

  TEST_CASE("move_assignment_empty_to_empty") {
    auto ptr1 = VirtualPtr<int>();
    auto ptr2 = VirtualPtr<int>();
    ptr2 = std::move(ptr1);
    REQUIRE(!ptr2);
  }

  TEST_CASE("move_assignment_value_to_empty") {
    auto ptr1 = make_virtual_ptr<SmallObject>(42);
    auto ptr2 = VirtualPtr<SmallObject>();
    ptr2 = std::move(ptr1);
    REQUIRE(!ptr1);
    REQUIRE(ptr2);
    REQUIRE(ptr2->m_value == 42);
  }

  TEST_CASE("move_assignment_empty_to_value") {
    auto ptr1 = VirtualPtr<SmallObject>();
    auto ptr2 = make_virtual_ptr<SmallObject>(42);
    ptr2 = std::move(ptr1);
    REQUIRE(!ptr1);
    REQUIRE(!ptr2);
  }

  TEST_CASE("move_assignment_value_to_value") {
    auto ptr1 = make_virtual_ptr<SmallObject>(42);
    auto ptr2 = make_virtual_ptr<SmallObject>(99);
    ptr2 = std::move(ptr1);
    REQUIRE(!ptr1);
    REQUIRE(ptr2);
    REQUIRE(ptr2->m_value == 42);
  }

  TEST_CASE("move_assignment_self") {
    auto ptr = make_virtual_ptr<SmallObject>(42);
    ptr = std::move(ptr);
    REQUIRE(ptr);
    REQUIRE(ptr->m_value == 42);
  }

  TEST_CASE("reset_empty") {
    auto ptr = VirtualPtr<int>();
    ptr.reset();
    REQUIRE(!ptr);
  }

  TEST_CASE("reset_small_object") {
    auto ptr = make_virtual_ptr<SmallObject>(42);
    REQUIRE(ptr);
    ptr.reset();
    REQUIRE(!ptr);
    REQUIRE(!ptr.get());
  }

  TEST_CASE("reset_large_object") {
    auto ptr = make_virtual_ptr<LargeObject>(99);
    REQUIRE(ptr);
    ptr.reset();
    REQUIRE(!ptr);
    REQUIRE(!ptr.get());
  }

  TEST_CASE("bool_operator_empty") {
    auto ptr = VirtualPtr<int>();
    REQUIRE(!static_cast<bool>(ptr));
  }

  TEST_CASE("bool_operator_non_empty") {
    auto ptr = make_virtual_ptr<SmallObject>(42);
    REQUIRE(static_cast<bool>(ptr));
  }

  TEST_CASE("get_empty") {
    auto ptr = VirtualPtr<int>();
    REQUIRE(!ptr.get());
  }

  TEST_CASE("get_non_empty") {
    auto ptr = make_virtual_ptr<SmallObject>(42);
    REQUIRE(ptr.get());
    REQUIRE(ptr.get()->m_value == 42);
  }

  TEST_CASE("dereference_operator") {
    auto ptr = make_virtual_ptr<SmallObject>(42);
    auto& obj = *ptr;
    REQUIRE(obj.m_value == 42);
  }

  TEST_CASE("arrow_operator") {
    auto ptr = make_virtual_ptr<SmallObject>(42);
    REQUIRE(ptr->m_value == 42);
  }

  TEST_CASE("destructor_small_object") {
    {
      auto ptr = make_virtual_ptr<SmallObject>(42);
      REQUIRE(ptr);
    }
  }

  TEST_CASE("destructor_large_object") {
    {
      auto ptr = make_virtual_ptr<LargeObject>(99);
      REQUIRE(ptr);
    }
  }

  TEST_CASE("multiple_copies") {
    auto ptr1 = make_virtual_ptr<SmallObject>(42);
    auto ptr2 = ptr1;
    auto ptr3 = ptr2;
    auto ptr4 = ptr3;
    REQUIRE(ptr1->m_value == 42);
    REQUIRE(ptr2->m_value == 42);
    REQUIRE(ptr3->m_value == 42);
    REQUIRE(ptr4->m_value == 42);
  }

  TEST_CASE("reassignment_chain") {
    auto ptr1 = make_virtual_ptr<SmallObject>(1);
    auto ptr2 = make_virtual_ptr<SmallObject>(2);
    auto ptr3 = make_virtual_ptr<SmallObject>(3);
    ptr3 = ptr2 = ptr1;
    REQUIRE(ptr1->m_value == 1);
    REQUIRE(ptr2->m_value == 1);
    REQUIRE(ptr3->m_value == 1);
  }

  TEST_CASE("small_type_erased") {
    auto dog = Animal(std::in_place_type<Dog>);
    REQUIRE(dog.speak() == "Woof!");
    REQUIRE(dog.feed() == "Kibble");
    auto unique_dog = Animal(std::make_unique<Dog>());
    REQUIRE(unique_dog.speak() == "Woof!");
    REQUIRE(unique_dog.feed() == "Kibble");
    auto shared_dog = Animal(std::make_shared<Dog>());
    REQUIRE(shared_dog.speak() == "Woof!");
    REQUIRE(shared_dog.feed() == "Kibble");
    auto value_dog = Animal(Dog());
    REQUIRE(value_dog.speak() == "Woof!");
    REQUIRE(value_dog.feed() == "Kibble");
    auto base_dog = Dog();
    auto ref_dog = Animal(base_dog);
    REQUIRE(ref_dog.speak() == "Woof!");
    REQUIRE(ref_dog.feed() == "Kibble");
  }

  TEST_CASE("large_type_erased") {
    auto cow = Animal(std::in_place_type<Cow>);
    REQUIRE(cow.speak() == "Mooo!");
    REQUIRE(cow.feed() == "Grass");
    auto unique_cow = Animal(std::make_unique<Cow>());
    REQUIRE(unique_cow.speak() == "Mooo!");
    REQUIRE(unique_cow.feed() == "Grass");
    auto shared_cow = Animal(std::make_shared<Cow>());
    REQUIRE(shared_cow.speak() == "Mooo!");
    REQUIRE(shared_cow.feed() == "Grass");
    auto value_cow = Animal(Cow());
    REQUIRE(value_cow.speak() == "Mooo!");
    REQUIRE(value_cow.feed() == "Grass");
    auto base_cow = Cow();
    auto ref_cow = Animal(base_cow);
    REQUIRE(ref_cow.speak() == "Mooo!");
    REQUIRE(ref_cow.feed() == "Grass");
  }
}
