#ifndef BEAM_SERIALIZED_VALUE_HPP
#define BEAM_SERIALIZED_VALUE_HPP
#include <cassert>
#include <concepts>
#include <memory>
#include <utility>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {

    /**
     * Delays the allocation/initialization of a serializable value.
     * @tparam T The type of the value.
     */
    template<typename T>
    class SerializedValue {
    public:

        /** The type of the value. */
        using Type = T;

        /** Constructs an uninitialized SerializedValue. */
        SerializedValue() noexcept
            : m_is_initialized(false) {
        }

        ~SerializedValue() {
            reset();
        }

        /** Tests if this SerializedValue is initialized. */
        explicit operator bool() const {
            return is_initialized();
        }

        /** Returns a reference to the value. */
        Type& operator *() const {
            return get();
        }

        /** Returns a pointer to the value. */
        Type* operator ->() const {
            assert(m_is_initialized);
            return std::launder(reinterpret_cast<T*>(m_storage));
        }

        /** Returns <code>true</code> iff this is initialized. */
        bool is_initialized() const {
            return m_is_initialized;
        }

        /** Initializes the value. */
        void initialize() {
            reset();
            BEAM_SUPPRESS_POD_INITIALIZER()
                new(m_storage) T(DataShuttle::make<T>());
            BEAM_UNSUPPRESS_POD_INITIALIZER()
                m_is_initialized = true;
        }

        /** Returns the value. */
        Type& get() const {
            assert(m_is_initialized);
            return *std::launder(
                const_cast<T*>(reinterpret_cast<const T*>(m_storage)));
        }

        /** Resets the value. */
        void reset() {
            if (!m_is_initialized) {
                return;
            }
            get().~T();
            m_is_initialized = false;
        }

    private:
        friend struct DataShuttle;
        bool m_is_initialized;
        alignas(Type) unsigned char m_storage[sizeof(Type)];

        SerializedValue(const SerializedValue&) = delete;
        SerializedValue& operator =(const SerializedValue&) = delete;
    };

    template<typename T> requires DataShuttle::is_default_constructible<T>
    class SerializedValue<T> {
    public:
        using Type = T;

        SerializedValue()
            : m_value(DataShuttle::make<Type>()) {
        }

        explicit operator bool() const {
            return is_initialized();
        }

        Type& operator *() const {
            return get();
        }

        Type* operator ->() const {
            return &m_value;
        }

        bool is_initialized() const {
            return true;
        }

        void initialize() {}

        Type& get() const {
            return m_value;
        }

        void reset() {
            m_value.~T();
            std::construct_at(std::addressof(m_value));
        }

    private:
        mutable Type m_value;

        SerializedValue(const SerializedValue&) = delete;
        SerializedValue& operator =(const SerializedValue&) = delete;
    };
}

#endif
