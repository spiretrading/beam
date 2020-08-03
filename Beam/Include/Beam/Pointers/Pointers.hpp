#ifndef BEAM_POINTERS_HPP
#define BEAM_POINTERS_HPP
#ifdef _MSC_VER
  #pragma warning(disable: 4250)
#endif

namespace Beam {
  class Cloneable;
  template<typename T> class CloneableMixin;
  struct VirtualCloner;
  template<typename T, typename ClonerType = VirtualCloner> class ClonePtr;
  template<typename... Args> struct Initializer;
  template<typename T> class LocalPtr;
  template<typename T> class NativePtr;
  template<typename T> class Out;
  template<typename T> class Ref;
}

#endif
