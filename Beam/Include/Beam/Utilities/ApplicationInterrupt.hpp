#ifdef __GNUC__
  #include "Beam/Utilities/ApplicationInterruptPosix.inl"
#elif defined WIN32
  #include "Beam/Utilities/ApplicationInterruptWin32.inl"
#endif
