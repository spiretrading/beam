#include "Beam/Python/IO.hpp"
#include <boost/lexical_cast.hpp>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/VirtualChannel.hpp"
#include "Beam/IO/VirtualChannelIdentifier.hpp"
#include "Beam/IO/VirtualConnection.hpp"
#include "Beam/IO/VirtualServerConnection.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Python/GilRelease.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Python;
using namespace boost;
using namespace pybind11;

namespace {
  object ioException;

  struct TrampolineConnection final : VirtualConnection {
    void Close() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualConnection, "close", Close);
    }
  };

  struct TrampolineServerConnection final : VirtualServerConnection {
    std::unique_ptr<VirtualChannel> Accept() override {
      return MakeVirtualChannel(PythonAccept());
    }

    std::shared_ptr<VirtualChannel> PythonAccept() {
      PYBIND11_OVERLOAD_PURE_NAME(std::shared_ptr<VirtualChannel>,
        VirtualServerConnection, "accept", PythonAccept);
    }
  };

  struct TrampolineReader final : VirtualReader {
    bool IsDataAvailable() const {
      PYBIND11_OVERLOAD_PURE_NAME(bool, VirtualReader, "is_data_available",
        IsDataAvailable);
    }

    std::size_t Read(Out<SharedBuffer> destination) {
      PYBIND11_OVERLOAD_PURE_NAME(std::size_t, VirtualReader, "read", Read,
        Store(destination));
    }

    std::size_t Read(char* destination, std::size_t size) {
      PYBIND11_OVERLOAD_PURE_NAME(std::size_t, VirtualReader, "read", Read,
        destination, size);
    }

    std::size_t Read(Out<SharedBuffer> destination, std::size_t size) {
      PYBIND11_OVERLOAD_PURE_NAME(std::size_t, VirtualReader, "read", Read,
        Store(destination), size);
    }
  };

  struct TrampolineWriter final : VirtualWriter {
    void Write(const void* data, std::size_t size) {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualWriter, "write", data, size);
    }

    void Write(const Buffer& data) {
      PYBIND11_OVERLOAD_PURE_NAME(void, VirtualWriter, "write", data);
    }
  };
}

const object& Beam::Python::GetIOException() {
  return ioException;
}

void Beam::Python::ExportChannel(pybind11::module& module) {
  class_<VirtualChannel>(module, "Channel")
    .def_property_readonly("identifier", &VirtualChannel::GetIdentifier,
      return_value_policy::reference_internal)
    .def_property_readonly("connection", &VirtualChannel::GetConnection,
      return_value_policy::reference_internal)
    .def_property_readonly("reader", &VirtualChannel::GetReader,
      return_value_policy::reference_internal)
    .def_property_readonly("writer", &VirtualChannel::GetWriter,
      return_value_policy::reference_internal);
}

void Beam::Python::ExportChannelIdentifier(pybind11::module& module) {
  class_<VirtualChannelIdentifier>(module, "ChannelIdentifier")
    .def("__str__", &lexical_cast<std::string, VirtualChannelIdentifier>);
}

void Beam::Python::ExportConnection(pybind11::module& module) {
  class_<VirtualConnection, TrampolineConnection>(module, "Connection")
    .def("close", &VirtualConnection::Close);
}

void Beam::Python::ExportIO(pybind11::module& module) {
  auto submodule = module.def_submodule("io");
  ExportChannel(submodule);
  ExportChannelIdentifier(submodule);
  ExportConnection(submodule);
  ExportOpenState(submodule);
  ExportReader(submodule);
  ExportServerConnection(submodule);
  ExportSharedBuffer(submodule);
  ExportWriter(submodule);
  ioException = register_exception<IOException>(submodule, "IOException");
  register_exception<ConnectException>(submodule, "ConnectException",
    ioException.ptr());
  register_exception<EndOfFileException>(submodule, "EndOfFileException",
    ioException.ptr());
  register_exception<NotConnectedException>(submodule, "NotConnectedException",
    ioException.ptr());
}

void Beam::Python::ExportOpenState(pybind11::module& module) {
  class_<OpenState>(module, "OpenState")
    .def(init())
    .def_property_readonly("is_open", &OpenState::IsOpen)
    .def_property_readonly("is_closing", &OpenState::IsClosing)
    .def_property_readonly("is_closed", &OpenState::IsClosed)
    .def("ensure_open", &OpenState::EnsureOpen)
    .def("set_closing", &OpenState::SetClosing, call_guard<GilRelease>())
    .def("close", &OpenState::Close, call_guard<GilRelease>());
}

void Beam::Python::ExportReader(pybind11::module& module) {
  class_<VirtualReader, TrampolineReader>(module, "Reader")
    .def("is_data_available", &VirtualReader::IsDataAvailable)
    .def("read", static_cast<std::size_t (VirtualReader::*)(Out<SharedBuffer>)>(
      &VirtualReader::Read))
    .def("read", static_cast<std::size_t (VirtualReader::*)(
      Out<SharedBuffer>, std::size_t)>(&VirtualReader::Read));
}

void Beam::Python::ExportServerConnection(pybind11::module& module) {
  class_<VirtualServerConnection, VirtualConnection,
      TrampolineServerConnection>(module, "ServerConnection")
    .def("accept", &VirtualServerConnection::Accept);
}

void Beam::Python::ExportSharedBuffer(pybind11::module& module) {
  class_<SharedBuffer>(module, "Buffer")
    .def(init())
    .def(init<std::size_t>())
    .def(init<const SharedBuffer&>())
    .def("__str__",
      [] (SharedBuffer& self) {
        return std::string(self.GetData(), self.GetSize());
      })
    .def("is_empty", &SharedBuffer::IsEmpty)
    .def("grow", &SharedBuffer::Grow)
    .def("shrink", &SharedBuffer::Shrink)
    .def("shrink_front", &SharedBuffer::ShrinkFront)
    .def("reserve", &SharedBuffer::Reserve)
    .def("write",
      [] (SharedBuffer& self, std::size_t index, pybind11::str value) {
        auto rawString = PyUnicode_AsUTF8(value.ptr());
        if(rawString != nullptr) {
          self.Write(index, rawString, len(value));
        }
      })
    .def("append", static_cast<void (SharedBuffer::*)(const SharedBuffer&)>(
      &SharedBuffer::Append))
    .def("append",
      [] (SharedBuffer& self, pybind11::str value) {
        auto rawString = PyUnicode_AsUTF8(value.ptr());
        if(rawString != nullptr) {
          self.Append(rawString, len(value));
        }
      })
    .def("reset", &SharedBuffer::Reset)
    .def_property_readonly("size", &SharedBuffer::GetSize);
}

void Beam::Python::ExportWriter(pybind11::module& module) {
  class_<VirtualWriter, TrampolineWriter>(module, "Writer")
    .def("write", static_cast<void (VirtualWriter::*)(const SharedBuffer&)>(
      &VirtualWriter::Write));
}
