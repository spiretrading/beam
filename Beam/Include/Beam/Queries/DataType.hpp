#ifndef BEAM_QUERYDATATYPE_HPP
#define BEAM_QUERYDATATYPE_HPP
#include <typeinfo>
#include "Beam/Pointers/ClonePtr.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Utilities/Streamable.hpp"

namespace Beam {
namespace Queries {

  /*! \class VirtualDataType
      \brief Base class representing the data type an Expression evaluates to.
   */
  class VirtualDataType : public virtual Cloneable, public Streamable {
    public:
      virtual ~VirtualDataType() = default;

      //! Returns the native type represented.
      virtual const std::type_info& GetNativeType() const = 0;

      //! Checks if two VirtualDataTypes represent the same type.
      /*!
        \param dataType The VirtualDataType to compare to.
        \return <code>true</code> iff <code>this</code> has the same runtime
                type as <i>dataType</i> and they are both equal.
      */
      bool operator ==(const VirtualDataType& dataType) const;

      //! Checks if two VirtualDataTypes represent different types.
      /*!
        \param dataType The VirtualDataType to compare to.
        \return <code>true</code> iff <code>this</code> has a different runtime
                type as <i>dataType</i> or they they are both equal.
      */
      bool operator !=(const VirtualDataType& dataType) const;

    protected:

      //! Constructs a DataType.
      VirtualDataType() = default;

      //! Copies a DataType.
      /*!
        \param type The type to copy.
      */
      VirtualDataType(const VirtualDataType& type) = default;

      //! Checks if two VirtualDataTypes represent the same type.
      /*!
        \param dataType The VirtualDataType to compare to.
        \return <code>true</code> iff <code>this</code> has the same runtime
                type as <i>dataType</i> and they are both equal.
      */
      virtual bool IsEqual(const VirtualDataType& dataType) const = 0;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      friend struct Serialization::DataShuttle;
  };

  inline bool VirtualDataType::operator ==(
      const VirtualDataType& dataType) const {
    return typeid(*this) == typeid(dataType) && IsEqual(dataType);
  }

  inline bool VirtualDataType::operator !=(
      const VirtualDataType& dataType) const {
    return !(*this == dataType);
  }

  template<typename Shuttler>
  void VirtualDataType::Shuttle(Shuttler& shuttle, unsigned int version) {}
}
}

#endif
