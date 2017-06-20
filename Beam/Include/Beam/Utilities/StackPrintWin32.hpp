#ifndef BEAM_STACKPRINTWIN32_HPP
#define BEAM_STACKPRINTWIN32_HPP
#pragma comment(lib, "DbgHelp.lib")
#include <string>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <Windows.h>
#include <DbgHelp.h>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {
  inline std::string CaptureStackPrint() {
    static boost::mutex mutex;
    boost::lock_guard<boost::mutex> lock{mutex};
    std::string stackPrint;
    typedef USHORT (WINAPI *CaptureStackBackTraceType)(__in ULONG, __in ULONG,
      __out PVOID*, __out_opt PULONG);
    static CaptureStackBackTraceType func = (CaptureStackBackTraceType)(
      GetProcAddress(LoadLibrary("kernel32.dll"), "RtlCaptureStackBackTrace"));
    void* stack[100];
    unsigned short frames;
    SYMBOL_INFO* symbol;
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, TRUE);
    frames = func(0, 100, stack, nullptr);
    symbol = static_cast<SYMBOL_INFO*>(std::calloc(sizeof(SYMBOL_INFO) +
      256 * sizeof(char), 1));
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    for(int i = 0; i < frames; i++) {
      char buffer[2048];
      SymFromAddr(process, reinterpret_cast<DWORD64>(stack[i]), 0, symbol);
      std::sprintf(buffer, "%i: %s - 0x%I64X\n", frames - i - 1, symbol->Name,
        symbol->Address);
      stackPrint += buffer;
    }
    std::free(symbol);
    return stackPrint;
  }
}

#endif
