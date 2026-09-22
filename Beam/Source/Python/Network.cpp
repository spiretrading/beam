#include "Beam/Python/Network.hpp"
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Network/DatagramPacket.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/MulticastSocket.hpp"
#include "Beam/Network/MulticastSocketChannel.hpp"
#include "Beam/Network/MulticastSocketConnection.hpp"
#include "Beam/Network/MulticastSocketOptions.hpp"
#include "Beam/Network/MulticastSocketReader.hpp"
#include "Beam/Network/MulticastSocketWriter.hpp"
#include "Beam/Network/SecureSocketChannel.hpp"
#include "Beam/Network/SecureSocketConnection.hpp"
#include "Beam/Network/SecureSocketOptions.hpp"
#include "Beam/Network/SecureSocketReader.hpp"
#include "Beam/Network/SecureSocketWriter.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/SocketIdentifier.hpp"
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Network/TcpSocketConnection.hpp"
#include "Beam/Network/TcpSocketOptions.hpp"
#include "Beam/Network/TcpSocketReader.hpp"
#include "Beam/Network/TcpSocketWriter.hpp"
#include "Beam/Network/UdpSocket.hpp"
#include "Beam/Network/UdpSocketChannel.hpp"
#include "Beam/Network/UdpSocketConnection.hpp"
#include "Beam/Network/UdpSocketOptions.hpp"
#include "Beam/Network/UdpSocketReader.hpp"
#include "Beam/Network/UdpSocketReceiver.hpp"
#include "Beam/Network/UdpSocketSender.hpp"
#include "Beam/Network/UdpSocketWriter.hpp"
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/IO.hpp"
#include "Beam/Python/Out.hpp"
#include "Beam/Python/ToPythonChannel.hpp"
#include "Beam/Python/ToPythonConnection.hpp"
#include "Beam/Python/ToPythonReader.hpp"
#include "Beam/Python/ToPythonServerConnection.hpp"
#include "Beam/Python/ToPythonWriter.hpp"
#include "Beam/Python/Utilities.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace pybind11;

void Beam::Python::export_datagram_packet(module& module) {
  auto packet = class_<DatagramPacket<SharedBuffer>>(module, "DatagramPacket").
    def(pybind11::init<SharedBuffer, IpAddress>()).
    def_property("data",
      overload_cast<>(&DatagramPacket<SharedBuffer>::get_data),
      [] (DatagramPacket<SharedBuffer>& self, SharedBuffer buffer) {
        self.get_data() = std::move(buffer);
      }).
    def_property("address",
      overload_cast<>(&DatagramPacket<SharedBuffer>::get_address),
      [] (DatagramPacket<SharedBuffer>& self, IpAddress address) {
        self.get_address() = std::move(address);
      });
  export_default_methods(packet);
}

void Beam::Python::export_ip_address(module& module) {
  auto address = class_<IpAddress>(module, "IpAddress").
    def(pybind11::init<std::string, unsigned short>()).
    def_property_readonly("host", &IpAddress::get_host).
    def_property_readonly("port", &IpAddress::get_port);
  export_default_methods(address);
}

void Beam::Python::export_multicast_socket(module& module) {
  auto export_type = [&] <typename T> (const char* name) {
    class_<T, std::shared_ptr<T>>(module, name).
      def(pybind11::init(&make_python_shared<T, const IpAddress&>),
        call_guard<GilRelease>()).
      def(pybind11::init(&make_python_shared<T, const IpAddress&,
        const MulticastSocketOptions&>), call_guard<GilRelease>()).
      def(pybind11::init(&make_python_shared<T, const IpAddress&,
        const IpAddress&>), call_guard<GilRelease>()).
      def(pybind11::init(&make_python_shared<T, const IpAddress&,
        const IpAddress&, const MulticastSocketOptions&>),
        call_guard<GilRelease>()).
      def_property_readonly("group", &T::get_group).
      def_property_readonly("receiver", &T::get_receiver,
        return_value_policy::reference_internal).
      def_property_readonly("sender", &T::get_sender,
        return_value_policy::reference_internal).
      def("close", &T::close, call_guard<GilRelease>());
  };
  export_type.operator ()<MulticastSocket>("MulticastSocket");
  export_type.operator ()<BufferedMulticastSocket>("BufferedMulticastSocket");
}

