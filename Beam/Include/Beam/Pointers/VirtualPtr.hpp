#ifndef BEAM_VIRTUAL_PTR_HPP
#define BEAM_VIRTUAL_PTR_HPP
#include <array>
#include <cstring>
#include <memory>

namespace Beam {

  /**
   * A smart pointer used for storing type erased values.
   * @tparam T The type to store.
   */
  template<typename T>
  class VirtualPtr {
  public:

    /** The type to store. */
    using Type = T;

    /** Constructs an empty VirtualPtr. */
    VirtualPtr() noexcept;

    /**
     * Constructs an empty VirtualPtr from nullptr.
     * @param nullptr Null pointer constant.
     */
    VirtualPtr(std::nullptr_t) noexcept;

    /**
     * Converting move constructor.
     * Allows moving from VirtualPtr<Derived> to VirtualPtr<Base>.
     * @tparam U The derived type (must be convertible to T).
     * @param other The InlinePtr to move from.
     */
    template<typename U>
    VirtualPtr(VirtualPtr<U>&& other) noexcept;

    VirtualPtr(const VirtualPtr& other);
    VirtualPtr(VirtualPtr&& other) noexcept;
    ~VirtualPtr();

    /**
     * Checks whether the pointer is non-null.
     * @return true if the pointer manages an object, false otherwise
     */
    explicit operator bool() const noexcept;

    /**
     * Returns a pointer to the managed object.
     * @return Pointer to the managed object, or nullptr if empty.
     */
    Type* get() const noexcept;

    /** Releases ownership of the managed object and resets to empty state. */
    void reset() noexcept;

    /**
     * Dereferences the pointer to the managed object.
     * @return Reference to the managed object.
     */
    Type& operator *() const noexcept;

    /**
     * Returns a pointer to the managed object for member access.
     * @return Pointer to the managed object.
     */
    Type* operator ->() const noexcept;

    VirtualPtr& operator =(const VirtualPtr& other);
    VirtualPtr& operator =(VirtualPtr&& other) noexcept;

  private:
    template<typename>
    friend class VirtualPtr;
    template<typename U, typename... Args>
    friend VirtualPtr<U> make_virtual_ptr(Args&&... args);
    static constexpr auto BUFFER_SIZE = std::size_t(24);
    using Buffer = std::array<std::byte, BUFFER_SIZE>;
    enum class Ownership {
      SMALL,
      SHARED,
      VIEW
    };
    struct OwnerTag {};
    struct Storage {
      alignas(alignof(std::max_align_t)) Buffer m_buffer;
      void (*m_move)(Buffer& self, Buffer& source) = nullptr;
    };
    union {
      Storage m_storage;
      std::shared_ptr<Type> m_shared;
    };
    Type* m_ptr;

    template<typename... Args>
    VirtualPtr(OwnerTag, Args&&... args);
    Ownership get_ownership() const;
  };

  /**
   * Constructs an VirtualPtr managing a new object of type T.
   * @tparam T The type of object to create
   * @tparam Args The types of arguments to forward to T's constructor
   * @param args The arguments to forward to T's constructor
   * @return An VirtualPtr managing the newly constructed object
   */
  template<typename T, typename... Args>
  VirtualPtr<T> make_virtual_ptr(Args&&... args) {
    return VirtualPtr<T>(
      typename VirtualPtr<T>::OwnerTag(), std::forward<Args>(args)...);
  }

  template<typename T>
  VirtualPtr<T>::VirtualPtr() noexcept
    : m_ptr(nullptr) {}

  template<typename T>
  VirtualPtr<T>::VirtualPtr(std::nullptr_t) noexcept
    : VirtualPtr() {}

  template<typename T>
  template<typename U>
  VirtualPtr<T>::VirtualPtr(VirtualPtr<U>&& other) noexcept {
    if(!other) {
      m_ptr = nullptr;
      return;
    }
    if(other.get_ownership() == VirtualPtr<U>::Ownership::SMALL) {
      new (&m_storage) Storage();
      other.m_storage.m_move(m_storage.m_buffer, other.m_storage.m_buffer);
      m_storage.m_move = other.m_storage.m_move;
      m_ptr = reinterpret_cast<T*>(reinterpret_cast<std::byte*>(&m_storage) +
        (reinterpret_cast<const std::byte*>(other.m_ptr) -
          reinterpret_cast<const std::byte*>(&other.m_storage)));
    } else if(other.get_ownership() == VirtualPtr<U>::Ownership::VIEW) {
      new (&m_storage) Storage();
      m_ptr = other.m_ptr;
    } else {
      new (&m_shared) std::shared_ptr<T>(std::move(other.m_shared));
      m_ptr = m_shared.get();
    }
    other = nullptr;
  }

  template<typename T>
  VirtualPtr<T>::VirtualPtr(const VirtualPtr& other) {
    if(!other) {
      m_ptr = nullptr;
    } else if(other.get_ownership() == Ownership::VIEW ||
        other.get_ownership() == Ownership::SMALL) {
      new (&m_storage) Storage();
      m_ptr = other.m_ptr;
    } else {
      new (&m_shared) std::shared_ptr<Type>(other.m_shared);
      m_ptr = m_shared.get();
    }
  }

