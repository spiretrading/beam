#ifndef BEAM_TYPEINDEX_HPP
#define BEAM_TYPEINDEX_HPP
#include <typeinfo>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class TypeIndex
      \brief Stores a type's RTTI.
   */
  class TypeIndex {
    public:

      //! Constructs a TypeIndex for a void type.
      TypeIndex();

      //! Constructs a TypeIndex for a specified type.
      /*!
        \param typeInfo The type to represent.
      */
      TypeIndex(const std::type_info& typeInfo);

      bool operator <(const TypeIndex& typeIndex) const;

      bool operator <=(const TypeIndex& typeIndex) const;

      bool operator ==(const TypeIndex& typeIndex) const;

      bool operator !=(const TypeIndex& typeIndex) const;

      bool operator >=(const TypeIndex& typeIndex) const;

      bool operator >(const TypeIndex& typeIndex) const;

    private:
      const std::type_info* m_typeInfo;
  };

  inline TypeIndex::TypeIndex()
      : m_typeInfo(&typeid(void)) {}

  inline TypeIndex::TypeIndex(const std::type_info& typeInfo)
      : m_typeInfo(&typeInfo) {}

  inline bool TypeIndex::operator <(const TypeIndex& typeIndex) const {
    return m_typeInfo->before(*typeIndex.m_typeInfo);
  }

  inline bool TypeIndex::operator <=(const TypeIndex& typeIndex) const {
    return (*this < typeIndex) || (*this == typeIndex);
  }

  inline bool TypeIndex::operator ==(const TypeIndex& typeIndex) const {
    return *m_typeInfo == *typeIndex.m_typeInfo;
  }

  inline bool TypeIndex::operator !=(const TypeIndex& typeIndex) const {
    return !(*this == typeIndex);
  }

  inline bool TypeIndex::operator >=(const TypeIndex& typeIndex) const {
    return !(*this < typeIndex);
  }

  inline bool TypeIndex::operator >(const TypeIndex& typeIndex) const {
    return !(*this < typeIndex) && (*this != typeIndex);
  }
}

#endif
