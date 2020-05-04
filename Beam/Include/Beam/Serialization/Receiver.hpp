#ifndef BEAM_RECEIVER_HPP
#define BEAM_RECEIVER_HPP
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam {
namespace Serialization {
namespace Details {
  BEAM_DEFINE_HAS_METHOD(ReceiverHasReceiveMethod, Receive, void, const char*,
    unsigned int version);
  BEAM_DEFINE_HAS_TYPEDEF(ReceiverHasSourceType, Source);
}

  /*! \class Receiver
      \brief Specifies a DataShuttle that's responsible for receiving data.
      \tparam SourceType A type of Buffer used to store the data.
   */
  template<typename SourceType>
  struct Receiver : Concept<DataShuttle>, Concept<Receiver<SourceType>> {

    //! The type of Buffer used to store the data.
    using Source = SourceType;

    //! Specifies where the received data is stored.
    /*!
      \param source Where the received data is stored.
    */
    void SetSource(Ref<Source> source);

    private:
      template<typename T, typename Enabled> friend struct Receive;

      template<bool HasMethod, typename Dummy>
      struct ResolveDirectMethods {
        template<typename Shuttler, typename T>
        void operator ()(Shuttler& shuttle, T& value,
          unsigned int version) const;
      };

      template<typename Dummy>
      struct ResolveDirectMethods<false, Dummy> {
        template<typename Shuttler, typename T>
        void operator ()(Shuttler& shuttle, T& value,
          unsigned int version) const;
      };
  };

  /*! \class IsReceiver
      \brief Checks if a specified type is a Receiver.
      \tparam T The type to check.
   */
  template<typename T, typename Enabled>
  struct IsReceiver : std::false_type {};

  /*! \class Receive
      \brief Contains operations for receive a type.
      \tparam T The type being specialized.
   */
  template<typename T, typename Enabled = void>
  struct Receive {

    //! Receives a value.
    /*!
      \tparam Shuttler The type of Sender to use.
      \param shuttle The Receiver to use.
      \param value The value to receive.
      \param version The class version being serialized.
    */
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, T& value, unsigned int version) const;

    //! Receives a value.
    /*!
      \tparam Shuttler The type of Sender to use.
      \param shuttle The Receiver to use.
      \param name The name of the value to receive.
      \param value The value to receive.
    */
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name, T& value) const;
  };

  template<typename T>
  struct IsReceiver<T, typename std::enable_if<
    Details::ReceiverHasSourceType<T>::value>::type> : boost::mpl::if_c<
    ImplementsConcept<T, Receiver<typename T::Source>>::value, std::true_type,
    std::false_type>::type {};

  template<typename SourceType>
  template<bool HasMethod, typename Dummy>
  template<typename Shuttler, typename T>
  void Receiver<SourceType>::ResolveDirectMethods<HasMethod, Dummy>::
      operator ()(Shuttler& shuttle, T& value, unsigned int version) const {
    DataShuttle::Receive(shuttle, value, version);
  }

  template<typename SourceType>
  template<typename Dummy>
  template<typename Shuttler, typename T>
  void Receiver<SourceType>::ResolveDirectMethods<false, Dummy>::operator ()(
      Shuttler& shuttle, T& value, unsigned int version) const {
    Shuttle<T>()(shuttle, value, version);
  }

  template<typename T, typename Enabled>
  template<typename Shuttler>
  void Receive<T, Enabled>::operator ()(Shuttler& shuttle, T& value,
      unsigned int version) const {
    typename Receiver<typename Shuttler::Source>::template ResolveDirectMethods<
      Details::ReceiverHasReceiveMethod<T, Shuttler>::value, void>()(shuttle,
      value, version);
  }

  template<typename T, typename Enabled>
  template<typename Shuttler>
  void Receive<T, Enabled>::operator ()(Shuttler& shuttle, const char* name,
      T& value) const {
    DataShuttle::Receive(shuttle, name, value);
  }
}

  template<typename T>
  struct ImplementsConcept<T, typename std::enable_if<
    Serialization::IsReceiver<T>::value, Serialization::DataShuttle>::type>
    : std::true_type {};
}

#endif
