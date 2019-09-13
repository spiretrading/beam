#ifndef BEAM_DLL_EXPORT_HPP
#define BEAM_DLL_EXPORT_HPP

#ifdef _MSC_VER
  #ifdef BEAM_BUILD_DLL
    #define BEAM_EXPORT_DLL __declspec(dllexport)
    #define BEAM_EXTERN
  #else
    #define BEAM_EXPORT_DLL __declspec(dllimport)
    #define BEAM_EXTERN extern
  #endif
#else
  #ifdef BEAM_BUILD_DLL
    #define BEAM_EXPORT_DLL __attribute__((visibility ("default")))
    #define BEAM_EXTERN extern
  #else
    #define BEAM_EXPORT_DLL
    #define BEAM_EXTERN extern
  #endif
#endif

#endif