  template<typename T>
  VirtualPtr<T>::VirtualPtr(VirtualPtr&& other) noexcept {
    if(!other) {
      m_ptr = nullptr;
      return;
    }
    if(other.get_ownership() == Ownership::SMALL) {
      new (&m_storage) Storage();
      other.m_storage.m_move(m_storage.m_buffer, other.m_storage.m_buffer);
      m_storage.m_move = other.m_storage.m_move;
      m_ptr = reinterpret_cast<T*>(reinterpret_cast<std::byte*>(&m_storage) +
        (reinterpret_cast<const std::byte*>(other.m_ptr) -
          reinterpret_cast<const std::byte*>(&other.m_storage)));
    } else if(other.get_ownership() == Ownership::VIEW) {
      new (&m_storage) Storage();
      m_ptr = other.m_ptr;
    } else {
      new (&m_shared) std::shared_ptr<Type>(std::move(other.m_shared));
      m_ptr = m_shared.get();
    }
    other = nullptr;
  }

  template<typename T>
  VirtualPtr<T>::~VirtualPtr() {
    reset();
  }

  template<typename T>
  VirtualPtr<T>::operator bool() const noexcept {
    return m_ptr;
  }

  template<typename T>
  typename VirtualPtr<T>::Type* VirtualPtr<T>::get() const noexcept {
    return m_ptr;
  }

  template<typename T>
  void VirtualPtr<T>::reset() noexcept {
    if(!m_ptr) {
      return;
    }
    if(get_ownership() == Ownership::SMALL) {
      m_ptr->~Type();
    } else if(get_ownership() == Ownership::SHARED) {
      m_shared.~shared_ptr<Type>();
    }
    m_ptr = nullptr;
  }

  template<typename T>
  typename VirtualPtr<T>::Type& VirtualPtr<T>::operator *() const noexcept {
    return *m_ptr;
  }

  template<typename T>
  typename VirtualPtr<T>::Type* VirtualPtr<T>::operator ->() const noexcept {
    return m_ptr;
  }

  template<typename T>
  VirtualPtr<T>& VirtualPtr<T>::operator =(const VirtualPtr& other) {
    if(this == &other) {
      return *this;
    }
    reset();
    if(other) {
      if(other.get_ownership() == Ownership::VIEW ||
          other.get_ownership() == Ownership::SMALL) {
        new (&m_storage) Storage();
        m_ptr = other.m_ptr;
      } else {
        new (&m_shared) std::shared_ptr<Type>(other.m_shared);
        m_ptr = m_shared.get();
      }
    }
    return *this;
  }

  template<typename T>
  VirtualPtr<typename VirtualPtr<T>::Type>&
      VirtualPtr<T>::operator=(VirtualPtr&& other) noexcept {
    if(this == &other) {
      return *this;
    }
    reset();
    if(!other) {
      return *this;
    }
    if(other.get_ownership() == Ownership::SMALL) {
      new (&m_storage) Storage();
      other.m_storage.m_move(m_storage.m_buffer, other.m_storage.m_buffer);
      m_storage.m_move = other.m_storage.m_move;
      m_ptr = reinterpret_cast<Type*>(reinterpret_cast<std::byte*>(&m_storage) +
        (reinterpret_cast<const std::byte*>(other.m_ptr) -
          reinterpret_cast<const std::byte*>(&other.m_storage)));
    } else if(other.get_ownership() == Ownership::VIEW) {
      new (&m_storage) Storage();
      m_ptr = other.m_ptr;
    } else {
      new (&m_shared) std::shared_ptr<Type>(std::move(other.m_shared));
      m_ptr = m_shared.get();
    }
    other = nullptr;
    return *this;
  }

  template<typename T>
  template<typename... Args>
  VirtualPtr<T>::VirtualPtr(OwnerTag, Args&&... args) {
    constexpr auto IS_SMALL =
      sizeof(Type) <= BUFFER_SIZE && alignof(Type) <= alignof(std::max_align_t);
    if constexpr(IS_SMALL) {
      new (&m_storage) Storage();
      new (&m_storage.m_buffer) Type(std::forward<Args>(args)...);
      m_storage.m_move = [] (Buffer& self, Buffer& source) {
        new (&self) Type(std::move(*reinterpret_cast<Type*>(&source)));
      };
      m_ptr = reinterpret_cast<Type*>(&m_storage.m_buffer);
    } else {
      new (&m_shared) std::shared_ptr<Type>(
        std::make_shared<Type>(std::forward<Args>(args)...));
      m_ptr = m_shared.get();
    }
  }

  template<typename T>
  typename VirtualPtr<T>::Ownership VirtualPtr<T>::get_ownership() const {
    auto start = reinterpret_cast<const std::byte*>(&m_storage);
    auto end = start + BUFFER_SIZE;
    auto address = reinterpret_cast<const std::byte*>(m_ptr);
    static const auto Z = Storage();
    if(address >= start && address < end) {
      return Ownership::SMALL;
    } else if(std::memcmp(&m_storage, &Z, sizeof(Storage)) == 0) {
      return Ownership::VIEW;
    } else {
      return Ownership::SHARED;
    }
  }
}

#endif
