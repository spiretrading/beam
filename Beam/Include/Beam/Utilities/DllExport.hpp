#ifndef BEAM_DLL_EXPORTS
#define BEAM_DLL_EXPORTS

#ifdef _MSC_VER
  #define BEAM_EXTERN
  #define BEAM_EMIT_DLL
  #define BEAM_EXPORT_CLASS
  #if defined(BEAM_BUILD_DLL)
    #define BEAM_EXPORT_DLL __declspec(dllexport)
  #elif defined(BEAM_USE_DLL)
    #define BEAM_EXPORT_DLL __declspec(dllimport)
  #else
    #define BEAM_EXPORT_DLL
  #endif
#else
  #if defined(BEAM_BUILD_DLL)
    #define BEAM_EMIT_DLL __attribute__((used))
  #else
    #define BEAM_EMIT_DLL
  #endif
  #if defined(BEAM_BUILD_DLL) || defined(BEAM_USE_DLL)
    #define BEAM_EXTERN extern
    #define BEAM_EXPORT_CLASS __attribute__((visibility ("default")))
    #define BEAM_EXPORT_DLL __attribute__((visibility ("default")))
  #else
    #define BEAM_EXTERN
    #define BEAM_EXPORT_CLASS
    #define BEAM_EXPORT_DLL
  #endif
#endif

#endif
