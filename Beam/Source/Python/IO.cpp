#include "Beam/Python/IO.hpp"
#include <iostream>
#include "Beam/IO/AsyncWriter.hpp"
#include "Beam/IO/BasicChannel.hpp"
#include "Beam/IO/BasicIStreamReader.hpp"
#include "Beam/IO/BasicOStreamWriter.hpp"
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/BufferWriter.hpp"
#include "Beam/IO/ConnectException.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/IOException.hpp"
#include "Beam/IO/LocalClientChannel.hpp"
#include "Beam/IO/LocalConnection.hpp"
#include "Beam/IO/LocalServerChannel.hpp"
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/NamedChannelIdentifier.hpp"
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
#include "Beam/IO/StaticBuffer.hpp"
#include "Beam/IO/SuffixBuffer.hpp"
#include "Beam/Python/ToPythonChannel.hpp"
#include "Beam/Python/ToPythonConnection.hpp"
#include "Beam/Python/ToPythonReader.hpp"
#include "Beam/Python/ToPythonServerConnection.hpp"
#include "Beam/Python/ToPythonWriter.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace pybind11;

namespace {
  auto buffer_ref = std::unique_ptr<class_<BufferRef>>();
  auto channel = std::unique_ptr<class_<Channel>>();
  auto channel_identifier = std::unique_ptr<class_<ChannelIdentifier>>();
  auto connection = std::unique_ptr<class_<Connection>>();
  auto connect_exception = object();
  auto io_exception = object();
  auto reader = std::unique_ptr<class_<Reader>>();
  auto server_connection = std::unique_ptr<class_<ServerConnection>>();
  auto writer = std::unique_ptr<class_<Writer>>();
}

const object& Beam::Python::get_connect_exception() {
  return connect_exception;
}

class_<BufferRef>& Beam::Python::get_exported_buffer_ref() {
  return *buffer_ref;
}

class_<Channel>& Beam::Python::get_exported_channel() {
  return *channel;
}

class_<ChannelIdentifier>& Beam::Python::get_exported_channel_identifier() {
  return *channel_identifier;
}

class_<Connection>& Beam::Python::get_exported_connection() {
  return *connection;
}

class_<Reader>& Beam::Python::get_exported_reader() {
  return *reader;
}

class_<ServerConnection>& Beam::Python::get_exported_server_connection() {
  return *server_connection;
}

class_<Writer>& Beam::Python::get_exported_writer() {
  return *writer;
}

const object& Beam::Python::get_io_exception() {
  return io_exception;
}

void Beam::Python::export_async_writer(module& module) {
  export_writer<ToPythonWriter<AsyncWriter<Writer>>>(module, "AsyncWriter").
    def(pybind11::init<Writer>());
}

void Beam::Python::export_basic_channel(module& module) {
  using Channel = BasicChannel<ChannelIdentifier, Connection, Reader, Writer>;
  export_channel<ToPythonChannel<Channel>>(module, "BasicChannel").
    def(pybind11::init([] (ChannelIdentifier identifier, Connection connection,
        Reader reader, Writer writer) {
      return std::make_unique<ToPythonChannel<Channel>>(std::move(identifier),
        std::move(connection), std::move(reader), std::move(writer));
    }));
}

void Beam::Python::export_buffer_reader(module& module) {
  struct PythonBufferReader : BufferReader<BufferRef> {
    std::unique_ptr<BufferRef> m_buffer;

    PythonBufferReader(std::unique_ptr<BufferRef> buffer)
      : BufferReader<BufferRef>(*buffer),
        m_buffer(std::move(buffer)) {}
  };
  export_reader<ToPythonReader<PythonBufferReader>>(module, "BufferReader").
    def(pybind11::init([] (BufferRef buffer) {
      return std::make_unique<ToPythonReader<PythonBufferReader>>(
        std::make_unique<BufferRef>(buffer));
    }), keep_alive<1, 2>());
}

void Beam::Python::export_buffer_ref(module& module) {
  buffer_ref = std::make_unique<class_<BufferRef>>(
    export_buffer<BufferRef>(module, "Buffer"));
}