void Beam::Python::export_multicast_socket_channel(module& module) {
  auto export_type = [&] <typename T> (const char* name) {
    export_channel<ToPythonChannel<T>>(module, name).
      def(pybind11::init<const IpAddress&>()).
      def(pybind11::init<const IpAddress&, const MulticastSocketOptions&>()).
      def(pybind11::init<const IpAddress&, const IpAddress&>()).
      def(pybind11::init<
        const IpAddress&, const IpAddress&, const MulticastSocketOptions&>());
  };
  export_type.operator ()<MulticastSocketChannel>("MulticastSocketChannel");
  export_type.operator ()<BufferedMulticastSocketChannel>(
    "BufferedMulticastSocketChannel");
}

void Beam::Python::export_multicast_socket_connection(module& module) {
  export_connection<ToPythonConnection<MulticastSocketConnection>>(
    module, "MulticastSocketConnection");
  export_connection<ToPythonConnection<BufferedMulticastSocketConnection>>(
    module, "BufferedMulticastSocketConnection");
}

void Beam::Python::export_multicast_socket_options(module& module) {
  class_<MulticastSocketOptions, UdpSocketOptions>(
    module, "MulticastSocketOptions").
    def(pybind11::init()).
    def(pybind11::init<const MulticastSocketOptions&>());
}

void Beam::Python::export_multicast_socket_reader(module& module) {
  export_reader<ToPythonReader<MulticastSocketReader>>(
    module, "MulticastSocketReader");
  export_reader<ToPythonReader<BufferedMulticastSocketReader>>(
    module, "BufferedMulticastSocketReader");
}

void Beam::Python::export_multicast_socket_writer(module& module) {
  export_writer<ToPythonWriter<MulticastSocketWriter>>(
    module, "MulticastSocketWriter");
  export_writer<ToPythonWriter<BufferedMulticastSocketWriter>>(
    module, "BufferedMulticastSocketWriter");
}

void Beam::Python::export_network(module& module) {
  export_datagram_packet(module);
  export_ip_address(module);
  export_udp_socket(module);
  export_udp_socket_channel(module);
  export_udp_socket_connection(module);
  export_udp_socket_options(module);
  export_udp_socket_reader(module);
  export_udp_socket_receiver(module);
  export_udp_socket_sender(module);
  export_udp_socket_writer(module);
  export_multicast_socket(module);
  export_multicast_socket_channel(module);
  export_multicast_socket_connection(module);
  export_multicast_socket_options(module);
  export_multicast_socket_reader(module);
  export_multicast_socket_writer(module);
  export_tcp_server_socket(module);
  export_tcp_socket_channel(module);
  export_tcp_socket_connection(module);
  export_tcp_socket_options(module);
  export_tcp_socket_reader(module);
  export_tcp_socket_writer(module);
  export_secure_socket_channel(module);
  export_secure_socket_connection(module);
  export_secure_socket_options(module);
  export_secure_socket_reader(module);
  export_secure_socket_writer(module);
  export_socket_identifier(module);
  register_exception<SocketException>(
    module, "SocketException", get_io_exception().ptr());
}

void Beam::Python::export_secure_socket_channel(module& module) {
  export_channel<ToPythonChannel<SecureSocketChannel>>(
    module, "SecureSocketChannel").
    def(pybind11::init<const IpAddress&>()).
    def(pybind11::init<const IpAddress&, const SecureSocketOptions&>()).
    def(pybind11::init<const IpAddress&, const IpAddress&>()).
    def(pybind11::init<
      const IpAddress&, const IpAddress&, const SecureSocketOptions&>()).
    def(pybind11::init<const std::vector<IpAddress>&>()).
    def(pybind11::init<
      const std::vector<IpAddress>&, const SecureSocketOptions&>()).
    def(pybind11::init<const std::vector<IpAddress>&, const IpAddress&>()).
    def(pybind11::init<const std::vector<IpAddress>&, const IpAddress&,
      const SecureSocketOptions&>());
}

