#ifndef BEAM_PYTHON_IO_HPP
#define BEAM_PYTHON_IO_HPP
#include <string_view>
#include <type_traits>
#include <boost/lexical_cast.hpp>
#include <pybind11/pybind11.h>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/BufferRef.hpp"
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/Connection.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/ServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Python/Out.hpp"
#include "Beam/Python/Ref.hpp"
#include "Beam/Python/Utilities.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Python {

  /** Returns the Python object representing an IOException. */
  BEAM_EXPORT_DLL const pybind11::object& get_connect_exception();

  /** Returns the exported BufferRef class. */
  BEAM_EXPORT_DLL pybind11::class_<BufferRef>& get_exported_buffer_ref();

  /** Returns the exported Channel class. */
  BEAM_EXPORT_DLL pybind11::class_<Channel>& get_exported_channel();

  /** Returns the exported ChannelIdentifier class. */
  BEAM_EXPORT_DLL pybind11::class_<ChannelIdentifier>&
    get_exported_channel_identifier();

  /** Returns the exported Connection class. */
  BEAM_EXPORT_DLL pybind11::class_<Connection>& get_exported_connection();

  /** Returns the exported Reader class. */
  BEAM_EXPORT_DLL pybind11::class_<Reader>& get_exported_reader();

  /** Returns the exported ServerConnection class. */
  BEAM_EXPORT_DLL pybind11::class_<ServerConnection>&
    get_exported_server_connection();

  /** Returns the exported Writer class. */
  BEAM_EXPORT_DLL pybind11::class_<Writer>& get_exported_writer();

  /** Returns the Python object representing an IOException. */
  BEAM_EXPORT_DLL const pybind11::object& get_io_exception();

  /**
   * Exports the AsyncWriter class.
   * @param module The module to export to.
   */
  void export_async_writer(pybind11::module& module);

  /**
   * Exports the BasicChannel class.
   * @param module The module to export to.
   */
  void export_basic_channel(pybind11::module& module);

  /**
   * Exports a Buffer type and related free functions.
   * @param module The module to export to.
   * @param name The name of the class to export.
   */
  template<IsBuffer B>
  auto export_buffer(pybind11::module& module, std::string_view name) {
    auto buffer_class = pybind11::class_<B>(module, name.data()).
      def("__len__", &B::get_size).
      def("__bool__", [] (const B& self) {
        return !is_empty(self);
      }).
      def_property_readonly("size", &B::get_size).
      def("get_data", [] (const B& self) {
        return pybind11::bytes(self.get_data(), self.get_size());
      }).
      def("is_empty", [] (const B& self) {
        return is_empty(self);
      }).
      def("grow", &B::grow).
      def("shrink", &B::shrink).
      def("write", [] (B& self, std::size_t index, pybind11::bytes data) {
        auto str = std::string(data);
        self.write(index, str.data(), str.size());
      }).
      def("append", [] (B& self, pybind11::bytes data) {
        auto str = std::string(data);
        append(self, str.data(), str.size());
      }).
      def("append", [] (B& destination, const B& source) {
        append(destination, source);
      }).
      def("reset", [] (B& self) {
        reset(self);
      }).
      def("reserve", [] (B& self, std::size_t size) {
        return reserve(self, size);
      }).
      def("__eq__", [] (const B& lhs, const B& rhs) {
        return lhs == rhs;
      }).
      def("__eq__", [] (const B& lhs, std::string_view rhs) {
        return lhs == rhs;
      }).
      def("__ne__", [] (const B& lhs, const B& rhs) {
        return lhs != rhs;
      }).
      def("__ne__", [] (const B& lhs, std::string_view rhs) {
        return lhs != rhs;
      }).
      def("__str__", &boost::lexical_cast<std::string, B>).
      def("__repr__", [name = std::string(name)] (const B& self) {
        return "<" + name + " size=" + std::to_string(self.get_size()) + ">";
      });
    module.def("encode_base64", [] (const B& source) {
      return encode_base64(source);
    });
    module.def("decode_base64", [] (std::string_view source, B& buffer) {
      decode_base64(source, out(buffer));
    });
    if constexpr(!std::is_same_v<B, BufferRef>) {
      buffer_class.def("append", [] (B& destination, BufferRef source) {
        append(destination, source);
      }).
      def("__eq__", [] (const B& lhs, BufferRef rhs) {
        return lhs == rhs;
      }).
      def("__ne__", [] (const B& lhs, BufferRef rhs) {
        return lhs != rhs;
      });
      get_exported_buffer_ref().def(
        pybind11::init<B&>(), pybind11::keep_alive<1, 2>());
      pybind11::implicitly_convertible<B, BufferRef>();
    }
    return buffer_class;
  }

  /**
   * Exports the BufferReader class.
   * @param module The module to export to.
   */
  void export_buffer_reader(pybind11::module& module);

  /**
   * Exports the BufferRef class.
   * @param module The module to export to.
   */
  void export_buffer_ref(pybind11::module& module);

  /**
   * Exports the BufferWriter class.
   * @param module The module to export to.
   */
  void export_buffer_writer(pybind11::module& module);

  /**
   * Exports a Channel type and related free functions.
   * @param module The module to export to.
   * @param name The name of the class to export.
   */
  template<IsChannel C>
  auto export_channel(pybind11::module& module, std::string_view name) {
    auto channel_class = pybind11::class_<C>(module, name.data()).
      def_property_readonly("identifier", &C::get_identifier,
        pybind11::return_value_policy::reference_internal).
      def_property_readonly("connection", &C::get_connection,
        pybind11::return_value_policy::reference_internal).
      def_property_readonly("reader", &C::get_reader,
        pybind11::return_value_policy::reference_internal).
      def_property_readonly("writer", &C::get_writer,
        pybind11::return_value_policy::reference_internal).
      def("__repr__", [name = std::string(name)] (const C& self) {
        return "<" + name + ">";
      });
    if constexpr(!std::is_same_v<C, Channel>) {
      get_exported_channel().def(
        pybind11::init<C*>(), pybind11::keep_alive<1, 2>());
      pybind11::implicitly_convertible<C, Channel>();
    }
    return channel_class;
  }

  /**
   * Exports a ChannelIdentifier type and related free functions.
   * @param module The module to export to.
   * @param name The name of the class to export.
   */
  template<IsChannelIdentifier I>
  auto export_channel_identifier(
      pybind11::module& module, std::string_view name) {
    auto identifier_class = pybind11::class_<I>(module, name.data()).
      def("__repr__", [name = std::string(name)] (const I& self) {
        return "<" + name + " " + boost::lexical_cast<std::string>(self) +
          ">";
      });
    export_default_methods(identifier_class);
    if constexpr(!std::is_same_v<I, ChannelIdentifier>) {
      get_exported_channel_identifier().def(
        pybind11::init<I>(), pybind11::keep_alive<1, 2>());
      pybind11::implicitly_convertible<I, ChannelIdentifier>();
    }
    return identifier_class;
  }

  /**
   * Exports a Connection type and related free functions.
   * @param module The module to export to.
   * @param name The name of the class to export.
   */
  template<IsConnection C>
  auto export_connection(pybind11::module& module, std::string_view name) {
    auto connection_class = pybind11::class_<C>(module, name.data()).
      def("close", &C::close).
      def("__repr__", [name = std::string(name)] (const C& self) {
        return "<" + name + ">";
      });
    if constexpr(!std::is_same_v<C, Connection>) {
      get_exported_connection().def(
        pybind11::init<C*>(), pybind11::keep_alive<1, 2>());
      pybind11::implicitly_convertible<C, Connection>();
    }
    return connection_class;
  }

  /**
   * Exports the IO namespace.
   * @param module The module to export to.
   */
  void export_io(pybind11::module& module);

  /**
   * Exports the LocalClientChannel class.
   * @param module The module to export to.
   */
  void export_local_client_channel(pybind11::module& module);

  /**
   * Exports the LocalConnection class.
   * @param module The module to export to.
   */
  void export_local_connection(pybind11::module& module);

  /**
   * Exports the LocalServerChannel class.
   * @param module The module to export to.
   */
  void export_local_server_channel(pybind11::module& module);

  /**
   * Exports the LocalServerConnection class.
   * @param module The module to export to.
   */
  void export_local_server_connection(pybind11::module& module);

  /**
   * Exports the NamedChannelIdentifier class.
   * @param module The module to export to.
   */
  void export_named_channel_identifier(pybind11::module& module);

  /**
   * Exports the NullChannel class.
   * @param module The module to export to.
   */
  void export_null_channel(pybind11::module& module);

  /**
   * Exports the NullConnection class.
   * @param module The module to export to.
   */
  void export_null_connection(pybind11::module& module);

  /**
   * Exports the NullReader class.
   * @param module The module to export to.
   */
  void export_null_reader(pybind11::module& module);

  /**
   * Exports the NullWriter class.
   * @param module The module to export to.
   */
  void export_null_writer(pybind11::module& module);

  /**
   * Exports the OpenState class.
   * @param module The module to export to.
   */
  void export_open_state(pybind11::module& module);

  /**
   * Exports the PipedReader class.
   * @param module The module to export to.
   */
  void export_piped_reader(pybind11::module& module);

  /**
   * Exports the PipedWriter class.
   * @param module The module to export to.
   */
  void export_piped_writer(pybind11::module& module);

  /**
   * Exports the QueuedReader class.
   * @param module The module to export to.
   */
  void export_queued_reader(pybind11::module& module);

  /**
   * Exports a Reader type and related free functions.
   * @param module The module to export to.
   * @param name The name of the class to export.
   */
  template<IsReader R>
  auto export_reader(pybind11::module& module, std::string_view name) {
    auto reader_class = pybind11::class_<R>(module, name.data()).
      def("poll", &R::poll).
      def("read", &R::template read<BufferRef>, pybind11::arg("destination"),
        pybind11::arg("size") = static_cast<std::size_t>(-1)).
      def("__repr__", [name = std::string(name)] (const R& self) {
        return "<" + name + ">";
      });
    module.def("read_exact", &read_exact<R, BufferRef>);
    if constexpr(!std::is_same_v<R, Reader>) {
      get_exported_reader().def(
        pybind11::init<R*>(), pybind11::keep_alive<1, 2>());
      pybind11::implicitly_convertible<R, Reader>();
    }
    return reader_class;
  }

  /**
   * Exports a ServerConnection type and related free functions.
   * @param module The module to export to.
   * @param name The name of the class to export.
   */
  template<IsServerConnection S>
  auto export_server_connection(
      pybind11::module& module, std::string_view name) {
    auto server_connection_class = pybind11::class_<S>(module, name.data()).
      def("accept", &S::accept).
      def("close", &S::close).
      def("__repr__", [name = std::string(name)] (const S& self) {
        return "<" + name + ">";
      });
    if constexpr(!std::is_same_v<S, ServerConnection>) {
      get_exported_server_connection().def(
        pybind11::init<S*>(), pybind11::keep_alive<1, 2>());
      pybind11::implicitly_convertible<S, ServerConnection>();
      get_exported_connection().def(
        pybind11::init<S*>(), pybind11::keep_alive<1, 2>());
      pybind11::implicitly_convertible<S, Connection>();
    }
    return server_connection_class;
  }

  /**
   * Exports the SharedBuffer class.
   * @param module The module to export to.
   */
  void export_shared_buffer(pybind11::module& module);

  /**
   * Exports the SizeDeclarativeReader class.
   * @param module The module to export to.
   */
  void export_size_declarative_reader(pybind11::module& module);

  /**
   * Exports the SizeDeclarativeWriter class.
   * @param module The module to export to.
   */
  void export_size_declarative_writer(pybind11::module& module);

  /**
   * Exports the StdinReader class.
   * @param module The module to export to.
   */
  void export_stdin_reader(pybind11::module& module);

  /**
   * Exports the StdoutWriter class.
   * @param module The module to export to.
   */
  void export_stdout_writer(pybind11::module& module);

  /**
   * Exports the SuffixBuffer class.
   * @param module The module to export to.
   */
  void export_suffix_buffer(pybind11::module& module);

  /**
   * Exports a Writer type and related free functions.
   * @param module The module to export to.
   * @param name The name of the class to export.
   */
  template<IsWriter W>
  auto export_writer(pybind11::module& module, std::string_view name) {
    auto writer_class = pybind11::class_<W>(module, name.data()).
      def("write", [] (W& self, pybind11::bytes data) {
        auto view = static_cast<std::string_view>(data);
        self.write(SharedBuffer(view.data(), view.size()));
      }).
      def("write", [] (W& self, BufferRef buffer) {
        self.write(buffer);
      }).
      def("__repr__", [name = std::string(name)] (const W& self) {
        return "<" + name + ">";
      });
    if constexpr(!std::is_same_v<W, Writer>) {
      get_exported_writer().def(
        pybind11::init<W*>(), pybind11::keep_alive<1, 2>());
      pybind11::implicitly_convertible<W, Writer>();
    }
    return writer_class;
  }
}

#endif
