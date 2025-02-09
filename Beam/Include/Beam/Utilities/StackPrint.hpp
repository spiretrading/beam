#ifdef __GNUC__
  #include "Beam/Utilities/StackPrintPosix.inl"
#elif defined WIN32
  #include "Beam/Utilities/StackPrintWin32.inl"
#endif
