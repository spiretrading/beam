#ifndef BEAM_DATASHUTTLE_HPP
#define BEAM_DATASHUTTLE_HPP
#include <type_traits>
#include <boost/mpl/if.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Serialization/SerializationException.hpp"
#include "Beam/Serialization/Serialization.hpp"
#include "Beam/Serialization/SerializedValue.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/Concept.hpp"

namespace Beam {
namespace Serialization {
  template<typename T>
  T DefaultConstruct() {
    return {};
  }

  struct ReceiveBuilder {
    template<typename T>
    explicit operator T() const {
      return DefaultConstruct<T>();
    }
  };

  template<typename T, typename Enabled = void>
  struct IsDefaultConstructable : std::true_type {};

  /*! \class DataShuttle
      \brief Concept for moving data back and forth.
   */
  struct DataShuttle : Concept<DataShuttle> {

    //! Shuttles an unnamed value.
    /*!
      \param value The value to shuttle.
    */
    template<typename T>
    void Shuttle(T& value);

    //! Shuttles a generic type.
    /*!
      \param name The name of the value.
      \param value The value to shuttle.
    */
    template<typename T>
    void Shuttle(const char* name, T& value);

    //! Marks the start of a structured datum.
    /*!
      \param name The name of the structured datum.
    */
    void StartStructure(const char* name);

    //! Marks the end of a structured datum.
    void EndStructure();

    //! Marks the start of a sequence with a predetermined size.
    /*!
      \param name The name of the sequence.
      \param size The number of elements in the sequence.
    */
    void StartSequence(const char* name, int& size);

    //! Marks the start of a sequence with a predetermined size.
    /*!
      \param name The name of the sequence.
      \param size The number of elements in the sequence.
    */
    void StartSequence(const char* name, const int& size);

    //! Marks the start of a sequence.
    /*!
      \param name The name of the sequence.
    */
    void StartSequence(const char* name);

    //! Marks the end of a sequence.
    void EndSequence();

    private:
      template<typename SinkType> friend struct Sender;
      template<typename SourceType> friend struct Receiver;
      template<typename SenderType> friend class TypeRegistry;
      template<typename T> friend class SerializedValue;
      template<typename T, typename Enabled> friend struct Send;
      template<typename T, typename Enabled> friend struct Shuttle;
      template<typename T, typename Enabled> friend struct Receive;
      template<typename T, typename Enabled = void>
      struct BuilderHelper {
        T* operator ()() const {
          return new T();
        }
      };
      template<typename T>
      struct BuilderHelper<T, typename std::enable_if<
          !IsDefaultConstructable<T>::value>::type> {
        T* operator ()() const {
          return new T(ReceiveBuilder{});
        }
      };
      template<typename T, typename Enabled = void>
      struct SerializedValueBuilder {
        void operator ()(SerializedValue<T>& value) const {
          BEAM_SUPPRESS_POD_INITIALIZER()
          value.m_ptr = new(&value.m_storage) T();
          BEAM_UNSUPPRESS_POD_INITIALIZER()
        }
      };
      template<typename T>
      struct SerializedValueBuilder<T, typename std::enable_if<
          !IsDefaultConstructable<T>::value>::type> {
        void operator ()(SerializedValue<T>& value) const {
          BEAM_SUPPRESS_POD_INITIALIZER()
          value.m_ptr = new(&value.m_storage) T(ReceiveBuilder{});
          BEAM_UNSUPPRESS_POD_INITIALIZER()
        }
      };

      template<typename T>
      static T* Builder();
      template<typename T>
      static void Builder(SerializedValue<T>& ptr);
      template<typename Shuttler, typename T>
      static void Send(Shuttler& shuttle, const T& value, unsigned int version);
      template<typename Shuttler, typename T>
      static void Send(Shuttler& shuttle, const char* name, const T& value);
      template<typename Shuttler, typename T>
      static void Receive(Shuttler& shuttle, T& value, unsigned int version);
      template<typename Shuttler, typename T>
      static void Receive(Shuttler& shuttle, const char* name, T& value);
      template<typename Shuttler, typename T>
      static void Shuttle(Shuttler& shuttle, T& value, unsigned int version);
      template<typename Shuttler, typename T>
      static void Shuttle(Shuttler& shuttle, const T& value,
        unsigned int version);
  };

