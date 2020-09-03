#ifndef BEAM_VIRTUAL_CHANNEL_IDENTIFIER_HPP
#define BEAM_VIRTUAL_CHANNEL_IDENTIFIER_HPP
#include <ostream>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/Streamable.hpp"

namespace Beam {
namespace IO {

  /** Provides a pure virtual interface to a ChannelIdentifier. */
  class VirtualChannelIdentifier : public Streamable {
    public:
      virtual ~VirtualChannelIdentifier() = default;

    protected:

      /** Constructs a VirtualChannelIdentifier. */
      VirtualChannelIdentifier() = default;

    private:
      VirtualChannelIdentifier(const VirtualChannelIdentifier&) = delete;
      VirtualChannelIdentifier& operator =(
        const VirtualChannelIdentifier&) = delete;
  };

  /**
   * Wraps a ChannelIdentifier providing it with a virtual interface.
   * @param <I> The type of ChannelIdentifier to wrap.
   */
  template<typename I>
  class WrapperChannelIdentifier : public VirtualChannelIdentifier {
    public:

      /** The ChannelIdentifier to wrap. */
      using ChannelIdentifier = GetTryDereferenceType<I>;

      /**
       * Constructs a WrapperChannelIdentifier.
       * @param identifier The ChannelIdentifier to wrap.
       */
      template<typename IF>
      WrapperChannelIdentifier(IF&& identifier);

      /** Returns the base identifier. */
      const ChannelIdentifier& GetBase() const;

    protected:
      std::ostream& ToStream(std::ostream& out) const override;

    private:
      GetOptionalLocalPtr<I> m_identifier;
  };

  /**
   * Wraps a ChannelIdentifier into a VirtualChannelIdentifier.
   * @param identifier The ChannelIdentifier to wrap.
   */
  template<typename ChannelIdentifier>
  std::unique_ptr<VirtualChannelIdentifier> MakeVirtualChannelIdentifier(
      ChannelIdentifier&& identifier) {
    return std::make_unique<WrapperChannelIdentifier<
      std::decay_t<ChannelIdentifier>>>(std::forward<ChannelIdentifier>(
      identifier));
  }

  template<typename I>
  template<typename IF>
  WrapperChannelIdentifier<I>::WrapperChannelIdentifier(IF&& identifier)
    : m_identifier(std::forward<IF>(identifier)) {}

  template<typename I>
  const typename WrapperChannelIdentifier<I>::ChannelIdentifier&
      WrapperChannelIdentifier<I>::GetBase() const {
    return *m_identifier;
  }

  template<typename I>
  std::ostream& WrapperChannelIdentifier<I>::ToStream(std::ostream& out) const {
    return out << *m_identifier;
  }
}

  template<>
  struct ImplementsConcept<IO::VirtualChannelIdentifier,
    IO::ChannelIdentifier> : std::true_type {};
}

#endif
