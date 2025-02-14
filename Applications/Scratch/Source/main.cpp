#include <iostream>
#include <windows.h>
#include <winternl.h>
#include <ntstatus.h>
//#include "Beam/Routines/Routine.hpp"

int main() {
  auto kernelModule = GetModuleHandleW(L"ntdll.dll");
  auto target = reinterpret_cast<
    NTSTATUS (NTAPI*)(_In_ void*,
      _In_ void*, _In_ size_t, _In_opt_ PLARGE_INTEGER)>(
        reinterpret_cast<std::uint8_t*>(
          GetProcAddress(kernelModule, "RtlWaitOnAddress")));
  target(nullptr, nullptr, 0, nullptr);
  return 0;
}
