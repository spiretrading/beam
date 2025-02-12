#ifndef BEAM_STACK_PRINT_WIN32_HPP
#define BEAM_STACK_PRINT_WIN32_HPP
#pragma comment(lib, "DbgHelp.lib")
#include <array>
#include <mutex>
#include <string>
#include <Windows.h>
#include <DbgHelp.h>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {
  inline std::string CaptureStackPrint() {
    static auto symInitializeFlag = std::once_flag();
    static auto process = GetCurrentProcess();
    std::call_once(symInitializeFlag, [] {
      SymInitialize(process, nullptr, TRUE);
    });
    #pragma warning(push)
    #pragma warning(disable:4191)
    static auto RtlCaptureStackBackTrace = reinterpret_cast<USHORT (WINAPI*)(
      __in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG)>(GetProcAddress(
        LoadLibrary("kernel32.dll"), "RtlCaptureStackBackTrace"));
    #pragma warning(pop)
    static auto mutex = std::mutex();
    auto lock = std::lock_guard(mutex);
    auto stack = std::array<void*, 100>();
    auto frames =
      RtlCaptureStackBackTrace(0, stack.size(), stack.data(), nullptr);
    auto symbol = static_cast<SYMBOL_INFO*>(
      std::calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1));
    if(!symbol) {
      return std::string();
    }
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    auto stackPrint = std::string();
    for(auto i = 0; i < frames; ++i) {
      auto buffer = std::array<char, 2048>();
      SymFromAddr(process,
        static_cast<DWORD64>(reinterpret_cast<UINT_PTR>(stack[i])), 0, symbol);
      std::sprintf(buffer.data(), "%i: %s - 0x%I64X\n", frames - i - 1,
        symbol->Name, symbol->Address);
      stackPrint += buffer.data();
    }
    std::free(symbol);
    return stackPrint;
  }
}

#endif
