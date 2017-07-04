#ifndef BEAM_VIRTUALCHANNELIDENTIFIER_HPP
#define BEAM_VIRTUALCHANNELIDENTIFIER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/IO.hpp"
#include "Beam/IO/Channel.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /*! \class VirtualChannelIdentifier
      \brief Provides a pure virtual interface to a ChannelIdentifier.
   */
  class VirtualChannelIdentifier : private boost::noncopyable {
    public:
      virtual ~VirtualChannelIdentifier() = default;

      virtual std::string ToString() const = 0;

    protected:

      //! Constructs a VirtualChannelIdentifier.
      VirtualChannelIdentifier() = default;
  };

  /*! \class WrapperChannelIdentifier
      \brief Wraps a ChannelIdentifier providing it with a virtual interface.
      \tparam ChannelIdentifierType The type of ChannelIdentifier to wrap.
   */
  template<typename ChannelIdentifierType>
  class WrapperChannelIdentifier : public VirtualChannelIdentifier {
    public:

      //! The ChannelIdentifier to wrap.
      using ChannelIdentifier = GetTryDereferenceType<ChannelIdentifierType>;

      //! Constructs a WrapperChannelIdentifier.
      /*!
        \param identifier The ChannelIdentifier to wrap.
      */
      template<typename ChannelIdentifierForward>
      WrapperChannelIdentifier(ChannelIdentifierForward&& identifier);

      virtual ~WrapperChannelIdentifier() override = default;

      //! Returns the ChannelIdentifier being wrapped.
      const ChannelIdentifier& GetIdentifier() const;

      //! Returns the ChannelIdentifier being wrapped.
      ChannelIdentifier& GetIdentifier();

      virtual std::string ToString() const override;

    private:
      GetOptionalLocalPtr<ChannelIdentifierType> m_identifier;
  };

  //! Wraps a ChannelIdentifier into a VirtualChannelIdentifier.
  /*!
    \param identifier The ChannelIdentifier to wrap.
  */
  template<typename ChannelIdentifier>
  std::unique_ptr<VirtualChannelIdentifier> MakeVirtualChannelIdentifier(
      ChannelIdentifier&& identifier) {
    return std::make_unique<WrapperChannelIdentifier<
      std::decay_t<ChannelIdentifier>>>(std::forward<ChannelIdentifier>(
      identifier));
  }

  template<typename ChannelIdentifierType>
  template<typename ChannelIdentifierForward>
  WrapperChannelIdentifier<ChannelIdentifierType>::WrapperChannelIdentifier(
      ChannelIdentifierForward&& identifier)
      : m_identifier{std::forward<ChannelIdentifierForward>(identifier)} {}

  template<typename ChannelIdentifierType>
  const typename WrapperChannelIdentifier<
      ChannelIdentifierType>::ChannelIdentifier& WrapperChannelIdentifier<
      ChannelIdentifierType>::GetIdentifier() const {
    return *m_identifier;
  }

  template<typename ChannelIdentifierType>
  typename WrapperChannelIdentifier<ChannelIdentifierType>::ChannelIdentifier&
      WrapperChannelIdentifier<ChannelIdentifierType>::GetIdentifier() {
    return *m_identifier;
  }

  template<typename ChannelIdentifierType>
  std::string WrapperChannelIdentifier<ChannelIdentifierType>::
      ToString() const {
    return m_identifier->ToString();
  }
}

  template<>
  struct ImplementsConcept<IO::VirtualChannelIdentifier,
    IO::ChannelIdentifier> : std::true_type {};
}

#endif