void Beam::Python::export_secure_socket_connection(module& module) {
  export_connection<ToPythonConnection<SecureSocketConnection>>(
    module, "SecureSocketConnection");
}

void Beam::Python::export_secure_socket_options(module& module) {
  auto options = class_<SecureSocketOptions, TcpSocketOptions>(
    module, "SecureSocketOptions");
  export_default_methods(options);
}

void Beam::Python::export_secure_socket_reader(module& module) {
  export_reader<ToPythonReader<SecureSocketReader>>(
    module, "SecureSocketReader");
}

void Beam::Python::export_secure_socket_writer(module& module) {
  export_writer<ToPythonWriter<SecureSocketWriter>>(
    module, "SecureSocketWriter");
}

void Beam::Python::export_socket_identifier(module& module) {
  export_channel_identifier<SocketIdentifier>(module, "SocketIdentifier").
    def(pybind11::init<const IpAddress&>()).
    def_property_readonly("address", &SocketIdentifier::get_address);
}

void Beam::Python::export_tcp_server_socket(module& module) {
  export_server_connection<ToPythonServerConnection<TcpServerSocket>>(
    module, "TcpServerSocket").
    def(pybind11::init()).
    def(pybind11::init<const TcpSocketOptions&>()).
    def(pybind11::init<const IpAddress&>()).
    def(pybind11::init<const IpAddress&, const TcpSocketOptions&>());
}

void Beam::Python::export_tcp_socket_channel(module& module) {
  export_channel<ToPythonChannel<TcpSocketChannel>>(
    module, "TcpSocketChannel").
    def(pybind11::init<const IpAddress&>()).
    def(pybind11::init<const IpAddress&, const TcpSocketOptions&>()).
    def(pybind11::init<const IpAddress&, const IpAddress&>()).
    def(pybind11::init<
      const IpAddress&, const IpAddress&, const TcpSocketOptions&>()).
    def(pybind11::init<const std::vector<IpAddress>&>()).
    def(pybind11::init<
      const std::vector<IpAddress>&, const TcpSocketOptions&>()).
    def(pybind11::init<const std::vector<IpAddress>&, const IpAddress&>()).
    def(pybind11::init<const std::vector<IpAddress>&, const IpAddress&,
      const TcpSocketOptions&>());
}

void Beam::Python::export_tcp_socket_connection(module& module) {
  export_connection<ToPythonConnection<TcpSocketConnection>>(
    module, "TcpSocketConnection");
}

void Beam::Python::export_tcp_socket_options(module& module) {
  auto options = class_<TcpSocketOptions>(module, "TcpSocketOptions").
    def_readwrite("no_delay_enabled", &TcpSocketOptions::m_no_delay_enabled).
    def_readwrite("write_buffer_size", &TcpSocketOptions::m_write_buffer_size);
  export_default_methods(options);
}

void Beam::Python::export_tcp_socket_reader(module& module) {
  export_reader<ToPythonReader<TcpSocketReader>>(module, "TcpSocketReader");
}

void Beam::Python::export_tcp_socket_writer(module& module) {
  export_writer<ToPythonWriter<TcpSocketWriter>>(module, "TcpSocketWriter");
}

void Beam::Python::export_udp_socket(module& module) {
  auto export_type = [&] <typename T> (const char* name) {
    class_<T, std::shared_ptr<T>>(module, name).
      def(pybind11::init(&make_python_shared<T, const IpAddress&>),
        call_guard<GilRelease>()).
      def(pybind11::init(&make_python_shared<T, const IpAddress&,
        const UdpSocketOptions&>), call_guard<GilRelease>()).
      def(pybind11::init(&make_python_shared<T, const IpAddress&,
        const IpAddress&>), call_guard<GilRelease>()).
      def(pybind11::init(&make_python_shared<T, const IpAddress&,
        const IpAddress&, const UdpSocketOptions&>), call_guard<GilRelease>()).
      def_property_readonly("address", &T::get_address).
      def_property_readonly("receiver", &T::get_receiver,
        return_value_policy::reference_internal).
      def_property_readonly("sender", &T::get_sender,
        return_value_policy::reference_internal).
      def("close", &T::close, call_guard<GilRelease>());
  };
  export_type.operator ()<UdpSocket>("UdpSocket");
  export_type.operator ()<BufferedUdpSocket>("BufferedUdpSocket");
}

