#include "Beam/Python/IO.hpp"
#include <boost/lexical_cast.hpp>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Python/GilRelease.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Python;
using namespace boost;
using namespace pybind11;

namespace {
  auto channelBox = std::unique_ptr<class_<ChannelBox>>();
  auto channelIdentifierBox = std::unique_ptr<class_<ChannelIdentifierBox>>();
  auto connectionBox = std::unique_ptr<class_<ConnectionBox>>();
  auto readerBox = std::unique_ptr<class_<ReaderBox>>();
  auto serverConnectionBox = std::unique_ptr<class_<ServerConnectionBox>>();
  auto writerBox = std::unique_ptr<class_<WriterBox>>();
  auto ioException = object();
}

class_<ChannelBox>& Beam::Python::GetExportedChannelBox() {
  return *channelBox;
}

class_<ChannelIdentifierBox>& Beam::Python::GetExportedChannelIdentifierBox() {
  return *channelIdentifierBox;
}

class_<ConnectionBox>& Beam::Python::GetExportedConnectionBox() {
  return *connectionBox;
}

class_<ReaderBox>& Beam::Python::GetExportedReaderBox() {
  return *readerBox;
}

class_<ServerConnectionBox>& Beam::Python::GetExportedServerConnectionBox() {
  return *serverConnectionBox;
}

class_<WriterBox>& Beam::Python::GetExportedWriterBox() {
  return *writerBox;
}

const object& Beam::Python::GetIOException() {
  return ioException;
}

void Beam::Python::ExportBufferBox(pybind11::module& module) {
  class_<BufferBox>(module, "Buffer")
    .def(init([] {
      return BufferBox(SharedBuffer());
    }))
    .def(init([] (std::size_t size) {
      return BufferBox(SharedBuffer(size));
    }))
    .def("__str__", [] (BufferBox& self) {
      return std::string(self.GetData(), self.GetSize());
    })
    .def("is_empty", &BufferBox::IsEmpty)
    .def("grow", &BufferBox::Grow)
    .def("shrink", &BufferBox::Shrink)
    .def("shrink_front", &BufferBox::ShrinkFront)
    .def("reserve", &BufferBox::Reserve)
    .def("write", [] (BufferBox& self, std::size_t index, pybind11::str value) {
      auto rawString = PyUnicode_AsUTF8(value.ptr());
      if(rawString != nullptr) {
        self.Write(index, rawString, len(value));
      }
    })
    .def("append", static_cast<void (BufferBox::*)(const BufferBox&)>(
      &BufferBox::Append<BufferBox>))
    .def("append", [] (BufferBox& self, pybind11::str value) {
      auto rawString = PyUnicode_AsUTF8(value.ptr());
      if(rawString != nullptr) {
        self.Append(rawString, len(value));
      }
    })
    .def("reset", &BufferBox::Reset)
    .def_property_readonly("size", &BufferBox::GetSize);
  implicitly_convertible<BufferBox, BufferView>();
}

void Beam::Python::ExportBufferView(pybind11::module& module) {
  class_<BufferView>(module, "BufferView")
    .def(init<const BufferBox&>())
    .def("__str__", [] (BufferView& self) {
      return std::string(self.GetData(), self.GetSize());
    })
    .def("is_empty", &BufferView::IsEmpty)
    .def_property_readonly("size", &BufferView::GetSize);
}

void Beam::Python::ExportIO(pybind11::module& module) {
  auto submodule = module.def_submodule("io");
  ExportBufferView(submodule);
  ExportBufferBox(submodule);
  channelBox = std::make_unique<class_<ChannelBox>>(ExportChannel<ChannelBox>(
    module, "Channel"));
  channelIdentifierBox = std::make_unique<class_<ChannelIdentifierBox>>(
    ExportChannelIdentifier<ChannelIdentifierBox>(module, "ChannelIdentifier"));
  connectionBox = std::make_unique<class_<ConnectionBox>>(
    ExportConnection<ConnectionBox>(module, "Connection"));
  readerBox = std::make_unique<class_<ReaderBox>>(ExportReader<ReaderBox>(
    module, "Reader"));
  serverConnectionBox = std::make_unique<class_<ServerConnectionBox>>(
    ExportServerConnection<ServerConnectionBox>(module, "ServerConnection"));
  writerBox = std::make_unique<class_<WriterBox>>(ExportWriter<WriterBox>(
    module, "Writer"));
  ExportOpenState(submodule);
  ExportSharedBuffer(submodule);
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

void Beam::Python::ExportSharedBuffer(pybind11::module& module) {
  class_<SharedBuffer>(module, "SharedBuffer")
    .def(init())
    .def(init<std::size_t>())
    .def(init<const SharedBuffer&>())
    .def("__str__", [] (SharedBuffer& self) {
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
    .def("append", [] (SharedBuffer& self, pybind11::str value) {
      auto rawString = PyUnicode_AsUTF8(value.ptr());
      if(rawString != nullptr) {
        self.Append(rawString, len(value));
      }
    })
    .def("reset", &SharedBuffer::Reset)
    .def_property_readonly("size", &SharedBuffer::GetSize);
  implicitly_convertible<SharedBuffer, BufferBox>();
  implicitly_convertible<SharedBuffer, BufferView>();
}
