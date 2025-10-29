#ifndef BEAM_CHANNEL_IDENTIFIER_HPP
#define BEAM_CHANNEL_IDENTIFIER_HPP
#include <memory>
#include <utility>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/Streamable.hpp"

namespace Beam {

  /** Satisfied by types that implement the ChannelIdentifier interface. */
  template<typename T>
  concept IsChannelIdentifier = std::copy_constructible<T> &&
    requires(const T& t) {
      { std::declval<std::ostream&>() << t } -> std::same_as<std::ostream&>;
    };

  /** Provides a generic interface over an arbitrary ChannelIdentifier. */
  class ChannelIdentifier : public Streamable {
    public:

      /**
       * Constructs a ChannelIdentifier of a specified type using emplacement.
       * @tparam T The type of identifier to emplace.
       * @param args The arguments to pass to the emplaced identifier.
       */
      template<typename T, typename... Args>
      explicit ChannelIdentifier(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ChannelIdentifier by copying an existing identifier.
       * @param identifier The identifier to copy.
       */
      template<DisableCopy<ChannelIdentifier> I> requires
        IsChannelIdentifier<I>
      explicit ChannelIdentifier(I identifier);
      ChannelIdentifier(const ChannelIdentifier& identifier);

    protected:
      std::ostream& to_stream(std::ostream& out) const override;

    private:
      struct VirtualChannelIdentifier {
        virtual ~VirtualChannelIdentifier() = default;

        virtual std::unique_ptr<VirtualChannelIdentifier> clone() const = 0;
        virtual std::ostream& to_stream(std::ostream& out) const = 0;
      };
      template<typename I>
      struct WrappedChannelIdentifier final : VirtualChannelIdentifier {
        using ChannelIdentifier = I;
        local_ptr_t<ChannelIdentifier> m_identifier;

        template<typename... Args>
        WrappedChannelIdentifier(Args&&... args);

        std::unique_ptr<VirtualChannelIdentifier> clone() const override;
        std::ostream& to_stream(std::ostream& out) const override;
      };
      std::unique_ptr<VirtualChannelIdentifier> m_identifier;
  };

  template<typename T, typename... Args>
  ChannelIdentifier::ChannelIdentifier(
      std::in_place_type_t<T>, Args&&... args)
    : m_identifier(std::make_unique<WrappedChannelIdentifier<T>>(
        std::forward<Args>(args)...)) {}

  template<DisableCopy<ChannelIdentifier> I> requires IsChannelIdentifier<I>
  ChannelIdentifier::ChannelIdentifier(I identifier)
    : m_identifier(std::make_unique<WrappedChannelIdentifier<I>>(
        std::move(identifier))) {}

  inline ChannelIdentifier::ChannelIdentifier(
    const ChannelIdentifier& identifier)
    : m_identifier(identifier.m_identifier->clone()) {}

  inline std::ostream& ChannelIdentifier::to_stream(std::ostream& out) const {
    return m_identifier->to_stream(out);
  }

  template<typename I>
  template<typename... Args>
  ChannelIdentifier::WrappedChannelIdentifier<I>::WrappedChannelIdentifier(
    Args&&... args)
    : m_identifier(std::forward<Args>(args)...) {}

  template<typename I>
  std::unique_ptr<typename ChannelIdentifier::VirtualChannelIdentifier>
      ChannelIdentifier::WrappedChannelIdentifier<I>::clone() const {
    return std::make_unique<WrappedChannelIdentifier<I>>(m_identifier);
  }

  template<typename I>
  std::ostream& ChannelIdentifier::WrappedChannelIdentifier<I>::to_stream(
      std::ostream& out) const {
    return out << *m_identifier;
  }
}

#endif