void Beam::Python::export_buffer_writer(module& module) {
  struct PythonBufferWriter : BufferWriter<BufferRef> {
    std::unique_ptr<BufferRef> m_buffer;

    PythonBufferWriter(std::unique_ptr<BufferRef> buffer)
      : BufferWriter<BufferRef>(Ref(*buffer)),
        m_buffer(std::move(buffer)) {}
  };
  export_writer<ToPythonWriter<PythonBufferWriter>>(module, "BufferWriter").
    def(pybind11::init([] (BufferRef buffer) {
      return std::make_unique<ToPythonWriter<PythonBufferWriter>>(
        std::make_unique<BufferRef>(buffer));
    }), keep_alive<1, 2>());
}

void Beam::Python::export_local_client_channel(module& module) {
  export_channel<ToPythonChannel<LocalClientChannel>>(
    module, "LocalClientChannel").
    def(pybind11::init(
      [] (const std::string& name, LocalServerConnection& server) {
        return std::make_unique<ToPythonChannel<LocalClientChannel>>(
          name, server);
      }), keep_alive<1, 3>());
}

void Beam::Python::export_local_connection(module& module) {
  export_connection<ToPythonConnection<LocalConnection>>(
    module, "LocalConnection");
}

void Beam::Python::export_local_server_channel(module& module) {
  export_channel<ToPythonChannel<LocalServerChannel>>(
    module, "LocalServerChannel");
}

void Beam::Python::export_local_server_connection(module& module) {
  export_server_connection<ToPythonServerConnection<LocalServerConnection>>(
    module, "LocalServerConnection").
    def(pybind11::init());
}

void Beam::Python::export_named_channel_identifier(module& module) {
  export_channel_identifier<NamedChannelIdentifier>(
    module, "NamedChannelIdentifier").
    def(pybind11::init()).
    def(pybind11::init<std::string>()).
    def_property_readonly("name", &NamedChannelIdentifier::get_name);
}

void Beam::Python::export_null_channel(module& module) {
  export_channel<ToPythonChannel<NullChannel>>(module, "NullChannel").
    def(pybind11::init()).
    def(pybind11::init([] (NamedChannelIdentifier identifier) {
      return std::make_unique<ToPythonChannel<NullChannel>>(
        std::move(identifier));
    }));
}

void Beam::Python::export_null_connection(module& module) {
  export_connection<ToPythonConnection<NullConnection>>(
    module, "NullConnection").
    def(pybind11::init());
}

void Beam::Python::export_null_reader(module& module) {
  export_reader<ToPythonReader<NullReader>>(module, "NullReader").
    def(pybind11::init());
}

void Beam::Python::export_null_writer(module& module) {
  export_writer<ToPythonWriter<NullWriter>>(module, "NullWriter").
    def(pybind11::init());
}

void Beam::Python::export_open_state(module& module) {
  class_<OpenState>(module, "OpenState").
    def(pybind11::init()).
    def("is_open", &OpenState::is_open).
    def("is_closing", &OpenState::is_closing).
    def("is_closed", &OpenState::is_closed).
    def("ensure_open", &OpenState::ensure_open).
    def("set_closing", &OpenState::set_closing).
    def("close", &OpenState::close);
}

void Beam::Python::export_piped_reader(module& module) {
  export_reader<ToPythonReader<PipedReader>>(module, "PipedReader").
    def(pybind11::init());
}

void Beam::Python::export_piped_writer(module& module) {
  export_writer<ToPythonWriter<PipedWriter>>(module, "PipedWriter").
    def(pybind11::init<Ref<PipedReader>>()).
    def("close", [] (ToPythonWriter<PipedWriter>& self) {
      self.get().close();
    }, call_guard<gil_scoped_release>());
}

void Beam::Python::export_queued_reader(module& module) {
  export_reader<ToPythonReader<QueuedReader<Reader>>>(module, "QueuedReader").
    def(pybind11::init<Reader>());
}

