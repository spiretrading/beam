#ifndef BEAM_NATIVEDATATYPE_HPP
#define BEAM_NATIVEDATATYPE_HPP
#include "Beam/Queries/DataType.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Queries {

  /*! \class NativeDataType
      \brief A DataType that wraps a native data type.
      \tparam T The native type represented.
   */
  template<typename T>
  class NativeDataType : public VirtualDataType,
      public CloneableMixin<NativeDataType<T>> {
    public:

      //! The native type represented.
      using Type = T;

      //! Returns an instance of this type.
      static const NativeDataType& GetInstance();

      //! Constructs a NativeDataType.
      NativeDataType() = default;

      //! Copies a NativeDataType.
      NativeDataType(const NativeDataType& type) = default;

      virtual ~NativeDataType() override = default;

      virtual const std::type_info& GetNativeType() const override;

    protected:
      virtual bool IsEqual(const VirtualDataType& dataType) const override;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      friend struct Serialization::DataShuttle;
  };

  template<typename T>
  const NativeDataType<T>& NativeDataType<T>::GetInstance() {
    static NativeDataType<T> instance;
    return instance;
  }

  template<typename T>
  const std::type_info& NativeDataType<T>::GetNativeType() const {
    return typeid(Type);
  }

  template<typename T>
  bool NativeDataType<T>::IsEqual(const VirtualDataType& type) const {
    return true;
  }

  template<typename T>
  template<typename Shuttler>
  void NativeDataType<T>::Shuttle(Shuttler& shuttle, unsigned int version) {
    VirtualDataType::Shuttle(shuttle, version);
  }
}
}

#endif
