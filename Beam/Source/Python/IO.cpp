#include "Beam/Python/IO.hpp"
#include <boost/lexical_cast.hpp>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include "Beam/IO/AsyncWriter.hpp"
#include "Beam/IO/BasicIStreamReader.hpp"
#include "Beam/IO/BasicOStreamWriter.hpp"
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/BufferSlice.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalConnection.hpp"
#include "Beam/IO/LocalServerChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
#include "Beam/IO/NotConnectedException.hpp"
#include "Beam/IO/NullChannel.hpp"
#include "Beam/IO/NullConnection.hpp"
#include "Beam/IO/NullReader.hpp"
#include "Beam/IO/NullWriter.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/QueuedReader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/SizeDeclarativeReader.hpp"
#include "Beam/IO/SizeDeclarativeWriter.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/ToPythonChannel.hpp"
#include "Beam/Python/ToPythonConnection.hpp"
#include "Beam/Python/ToPythonReader.hpp"
#include "Beam/Python/ToPythonServerConnection.hpp"
#include "Beam/Python/ToPythonWriter.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Python;
using namespace boost;
using namespace pybind11;

namespace {
  auto bufferBox = std::unique_ptr<class_<BufferBox>>();
  auto bufferView = std::unique_ptr<class_<BufferView>>();
  auto channelBox = std::unique_ptr<class_<ChannelBox>>();
  auto channelIdentifierBox = std::unique_ptr<class_<ChannelIdentifierBox>>();
  auto connectionBox = std::unique_ptr<class_<ConnectionBox>>();
  auto readerBox = std::unique_ptr<class_<ReaderBox>>();
  auto serverConnectionBox = std::unique_ptr<class_<ServerConnectionBox>>();
  auto writerBox = std::unique_ptr<class_<WriterBox>>();
  auto ioException = object();
}

class_<BufferBox>& Beam::Python::GetExportedBufferBox() {
  return *bufferBox;
}