void Beam::Python::export_udp_socket_channel(module& module) {
  auto export_type = [&] <typename T> (const char* name) {
    export_channel<ToPythonChannel<T>>(module, name).
      def(pybind11::init<const IpAddress&>()).
      def(pybind11::init<const IpAddress&, const UdpSocketOptions&>()).
      def(pybind11::init<const IpAddress&, const IpAddress&>()).
      def(pybind11::init<
        const IpAddress&, const IpAddress&, const UdpSocketOptions&>());
  };
  export_type.operator ()<UdpSocketChannel>("UdpSocketChannel");
  export_type.operator ()<BufferedUdpSocketChannel>("BufferedUdpSocketChannel");
}

void Beam::Python::export_udp_socket_connection(module& module) {
  export_connection<ToPythonConnection<UdpSocketConnection>>(
    module, "UdpSocketConnection");
  export_connection<ToPythonConnection<BufferedUdpSocketConnection>>(
    module, "BufferedUdpSocketConnection");
}

void Beam::Python::export_udp_socket_options(module& module) {
  auto options = class_<UdpSocketOptions>(module, "UdpSocketOptions").
    def_readwrite("timeout", &UdpSocketOptions::m_timeout).
    def_readwrite("ttl", &UdpSocketOptions::m_ttl).
    def_readwrite("enable_loopback", &UdpSocketOptions::m_enable_loopback).
    def_readwrite("max_datagram_size", &UdpSocketOptions::m_max_datagram_size).
    def_readwrite(
      "receive_buffer_size", &UdpSocketOptions::m_receive_buffer_size);
  export_default_methods(options);
}

void Beam::Python::export_udp_socket_reader(module& module) {
  export_reader<ToPythonReader<UdpSocketReader>>(
    module, "UdpSocketReader");
  export_reader<ToPythonReader<BufferedUdpSocketReader>>(
    module, "BufferedUdpSocketReader");
}

void Beam::Python::export_udp_socket_receiver(module& module) {
  auto export_type = [&] <IsUdpSocketReceiver T> (const char* name) {
    class_<T>(module, name).
      def("poll", &T::poll, call_guard<GilRelease>()).
      def("receive", [] (T& self, Out<DatagramPacket<SharedBuffer>> packet) {
        return self.receive(out(packet));
      }, call_guard<GilRelease>()).
      def("receive", [] (T& self,
          Out<DatagramPacket<SharedBuffer>> packet, std::size_t size) {
        return self.receive(out(packet), size);
      }, call_guard<GilRelease>()).
      def("receive", [] (T& self, Out<SharedBuffer> buffer,
          Out<IpAddress> address) {
        return self.receive(out(buffer), -1, out(address));
      }, call_guard<GilRelease>()).
      def("receive", [] (T& self, Out<SharedBuffer> buffer,
          std::size_t size, Out<IpAddress> address) {
        return self.receive(out(buffer), size, out(address));
      }, call_guard<GilRelease>()).
      def("receive", [] (T& self, Out<SharedBuffer> buffer,
          std::size_t size) {
        return self.receive(out(buffer), size);
      }, call_guard<GilRelease>());
  };
  export_type.operator ()<UdpSocketReceiver>("UdpSocketReceiver");
  export_type.operator ()<BufferedUdpSocketReceiver>(
    "BufferedUdpSocketReceiver");
}

void Beam::Python::export_udp_socket_sender(module& module) {
  class_<UdpSocketSender>(module, "UdpSocketSender").
    def("send", overload_cast<const DatagramPacket<SharedBuffer>&>(
      &UdpSocketSender::send<SharedBuffer>), call_guard<GilRelease>());
}

void Beam::Python::export_udp_socket_writer(module& module) {
  export_writer<ToPythonWriter<UdpSocketWriter>>(
    module, "UdpSocketWriter");
  export_writer<ToPythonWriter<BufferedUdpSocketWriter>>(
    module, "BufferedUdpSocketWriter");
}