void Beam::Python::export_shared_buffer(module& module) {
  export_buffer<SharedBuffer>(module, "SharedBuffer").
    def(pybind11::init()).
    def(pybind11::init<std::size_t>()).
    def(pybind11::init([] (bytes data) {
      auto view = static_cast<std::string_view>(data);
      return SharedBuffer(view.data(), view.size());
    }));
}

void Beam::Python::export_size_declarative_reader(module& module) {
  export_reader<ToPythonReader<SizeDeclarativeReader<Reader>>>(
    module, "SizeDeclarativeReader").
    def(pybind11::init<Reader>());
}

void Beam::Python::export_size_declarative_writer(module& module) {
  export_writer<ToPythonWriter<SizeDeclarativeWriter<Writer>>>(
    module, "SizeDeclarativeWriter").
    def(pybind11::init<Writer>());
}

void Beam::Python::export_stdin_reader(module& module) {
  export_reader<ToPythonReader<BasicIStreamReader<std::istream*>>>(
    module, "StdinReader").
    def(pybind11::init([] {
      return std::make_unique<
        ToPythonReader<BasicIStreamReader<std::istream*>>>(&std::cin);
    }));
}

void Beam::Python::export_stdout_writer(module& module) {
  export_writer<ToPythonWriter<BasicOStreamWriter<std::ostream*>>>(
    module, "StdoutWriter").
    def(pybind11::init([] {
      return std::make_unique<
        ToPythonWriter<BasicOStreamWriter<std::ostream*>>>(&std::cout);
    }));
}

void Beam::Python::export_suffix_buffer(module& module) {
  struct PythonSuffixBuffer : SuffixBuffer<BufferRef> {
    std::unique_ptr<BufferRef> m_buffer;

    PythonSuffixBuffer(std::unique_ptr<BufferRef> buffer, std::size_t offset)
      : SuffixBuffer<BufferRef>(Ref(*buffer), offset),
        m_buffer(std::move(buffer)) {}
  };
  export_buffer<PythonSuffixBuffer>(module, "SuffixBuffer").
    def(pybind11::init([] (BufferRef buffer, std::size_t offset) {
      return PythonSuffixBuffer(std::make_unique<BufferRef>(buffer), offset);
    }), keep_alive<1, 2>());
}

void Beam::Python::export_io(module& module) {
  channel = std::make_unique<class_<Channel>>(
    export_channel<Channel>(module, "Channel"));
  channel_identifier = std::make_unique<class_<ChannelIdentifier>>(
    export_channel_identifier<ChannelIdentifier>(module, "ChannelIdentifier"));
  connection = std::make_unique<class_<Connection>>(
    export_connection<Connection>(module, "Connection"));
  reader =
    std::make_unique<class_<Reader>>(export_reader<Reader>(module, "Reader"));
  server_connection = std::make_unique<class_<ServerConnection>>(
    export_server_connection<ServerConnection>(module, "ServerConnection"));
  writer =
    std::make_unique<class_<Writer>>(export_writer<Writer>(module, "Writer"));
  export_async_writer(module);
  export_basic_channel(module);
  export_buffer_reader(module);
  export_buffer_ref(module);
  export_buffer_writer(module);
  export_local_client_channel(module);
  export_local_connection(module);
  export_local_server_channel(module);
  export_local_server_connection(module);
  export_named_channel_identifier(module);
  export_null_channel(module);
  export_null_connection(module);
  export_null_reader(module);
  export_null_writer(module);
  export_open_state(module);
  export_piped_reader(module);
  export_piped_writer(module);
  export_queued_reader(module);
  export_shared_buffer(module);
  export_size_declarative_reader(module);
  export_size_declarative_writer(module);
  export_stdin_reader(module);
  export_stdout_writer(module);
  export_suffix_buffer(module);
  io_exception = register_exception<IOException>(module, "IOException");
  connect_exception = register_exception<ConnectException>(
    module, "ConnectException", io_exception.ptr());
  register_exception<EndOfFileException>(
    module, "EndOfFileException", io_exception.ptr());
}