class_<BufferView>& Beam::Python::GetExportedBufferView() {
  return *bufferView;
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

void Beam::Python::ExportAsyncWriter(module& module) {
  ExportWriter<ToPythonWriter<AsyncWriter<WriterBox>>>(module, "AsyncWriter").
    def(init<WriterBox>());
}

void Beam::Python::ExportBufferBox(module& module) {
  bufferBox = std::make_unique<class_<BufferBox>>(
    ExportBuffer<BufferBox>(module, "Buffer"));
}

void Beam::Python::ExportBufferReader(module& module) {
  ExportReader<ToPythonReader<BufferReader<BufferBox>>>(module, "BufferReader").
    def(init<BufferBox>()).
    def(init([] (const ToPythonReader<BufferReader<BufferBox>>& reader) {
      return std::make_shared<ToPythonReader<BufferReader<BufferBox>>>(
        reader.GetReader());
    }));
}

void Beam::Python::ExportBufferSlice(module& module) {
  ExportBuffer<BufferSlice<BufferBox>>(module, "BufferSlice").
    def(init([] (BufferBox& buffer, std::size_t offset) {
      return BufferSlice(Ref(buffer), offset);
    }));
}

void Beam::Python::ExportBufferView(module& module) {
  bufferView = std::make_unique<class_<BufferView>>(module, "BufferView");
  bufferView->def("__str__", [] (BufferView& self) {
      return std::string(self.GetData(), self.GetSize());
    }).
    def("is_empty", &BufferView::IsEmpty).
    def_property_readonly("size", &BufferView::GetSize);
}

void Beam::Python::ExportIO(module& module) {
  auto submodule = module.def_submodule("io");
  ExportBufferView(submodule);
  ExportBufferBox(submodule);
  channelBox = std::make_unique<class_<ChannelBox>>(ExportChannel<ChannelBox>(
    submodule, "Channel"));
  channelIdentifierBox = std::make_unique<class_<ChannelIdentifierBox>>(
    ExportChannelIdentifier<ChannelIdentifierBox>(submodule,
    "ChannelIdentifier"));
  connectionBox = std::make_unique<class_<ConnectionBox>>(
    ExportConnection<ConnectionBox>(submodule, "Connection"));
  readerBox = std::make_unique<class_<ReaderBox>>(ExportReader<ReaderBox>(
    submodule, "Reader"));
  serverConnectionBox = std::make_unique<class_<ServerConnectionBox>>(
    ExportServerConnection<ServerConnectionBox>(submodule, "ServerConnection"));
  writerBox = std::make_unique<class_<WriterBox>>(ExportWriter<WriterBox>(
    submodule, "Writer"));
  ExportAsyncWriter(submodule);
  ExportBufferReader(submodule);
  ExportBufferSlice(submodule);
  ExportLocalClientChannel(submodule);
  ExportLocalConnection(submodule);
  ExportLocalServerChannel(submodule);
  ExportLocalServerConnection(submodule);
  ExportNamedChannelIdentifier(submodule);
  ExportNullChannel(submodule);
  ExportNullConnection(submodule);
  ExportNullReader(submodule);
  ExportNullWriter(submodule);
  ExportOpenState(submodule);
  ExportPipedReader(submodule);
  ExportPipedWriter(submodule);
  ExportQueuedReader(submodule);
  ExportSharedBuffer(submodule);
  ExportSizeDeclarativeReader(submodule);
  ExportSizeDeclarativeWriter(submodule);
  ExportStdinReader(submodule);
  ExportStdoutWriter(submodule);
  ioException = register_exception<IOException>(submodule, "IOException");
  register_exception<ConnectException>(submodule, "ConnectException",
    ioException.ptr());
  register_exception<EndOfFileException>(submodule, "EndOfFileException",
    ioException.ptr());
  register_exception<NotConnectedException>(submodule, "NotConnectedException",
    ioException.ptr());
}

void Beam::Python::ExportLocalClientChannel(module& module) {
  ExportChannel<ToPythonChannel<LocalClientChannel<SharedBuffer>>>(module,
    "LocalClientChannel").
    def(init([] (const std::string& name,
        ToPythonServerConnection<LocalServerConnection<SharedBuffer>>&
        connection) {
      return std::make_shared<
        ToPythonChannel<LocalClientChannel<SharedBuffer>>>(name,
          connection.GetConnection());
    }));
}

void Beam::Python::ExportLocalConnection(module& module) {
  ExportConnection<ToPythonConnection<LocalConnection<SharedBuffer>>>(module,
    "LocalConnection");
}

void Beam::Python::ExportLocalServerChannel(module& module) {
  ExportChannel<ToPythonChannel<LocalServerChannel<SharedBuffer>>>(module,
    "LocalServerChannel");
}

void Beam::Python::ExportLocalServerConnection(module& module) {
  ExportServerConnection<
    ToPythonServerConnection<LocalServerConnection<SharedBuffer>>>(module,
    "LocalServerConnection").
    def(init());
}

void Beam::Python::ExportNamedChannelIdentifier(module& module) {
  ExportChannelIdentifier<NamedChannelIdentifier>(module,
    "NamedChannelIdentifier").
    def(init<std::string>()).
    def_property_readonly("name", &NamedChannelIdentifier::GetName);
}

void Beam::Python::ExportNullChannel(module& module) {
  ExportChannel<ToPythonChannel<NullChannel>>(module, "NullChannel").
    def(init()).
    def(init<const NamedChannelIdentifier&>());
}

void Beam::Python::ExportNullConnection(module& module) {
  ExportConnection<ToPythonConnection<NullConnection>>(module,
    "NullConnection").
    def(init());
}

void Beam::Python::ExportNullReader(module& module) {
  ExportReader<ToPythonReader<NullReader>>(module, "NullReader").
    def(init());
}

void Beam::Python::ExportNullWriter(module& module) {
  ExportWriter<ToPythonWriter<NullWriter>>(module, "NullWriter").
    def(init());
}

void Beam::Python::ExportOpenState(module& module) {
  class_<OpenState>(module, "OpenState").
    def(init()).
    def_property_readonly("is_open", &OpenState::IsOpen).
    def_property_readonly("is_closing", &OpenState::IsClosing).
    def_property_readonly("is_closed", &OpenState::IsClosed).
    def("ensure_open", &OpenState::EnsureOpen).
    def("set_closing", &OpenState::SetClosing, call_guard<GilRelease>()).
    def("close", &OpenState::Close, call_guard<GilRelease>());
}

void Beam::Python::ExportPipedReader(module& module) {
  ExportReader<ToPythonReader<PipedReader<SharedBuffer>>>(module,
    "PipedReader").
    def(init());
}

void Beam::Python::ExportPipedWriter(module& module) {
  ExportWriter<ToPythonWriter<PipedWriter<SharedBuffer>>>(module,
    "PipedWriter").
    def(init([] (ToPythonReader<PipedReader<SharedBuffer>>& reader) {
      return std::make_shared<ToPythonWriter<PipedWriter<SharedBuffer>>>(
        Ref(reader.GetReader()));
    }));
}

void Beam::Python::ExportQueuedReader(module& module) {
  ExportReader<ToPythonReader<QueuedReader<SharedBuffer, ReaderBox>>>(module,
    "QueuedReader").
    def(init<ReaderBox>());
}

void Beam::Python::ExportSharedBuffer(module& module) {
  ExportBuffer<SharedBuffer>(module, "SharedBuffer").
    def(init()).
    def(init<std::size_t>()).
    def(init<const SharedBuffer&>());
}

void Beam::Python::ExportSizeDeclarativeReader(module& module) {
  ExportReader<ToPythonReader<SizeDeclarativeReader<ReaderBox>>>(module,
    "SizeDeclarativeReader").
    def(init<ReaderBox>());
}

void Beam::Python::ExportSizeDeclarativeWriter(module& module) {
  ExportWriter<ToPythonWriter<SizeDeclarativeWriter<WriterBox>>>(module,
    "SizeDeclarativeWriter").
    def(init<WriterBox>());
}

void Beam::Python::ExportStdinReader(module& module) {
  ExportReader<ToPythonReader<BasicIStreamReader<std::istream*>>>(module,
    "StdinReader").
    def(init([] {
      return std::make_shared<
        ToPythonReader<BasicIStreamReader<std::istream*>>>(&std::cin);
    }));
}

void Beam::Python::ExportStdoutWriter(module& module) {
  ExportWriter<ToPythonWriter<BasicOStreamWriter<std::ostream*>>>(module,
    "StdoutWriter").
    def(init([] {
      return std::make_shared<
        ToPythonWriter<BasicOStreamWriter<std::ostream*>>>(&std::cout);
    }));
}
