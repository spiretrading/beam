#ifndef BEAM_QUERIES_VALUE_HPP
#define BEAM_QUERIES_VALUE_HPP
#include <ostream>
#include "Beam/Pointers/ClonePtr.hpp"
#include "Beam/Queries/DataType.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Utilities/Streamable.hpp"

namespace Beam::Queries {

  /** Base class used to encapsulate a value used in a Query. */
  class VirtualValue : public Streamable, public virtual Cloneable {
    public:
      virtual ~VirtualValue() = default;

      /** Returns the VirtualValue's data type. */
      virtual const DataType& GetType() const = 0;

      /** Returns the value stored. */
      template<typename T>
      const T& GetValue() const;

    protected:

      /** Constructs a VirtualValue. */
      VirtualValue() = default;

      /**
       * Copies a VirtualValue.
       * @param value The value to copy.
       */
      VirtualValue(const VirtualValue& type) = default;

      /** Returns a raw pointer to the stored value. */
      virtual const void* GetValuePtr() const = 0;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      friend struct Serialization::DataShuttle;
  };

  template<typename T>
  const T& VirtualValue::GetValue() const {
    return *static_cast<const T*>(GetValuePtr());
  }

  template<typename Shuttler>
  void VirtualValue::Shuttle(Shuttler& shuttle, unsigned int version) {}
}

#endif
