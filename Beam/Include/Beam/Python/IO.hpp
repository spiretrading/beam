#ifndef BEAM_PYTHON_IO_HPP
#define BEAM_PYTHON_IO_HPP
#include <pybind11/pybind11.h>
#include "Beam/IO/BufferBox.hpp"
#include "Beam/IO/BufferView.hpp"
#include "Beam/IO/ChannelBox.hpp"
#include "Beam/IO/ChannelIdentifierBox.hpp"
#include "Beam/IO/ConnectionBox.hpp"
#include "Beam/IO/ReaderBox.hpp"
#include "Beam/IO/ServerConnectionBox.hpp"
#include "Beam/IO/WriterBox.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Python {

  /** Returns the exported ChannelBox. */
  BEAM_EXPORT_DLL pybind11::class_<IO::ChannelBox>& GetExportedChannelBox();

  /** Returns the exported ChannelIdentifierBox. */
  BEAM_EXPORT_DLL pybind11::class_<IO::ChannelIdentifierBox>&
    GetExportedChannelIdentifierBox();

  /** Returns the exported ConnectionBox. */
  BEAM_EXPORT_DLL pybind11::class_<IO::ConnectionBox>&
    GetExportedConnectionBox();

  /** Returns the exported ReaderBox. */
  BEAM_EXPORT_DLL pybind11::class_<IO::ReaderBox>& GetExportedReaderBox();

  /** Returns the exported ServerConnectionBox. */
  BEAM_EXPORT_DLL pybind11::class_<IO::ServerConnectionBox>&
    GetExportedServerConnectionBox();

  /** Returns the exported WriterBox. */
  BEAM_EXPORT_DLL pybind11::class_<IO::WriterBox>& GetExportedWriterBox();

  /** Returns the Python object representing an IOException. */
  BEAM_EXPORT_DLL const pybind11::object& GetIOException();

  /**
   * Exports the BufferBox class.
   * @param module The module to export to.
   */
  void ExportBufferBox(pybind11::module& module);

  /**
   * Exports the BufferView class.
   * @param module The module to export to.
   */
  void ExportBufferView(pybind11::module& module);

  /**
   * Exports the IO namespace.
   * @param module The module to export to.
   */
  void ExportIO(pybind11::module& module);

  /**
   * Exports the OpenState class.
   * @param module The module to export to.
   */
  void ExportOpenState(pybind11::module& module);

  /**
   * Exports the SharedBuffer class.
   * @param module The module to export to.
   */
  void ExportSharedBuffer(pybind11::module& module);

  /**
   * Exports a Channel class.
   * @param <Channel> The type of Channel to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported Channel.
   */
  template<typename Channel>
  auto ExportChannel(pybind11::module& module, const std::string& name) {
    auto channel = pybind11::class_<Channel>(module, name.c_str())
      .def_property_readonly("identifier", &Channel::GetIdentifier,
        pybind11::return_value_policy::reference_internal)
      .def_property_readonly("connection", &Channel::GetConnection,
        pybind11::return_value_policy::reference_internal)
      .def_property_readonly("reader", &Channel::GetReader,
        pybind11::return_value_policy::reference_internal)
      .def_property_readonly("writer", &Channel::GetWriter,
        pybind11::return_value_policy::reference_internal);
    if constexpr(!std::is_same_v<Channel, IO::ChannelBox>) {
      pybind11::implicitly_convertible<Channel, IO::ChannelBox>();
      GetExportedChannelBox().def(pybind11::init<Channel>());
    }
    return channel;
  }

  /**
   * Exports a ChannelIdentifier class.
   * @param <ChannelIdentifier> The type of ChannelIdentifier to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported ChannelIdentifier.
   */
  template<typename ChannelIdentifier>
  auto ExportChannelIdentifier(pybind11::module& module,
      const std::string& name) {
    auto identifier = pybind11::class_<ChannelIdentifier>(module, name.c_str())
      .def("__str__", &boost::lexical_cast<std::string, ChannelIdentifier>);
    if constexpr(!std::is_same_v<ChannelIdentifier, IO::ChannelIdentifierBox>) {
      pybind11::implicitly_convertible<ChannelIdentifier,
        IO::ChannelIdentifierBox>();
      GetExportedChannelIdentifierBox().def(
        pybind11::init<ChannelIdentifier>());
    }
    return identifier;
  }

  /**
   * Exports a Connection class.
   * @param <Connection> The type of Connection to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported Connection.
   */
  template<typename Connection>
  auto ExportConnection(pybind11::module& module, const std::string& name) {
    auto connection = pybind11::class_<Connection>(module, name.c_str())
      .def("close", &Connection::Close);
    if constexpr(!std::is_same_v<Connection, IO::ConnectionBox>) {
      pybind11::implicitly_convertible<Connection, IO::ConnectionBox>();
      GetExportedConnectionBox().def(pybind11::init<Connection>());
    }
    return connection;
  }

  /**
   * Exports a Reader class.
   * @param <Reader> The type of Reader to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported Reader.
   */
  template<typename Reader>
  auto ExportReader(pybind11::module& module, const std::string& name) {
    auto reader = pybind11::class_<Reader>(module, name.c_str())
      .def("is_data_available", &Reader::IsDataAvailable)
      .def("read", static_cast<std::size_t (Reader::*)(Out<IO::BufferBox>)>(
        &Reader::Read))
      .def("read", static_cast<std::size_t (Reader::*)(
        Out<IO::BufferBox>, std::size_t)>(&Reader::Read));
    if constexpr(!std::is_same_v<Reader, IO::ReaderBox>) {
      pybind11::implicitly_convertible<Reader, IO::ReaderBox>();
      GetExportedReaderBox().def(pybind11::init<Reader>());
    }
    return reader;
  }

  /**
   * Exports a ServerConnection class.
   * @param <ServerConnection> The type of ServerConnection to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported ServerConnection.
   */
  template<typename ServerConnection>
  auto ExportServerConnection(pybind11::module& module,
      const std::string& name) {
    auto connection = pybind11::class_<ServerConnection>(module, name.c_str())
      .def("accept", &ServerConnection::Accept)
      .def("close", &ServerConnection::Close);
    if constexpr(!std::is_same_v<ServerConnection, IO::ServerConnectionBox>) {
      pybind11::implicitly_convertible<ServerConnection,
        IO::ServerConnectionBox>();
      pybind11::implicitly_convertible<ServerConnection, IO::ConnectionBox>();
      GetExportedServerConnectionBox().def(pybind11::init<ServerConnection>());
      GetExportedConnectionBox().def(pybind11::init<ServerConnection>());
    }
    return connection;
  }

  /**
   * Exports a Writer class.
   * @param <Writer> The type of Writer to export.
   * @param module The module to export to.
   * @param name The name of the class.
   * @return The exported Writer.
   */
  template<typename Writer>
  auto ExportWriter(pybind11::module& module, const std::string& name) {
    auto writer = pybind11::class_<Writer>(module, name.c_str())
      .def("write", static_cast<void (Writer::*)(const IO::BufferView&)>(
        &Writer::Write));
    if constexpr(!std::is_same_v<Writer, IO::WriterBox>) {
      pybind11::implicitly_convertible<Writer, IO::WriterBox>();
      GetExportedWriterBox().def(pybind11::init<Writer>());
    }
    return writer;
  }
}

#endif
