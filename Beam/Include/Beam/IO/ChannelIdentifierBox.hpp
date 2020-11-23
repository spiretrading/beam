#ifndef BEAM_CHANNEL_IDENTIFIER_BOX_HPP
#define BEAM_CHANNEL_IDENTIFIER_BOX_HPP
#include <memory>
#include <type_traits>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/Streamable.hpp"

namespace Beam {
namespace IO {

  /** Provides a generic interface over an arbitrary ChannelIdentifier. */
  class ChannelIdentifierBox : public Streamable {
    public:

      /**
       * Constructs a ChannelIdentifierBox of a specified type using
       * emplacement.
       * @param <T> The type of identifier to emplace.
       * @param args The arguments to pass to the emplaced identifier.
       */
      template<typename T, typename... Args>
      explicit ChannelIdentifierBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ChannelIdentifierBox by copying an existing identifier.
       * @param identifier The identifier to copy.
       */
      template<typename ChannelIdentifier>
      explicit ChannelIdentifierBox(ChannelIdentifier identifier);

      explicit ChannelIdentifierBox(ChannelIdentifierBox* identifier);

      explicit ChannelIdentifierBox(
        const std::shared_ptr<ChannelIdentifierBox>& identifier);

      explicit ChannelIdentifierBox(
        const std::unique_ptr<ChannelIdentifierBox>& identifier);

    protected:
      std::ostream& ToStream(std::ostream& out) const override;

    private:
      struct VirtualChannelIdentifier {
        virtual ~VirtualChannelIdentifier() = default;
        virtual std::ostream& ToStream(std::ostream& out) = 0;
      };
      template<typename I>
      struct WrappedChannelIdentifier final : VirtualChannelIdentifier {
        using ChannelIdentifier = I;
        GetOptionalLocalPtr<ChannelIdentifier> m_identifier;

        template<typename... Args>
        WrappedChannelIdentifier(Args&&... args);
        std::ostream& ToStream(std::ostream& out) override;
      };
      std::shared_ptr<VirtualChannelIdentifier> m_identifier;
  };

  template<typename T, typename... Args>
  ChannelIdentifierBox::ChannelIdentifierBox(std::in_place_type_t<T>,
    Args&&... args)
    : m_identifier(std::make_shared<WrappedChannelIdentifier<T>>(
        std::forward<Args>(args)...)) {}

  template<typename ChannelIdentifier>
  ChannelIdentifierBox::ChannelIdentifierBox(ChannelIdentifier identifier)
    : ChannelIdentifierBox(std::in_place_type<ChannelIdentifier>,
        std::move(identifier)) {}

  inline ChannelIdentifierBox::ChannelIdentifierBox(
    ChannelIdentifierBox* identifier)
    : ChannelIdentifierBox(*identifier) {}

  inline ChannelIdentifierBox::ChannelIdentifierBox(
    const std::shared_ptr<ChannelIdentifierBox>& identifier)
    : ChannelIdentifierBox(*identifier) {}

  inline ChannelIdentifierBox::ChannelIdentifierBox(
    const std::unique_ptr<ChannelIdentifierBox>& identifier)
    : ChannelIdentifierBox(*identifier) {}

  inline std::ostream& ChannelIdentifierBox::ToStream(std::ostream& out) const {
    return m_identifier->ToStream(out);
  }

  template<typename I>
  template<typename... Args>
  ChannelIdentifierBox::WrappedChannelIdentifier<I>::WrappedChannelIdentifier(
    Args&&... args)
    : m_identifier(std::forward<Args>(args)...) {}

  template<typename I>
  std::ostream& ChannelIdentifierBox::WrappedChannelIdentifier<I>::ToStream(
      std::ostream& out) {
    return out << *m_identifier;
  }
}

  template<>
  struct ImplementsConcept<IO::ChannelIdentifierBox,
    IO::ChannelIdentifier> : std::true_type {};
}

#endif
