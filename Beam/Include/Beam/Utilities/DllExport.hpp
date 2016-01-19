#ifndef BEAM_DLLEXPORT_HPP
#define BEAM_DLLEXPORT_HPP

#ifdef BEAM_BUILD_DLL
  #ifdef _MSC_VER
    #define BEAM_EXPORT_DLL __declspec(dllexport)
    #define BEAM_EXTERN
  #else
    #define BEAM_EXPORT_DLL
    #define BEAM_EXTERN
  #endif
#elif defined BEAM_USE_DLL
  #ifdef _MSC_VER
    #define BEAM_EXPORT_DLL __declspec(dllimport)
    #define BEAM_EXTERN extern
  #else
    #define BEAM_EXPORT_DLL
    #define BEAM_EXTERN
  #endif
#else
  #define BEAM_EXPORT_DLL
  #define BEAM_EXTERN
#endif

#endif
