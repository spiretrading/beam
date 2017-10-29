#include "Beam/Python/IO.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/IOException.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/VirtualChannel.hpp"
#include "Beam/IO/VirtualChannelIdentifier.hpp"
#include "Beam/IO/VirtualConnection.hpp"
#include "Beam/IO/VirtualReader.hpp"
#include "Beam/IO/VirtualWriter.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Copy.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/GilRelease.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Python;
using namespace boost;
using namespace boost::python;
using namespace std;

namespace {
  std::size_t VirtualReaderRead(VirtualReader& reader, SharedBuffer& buffer) {
    return reader.Read(Store(buffer));
  }

  std::size_t VirtualReaderReadSize(VirtualReader& reader,
      SharedBuffer& buffer, std::size_t size) {
    return reader.Read(Store(buffer), size);
  }

  void SharedBufferWriteString(SharedBuffer& buffer, std::size_t index,
      boost::python::str value) {
    auto rawString = PyString_AsString(value.ptr());
    if(rawString == nullptr) {
      return;
    }
    buffer.Write(index, rawString, len(value));
  }

  void SharedBufferAppendString(SharedBuffer& buffer,
      boost::python::str value) {
    auto rawString = PyString_AsString(value.ptr());
    if(rawString == nullptr) {
      return;
    }
    buffer.Append(rawString, len(value));
  }

  std::string BufferToString(const SharedBuffer& buffer) {
    return std::string{buffer.GetData(), buffer.GetSize()};
  }

  void OpenStateSetOpenFailure(OpenState& openState,
      const boost::python::object& object) {
    openState.SetOpenFailure(std::runtime_error{extract<string>(str(object))});
  }
}

BEAM_DEFINE_PYTHON_POINTER_LINKER(VirtualChannelIdentifier);
BEAM_DEFINE_PYTHON_POINTER_LINKER(VirtualConnection);
BEAM_DEFINE_PYTHON_POINTER_LINKER(VirtualReader);
BEAM_DEFINE_PYTHON_POINTER_LINKER(VirtualWriter);

void Beam::Python::ExportChannel() {
  class_<VirtualChannel, boost::noncopyable>("Channel", no_init)
    .add_property("identifier", make_function(&VirtualChannel::GetIdentifier,
      return_internal_reference<>()))
    .add_property("connection", make_function(&VirtualChannel::GetConnection,
      return_internal_reference<>()))
    .add_property("reader", make_function(&VirtualChannel::GetReader,
      return_internal_reference<>()))
    .add_property("writer", make_function(&VirtualChannel::GetWriter,
      return_internal_reference<>()));
}

void Beam::Python::ExportChannelIdentifier() {
  class_<VirtualChannelIdentifier, boost::noncopyable>(
      "ChannelIdentifier", no_init)
    .def("__str__", &VirtualChannelIdentifier::ToString)
    .def("to_string", &VirtualChannelIdentifier::ToString);
}

void Beam::Python::ExportConnection() {
  class_<VirtualConnection, boost::noncopyable>("Connection", no_init)
    .def("open", BlockingFunction(&VirtualConnection::Open))
    .def("close", BlockingFunction(&VirtualConnection::Close));
}

void Beam::Python::ExportIO() {
  string nestedName = extract<string>(scope().attr("__name__") + ".io");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("io") = nestedModule;
  scope parent = nestedModule;
  ExportChannelIdentifier();
  ExportConnection();
  ExportOpenState();
  ExportReader();
  ExportSharedBuffer();
  ExportWriter();
  ExportChannel();
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
}

void Beam::Python::ExportOpenState() {
  class_<OpenState, boost::noncopyable>("OpenState", init<>())
    .def(init<bool>())
    .def("is_opening", BlockingFunction(&OpenState::IsOpening))
    .def("is_open", BlockingFunction(&OpenState::IsOpen))
    .def("is_running", BlockingFunction(&OpenState::IsRunning))
    .def("is_closing", BlockingFunction(&OpenState::IsClosing))
    .def("is_closed", BlockingFunction(&OpenState::IsClosed))
    .def("set_opening", BlockingFunction(&OpenState::SetOpening))
    .def("set_open", BlockingFunction(&OpenState::SetOpen))
    .def("set_open_failure", BlockingFunction(
      static_cast<void (OpenState::*)()>(&OpenState::SetOpenFailure)))
    .def("set_open_failure", BlockingFunction(OpenStateSetOpenFailure))
    .def("set_closing", BlockingFunction(&OpenState::SetClosing))
    .def("set_closed", BlockingFunction(&OpenState::SetClosed));
}

void Beam::Python::ExportReader() {
  class_<VirtualReader, boost::noncopyable>("Reader", no_init)
    .def("is_data_available", BlockingFunction(&VirtualReader::IsDataAvailable))
    .def("read", BlockingFunction(&VirtualReaderRead))
    .def("read", BlockingFunction(&VirtualReaderReadSize));
}

void Beam::Python::ExportSharedBuffer() {
  class_<SharedBuffer>("Buffer", init<>())
    .def(init<std::size_t>())
    .def("__copy__", &MakeCopy<SharedBuffer>)
    .def("__deepcopy__", &MakeDeepCopy<SharedBuffer>)
    .def("__str__", &BufferToString)
    .def("is_empty", &SharedBuffer::IsEmpty)
    .def("grow", &SharedBuffer::Grow)
    .def("shrink", &SharedBuffer::Shrink)
    .def("shrink_front", &SharedBuffer::ShrinkFront)
    .def("reserve", &SharedBuffer::Reserve)
    .def("write", &SharedBufferWriteString)
    .def("append", static_cast<void (SharedBuffer::*)(const SharedBuffer&)>(
      &SharedBuffer::Append))
    .def("append", &SharedBufferAppendString)
    .def("reset", &SharedBuffer::Reset)
    .add_property("size", &SharedBuffer::GetSize);
}

void Beam::Python::ExportWriter() {
  class_<VirtualWriter, boost::noncopyable>("Writer", no_init)
    .def("write", BlockingFunction(
      static_cast<void (VirtualWriter::*)(const SharedBuffer&)>(
      &VirtualWriter::Write)));
}