  /*! \class Version
      \brief Stores a type's serialization version.
   */
  template<typename T>
  struct Version : std::integral_constant<unsigned int, 0> {};

  /*! \class Inverse
      \brief Type trait providing a Shuttle's inverse.
      \tparam ShuttleType The type of Shuttle whose inverse is to be given.
   */
  template<typename ShuttleType>
  struct Inverse {};

  template<typename ShuttleType>
  using GetInverse = typename Inverse<ShuttleType>::type;

  /*! \class IsStructure
      \brief Type trait for whether a type is shuttled as a structure.
      \tparam T The type to check.
   */
  template<typename T, typename Enabled = void>
  struct IsStructure : boost::mpl::if_c<std::is_class<T>::value,
    std::true_type, std::false_type>::type {};

  /*! \class IsSequence
      \brief Type trait for whether a type is shuttled as a sequence.
      \tparam T The type to check.
   */
  template<typename T>
  struct IsSequence : std::false_type {};

  /*! \class Shuttle
      \brief Contains operations for shuttling a type.
      \tparam T The type being specialized.
   */
  template<typename T, typename Enabled = void>
  struct Shuttle {

    //! Shuttles a value.
    /*!
      \tparam Shuttler The type of DataShuttle to use.
      \param shuttle The DataShuttle to use.
      \param value The value to shuttle.
      \param version The class version being serialized.
    */
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, T& value, unsigned int version) const;
  };

  //! Shuttles a value and checks that it does not evaluate to
  //! <code>nullptr</code>.
  /*!
    \tparam Shuttler The type of DataShuttle to use.
    \tparam T The type of value to shuttle.
    \param shuttle The DataShuttle to use.
    \param name The name of the value being shuttled.
    \param value The value to shuttle.
  */
  template<typename Shuttler, typename T>
  void ShuttleNonNull(Shuttler& shuttle, const char* name, T& value) {
    shuttle.Shuttle(name, value);
    if(IsReceiver<Shuttler>::value) {
      if(value == nullptr) {
        BOOST_THROW_EXCEPTION(SerializationException("Invalid null value."));
      }
    }
  }

  template<typename T>
  T* DataShuttle::Builder() {
    return BuilderHelper<T>()();
  }

  template<typename T>
  void DataShuttle::Builder(SerializedValue<T>& value) {
    return SerializedValueBuilder<T>()(value);
  }

  template<typename Shuttler, typename T>
  void DataShuttle::Send(Shuttler& shuttle, const T& value,
      unsigned int version) {
    value.Send(shuttle, version);
  }

  template<typename Shuttler, typename T>
  void DataShuttle::Send(Shuttler& shuttle, const char* name, const T& value) {
    value.Send(shuttle, name);
  }

  template<typename Shuttler, typename T>
  void DataShuttle::Receive(Shuttler& shuttle, T& value, unsigned int version) {
    value.Receive(shuttle, version);
  }

  template<typename Shuttler, typename T>
  void DataShuttle::Receive(Shuttler& shuttle, const char* name, T& value) {
    value.Receive(shuttle, name);
  }

  template<typename Shuttler, typename T>
  void DataShuttle::Shuttle(Shuttler& shuttle, T& value, unsigned int version) {
    value.Shuttle(shuttle, version);
  }

  template<typename Shuttler, typename T>
  void DataShuttle::Shuttle(Shuttler& shuttle, const T& value,
      unsigned int version) {
    const_cast<T&>(value).Shuttle(shuttle, version);
  }

  template<typename T, typename Enabled>
  template<typename Shuttler>
  void Shuttle<T, Enabled>::operator ()(Shuttler& shuttle, T& value,
      unsigned int version) const {
    DataShuttle::Shuttle(shuttle, value, version);
  }

  template<typename T>
  void SerializedValue<T>::Initialize() {
    Reset();
    DataShuttle::Builder(*this);
  }
}
}

#endif
