#include "Beam/Python/IO.hpp"
#include "Beam/IO/VirtualChannel.hpp"
#include "Beam/IO/VirtualChannelIdentifier.hpp"
#include "Beam/IO/VirtualConnection.hpp"
#include "Beam/IO/OpenState.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Python;
using namespace pybind11;

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
    .def("__str__", &VirtualChannelIdentifier::ToString);
}

void Beam::Python::ExportConnection(pybind11::module& module) {
  class_<VirtualConnection>(module, "Connection")
    .def("open", &VirtualConnection::Open, call_guard<gil_scoped_release>())
    .def("close", &VirtualConnection::Close, call_guard<gil_scoped_release>());
}

void Beam::Python::ExportIO(pybind11::module& module) {
  auto submodule = module.def_submodule("io");
  ExportChannel(submodule);
  ExportChannelIdentifier(submodule);
  ExportConnection(submodule);
  ExportOpenState(submodule);
  ExportReader(submodule);
  ExportSharedBuffer(submodule);
  ExportWriter(submodule);

#if 0 // TODO Exceptions
  ExportException<IOException, std::runtime_error>("IOException")
    .def(init<>())
    .def(init<const string&>());
  ExportException<ConnectException, IOException>("ConnectException")
    .def(init<>())
    .def(init<const string&>());
  ExportException<EndOfFileException, IOException>("EndOfFileException")
    .def(init<>())
    .def(init<const string&>());
  ExportException<NotConnectedException, IOException>("NotConnectedException")
    .def(init<>())
    .def(init<const string&>());
#endif
}

void Beam::Python::ExportOpenState(pybind11::module& module) {
  class_<OpenState>(module, "OpenState")
    .def(init())
    .def(init<bool>())
    .def("is_opening", &OpenState::IsOpening, call_guard<gil_scoped_release>())
    .def("is_open", &OpenState::IsOpen, call_guard<gil_scoped_release>())
    .def("is_running", &OpenState::IsRunning, call_guard<gil_scoped_release>())
    .def("is_closing", &OpenState::IsClosing, call_guard<gil_scoped_release>())
    .def("is_closed", &OpenState::IsClosed, call_guard<gil_scoped_release>())
    .def("set_opening", &OpenState::SetOpening,
      call_guard<gil_scoped_release>())
    .def("set_open", &OpenState::SetOpen, call_guard<gil_scoped_release>())
    .def("set_open_failure",
      static_cast<void (OpenState::*)()>(&OpenState::SetOpenFailure),
      call_guard<gil_scoped_release>())
    .def("set_open_failure",
      static_cast<void (OpenState::*)(const std::exception_ptr&)>(
      &OpenState::SetOpenFailure), call_guard<gil_scoped_release>())
    .def("set_closing", &OpenState::SetClosing,
      call_guard<gil_scoped_release>())
    .def("set_closed", &OpenState::SetClosed, call_guard<gil_scoped_release>());
}

void Beam::Python::ExportReader(pybind11::module& module) {
  class_<VirtualReader>(module, "Reader")
    .def("is_data_available", &VirtualReader::IsDataAvailable,
      call_guard<gil_scoped_release>())
    .def("read", static_cast<std::size_t (VirtualReader::*)(Out<SharedBuffer>)>(
      &VirtualReader::Read), call_guard<gil_scoped_release>())
    .def("read", static_cast<std::size_t (VirtualReader::*)(
      Out<SharedBuffer>, std::size_t)>(&VirtualReader::Read),
      call_guard<gil_scoped_release>());
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
  class_<VirtualWriter>(module, "Writer")
    .def("write", static_cast<void (VirtualWriter::*)(const SharedBuffer&)>(
      &VirtualWriter::Write), call_guard<gil_scoped_release>());
}
