#include "Beam/Python/Network.hpp"
#include <boost/lexical_cast.hpp>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include "Beam/Network/MulticastSocketChannel.hpp"
#include "Beam/Network/SecureSocketChannel.hpp"
#include "Beam/Network/TcpServerSocket.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Network/UdpSocketChannel.hpp"
#include "Beam/Python/Beam.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace boost;
using namespace pybind11;

void Beam::Python::ExportDatagramPacket(pybind11::module& module) {
  class_<DatagramPacket<SharedBuffer>>(module, "DatagramPacket").
    def(init()).
    def(init<BufferBox, IpAddress>()).
    def_property("data",
      [] (DatagramPacket<SharedBuffer>& self) {
        return self.GetData();
      },
      [] (DatagramPacket<SharedBuffer>& self, BufferBox buffer) {
        self.GetData() = std::move(buffer);
      }).
    def_property("address",
      [] (DatagramPacket<SharedBuffer>& self) {
        return self.GetAddress();
      },
      [] (DatagramPacket<SharedBuffer>& self, IpAddress address) {
        self.GetAddress() = std::move(address);
      }).
    def("__str__", &lexical_cast<std::string, DatagramPacket<SharedBuffer>>);
}

void Beam::Python::ExportIpAddress(pybind11::module& module) {
  class_<IpAddress>(module, "IpAddress").
    def(init()).
    def(init<std::string, unsigned short>()).
    def(init<const IpAddress&>()).
    def("__str__", &lexical_cast<std::string, IpAddress>).
    def_property_readonly("host", &IpAddress::GetHost).
    def_property_readonly("port", &IpAddress::GetPort).
    def(self == self).
    def(self != self);
}

void Beam::Python::ExportMulticastSocket(pybind11::module& module) {
  class_<MulticastSocket>(module, "MulticastSocket").
    def(init<const IpAddress&>(), call_guard<GilRelease>()).
    def(init<const IpAddress&, const MulticastSocketOptions&>(),
      call_guard<GilRelease>()).
    def(init<const IpAddress&, const IpAddress&>(), call_guard<GilRelease>()).
    def(init<const IpAddress&, const IpAddress&,
      const MulticastSocketOptions&>(), call_guard<GilRelease>()).
    def("__del__",
      [] (MulticastSocket& self) {
        self.Close();
      }, call_guard<GilRelease>()).
    def_property_readonly("group", &MulticastSocket::GetGroup).
    def_property_readonly("receiver", &MulticastSocket::GetReceiver,
      return_value_policy::reference_internal).
    def_property_readonly("sender", &MulticastSocket::GetSender,
      return_value_policy::reference_internal).
    def("close", &MulticastSocket::Close, call_guard<GilRelease>());
}

void Beam::Python::ExportMulticastSocketChannel(pybind11::module& module) {
  ExportChannel<ToPythonChannel<MulticastSocketChannel>>(module,
    "MulticastSocketChannel").
    def(init<const IpAddress&>()).
    def(init<const IpAddress&, const MulticastSocketOptions&>()).
    def(init<const IpAddress&, const IpAddress&>()).
    def(init<const IpAddress&, const IpAddress&,
      const MulticastSocketOptions&>());
}

void Beam::Python::ExportMulticastSocketConnection(pybind11::module& module) {
  ExportConnection<ToPythonConnection<MulticastSocketConnection>>(module,
    "MulticastSocketConnection");
}

void Beam::Python::ExportMulticastSocketOptions(pybind11::module& module) {
  class_<MulticastSocketOptions, UdpSocketOptions>(module,
      "MulticastSocketOptions").
    def(init()).
    def(init<const MulticastSocketOptions&>());
}

void Beam::Python::ExportMulticastSocketReader(pybind11::module& module) {
  ExportReader<ToPythonReader<MulticastSocketReader>>(module,
    "MulticastSocketReader");
}

void Beam::Python::ExportMulticastSocketWriter(pybind11::module& module) {
  ExportWriter<ToPythonWriter<MulticastSocketWriter>>(module,
    "MulticastSocketWriter");
}

void Beam::Python::ExportNetwork(pybind11::module& module) {
  auto submodule = module.def_submodule("network");
  ExportDatagramPacket(submodule);
  ExportIpAddress(submodule);
  ExportTcpServerSocket(submodule);
  ExportTcpSocketChannel(submodule);
  ExportTcpSocketConnection(submodule);
  ExportTcpSocketOptions(submodule);
  ExportTcpSocketReader(submodule);
  ExportTcpSocketWriter(submodule);
  ExportSecureSocketChannel(submodule);
  ExportSecureSocketConnection(submodule);
  ExportSecureSocketOptions(submodule);
  ExportSecureSocketReader(submodule);
  ExportSecureSocketWriter(submodule);
  ExportSocketIdentifier(submodule);
  ExportUdpSocket(submodule);
  ExportUdpSocketChannel(submodule);
  ExportUdpSocketConnection(submodule);
  ExportUdpSocketOptions(submodule);
  ExportUdpSocketReader(submodule);
  ExportUdpSocketReceiver(submodule);
  ExportUdpSocketSender(submodule);
  ExportUdpSocketWriter(submodule);
  ExportMulticastSocket(submodule);
  ExportMulticastSocketChannel(submodule);
  ExportMulticastSocketConnection(submodule);
  ExportMulticastSocketOptions(submodule);
  ExportMulticastSocketReader(submodule);
  ExportMulticastSocketWriter(submodule);
  register_exception<SocketException>(submodule, "SocketException",
    GetIOException().ptr());
}

void Beam::Python::ExportSecureSocketChannel(pybind11::module& module) {
  ExportChannel<ToPythonChannel<SecureSocketChannel>>(module,
    "SecureSocketChannel").
    def(init<const IpAddress&>()).
    def(init<const IpAddress&, const SecureSocketOptions&>()).
    def(init<const IpAddress&, const IpAddress&>()).
    def(init<const IpAddress&, const IpAddress&, const SecureSocketOptions&>()).
    def(init<const std::vector<IpAddress>&>()).
    def(init<const std::vector<IpAddress>&, const SecureSocketOptions&>()).
    def(init<const std::vector<IpAddress>&, const IpAddress&>()).
    def(init<const std::vector<IpAddress>&, const IpAddress&,
      const SecureSocketOptions&>());
}

void Beam::Python::ExportSecureSocketConnection(pybind11::module& module) {
  ExportConnection<ToPythonConnection<SecureSocketConnection>>(module,
    "SecureSocketConnection");
}

void Beam::Python::ExportSecureSocketOptions(pybind11::module& module) {
  class_<SecureSocketOptions, TcpSocketOptions>(module, "SecureSocketOptions").
    def(init()).
    def(init<const SecureSocketOptions&>());
}

void Beam::Python::ExportSecureSocketReader(pybind11::module& module) {
  ExportReader<ToPythonReader<SecureSocketReader>>(module,
    "SecureSocketReader");
}

void Beam::Python::ExportSecureSocketWriter(pybind11::module& module) {
  ExportWriter<ToPythonWriter<SecureSocketWriter>>(module,
    "SecureSocketWriter");
}

void Beam::Python::ExportSocketIdentifier(pybind11::module& module) {
  ExportChannelIdentifier<SocketIdentifier>(module, "SocketIdentifier").
    def(init()).
    def(init<const IpAddress&>()).
    def_property_readonly("address", &SocketIdentifier::GetAddress);
}

void Beam::Python::ExportTcpServerSocket(pybind11::module& module) {
  ExportServerConnection<ToPythonServerConnection<TcpServerSocket>>(module,
    "TcpServerSocket").
    def(init()).
    def(init<const TcpSocketOptions&>()).
    def(init<const IpAddress&>()).
    def(init<const IpAddress&, const TcpSocketOptions&>());
}

void Beam::Python::ExportTcpSocketChannel(pybind11::module& module) {
  ExportChannel<ToPythonChannel<TcpSocketChannel>>(module, "TcpSocketChannel").
    def(init<const IpAddress&>()).
    def(init<const IpAddress&, const TcpSocketOptions&>()).
    def(init<const IpAddress&, const IpAddress&>()).
    def(init<const IpAddress&, const IpAddress&>()).
    def(init<const std::vector<IpAddress>&>()).
    def(init<const std::vector<IpAddress>&, const TcpSocketOptions&>()).
    def(init<const std::vector<IpAddress>&, const IpAddress&>()).
    def(init<const std::vector<IpAddress>&, const IpAddress&,
      const TcpSocketOptions&>());
}

void Beam::Python::ExportTcpSocketConnection(pybind11::module& module) {
  ExportConnection<ToPythonConnection<TcpSocketConnection>>(module,
    "TcpSocketConnection");
}

void Beam::Python::ExportTcpSocketOptions(pybind11::module& module) {
  class_<TcpSocketOptions>(module, "TcpSocketOptions").
    def(init()).
    def(init<const TcpSocketOptions&>()).
    def_readwrite("no_delay_enabled", &TcpSocketOptions::m_noDelayEnabled).
    def_readwrite("write_buffer_size", &TcpSocketOptions::m_writeBufferSize);
}

void Beam::Python::ExportTcpSocketReader(pybind11::module& module) {
  ExportReader<ToPythonReader<TcpSocketReader>>(module, "TcpSocketReader");
}

void Beam::Python::ExportTcpSocketWriter(pybind11::module& module) {
  ExportWriter<ToPythonWriter<TcpSocketWriter>>(module, "TcpSocketWriter");
}

void Beam::Python::ExportUdpSocket(pybind11::module& module) {
  class_<UdpSocket>(module, "UdpSocket").
    def(init<const IpAddress&>(), call_guard<GilRelease>()).
    def(init<const IpAddress&, const UdpSocketOptions&>(),
      call_guard<GilRelease>()).
    def(init<const IpAddress&, const IpAddress&>(), call_guard<GilRelease>()).
    def(init<const IpAddress&, const IpAddress&, const UdpSocketOptions&>(),
      call_guard<GilRelease>()).
    def("__del__", [] (UdpSocket& self) {
      self.Close();
    }, call_guard<GilRelease>()).
    def_property_readonly("address", &UdpSocket::GetAddress).
    def_property_readonly("receiver", &UdpSocket::GetReceiver,
      return_value_policy::reference_internal).
    def_property_readonly("sender", &UdpSocket::GetSender,
      return_value_policy::reference_internal).
    def("close", &UdpSocket::Close);
}

void Beam::Python::ExportUdpSocketChannel(pybind11::module& module) {
  ExportChannel<ToPythonChannel<UdpSocketChannel>>(module, "UdpSocketChannel").
    def(init<const IpAddress&>()).
    def(init<const IpAddress&, const UdpSocketOptions&>()).
    def(init<const IpAddress&, const IpAddress&>()).
    def(init<const IpAddress&, const IpAddress&, const UdpSocketOptions&>());
}

void Beam::Python::ExportUdpSocketConnection(pybind11::module& module) {
  ExportConnection<ToPythonConnection<UdpSocketConnection>>(module,
    "UdpSocketConnection");
}

void Beam::Python::ExportUdpSocketOptions(pybind11::module& module) {
  class_<UdpSocketOptions>(module, "UdpSocketOptions").
    def(init()).
    def(init<const UdpSocketOptions&>()).
    def_readwrite("timeout", &UdpSocketOptions::m_timeout).
    def_readwrite("ttl", &UdpSocketOptions::m_ttl).
    def_readwrite("ttl", &UdpSocketOptions::m_ttl).
    def_readwrite("enable_loopback", &UdpSocketOptions::m_enableLoopback).
    def_readwrite("max_datagram_size", &UdpSocketOptions::m_maxDatagramSize).
    def_readwrite("receive_buffer_size",
      &UdpSocketOptions::m_receiveBufferSize);
}

void Beam::Python::ExportUdpSocketReader(pybind11::module& module) {
  ExportReader<ToPythonReader<UdpSocketReader>>(module, "UdpSocketReader");
}

void Beam::Python::ExportUdpSocketReceiver(pybind11::module& module) {
  class_<UdpSocketReceiver>(module, "UdpSocketReceiver").
    def("receive", [] (UdpSocketReceiver& self,
        Out<DatagramPacket<SharedBuffer>> packet) {
      return self.Receive(Store(packet));
    }, call_guard<GilRelease>()).
    def("receive", [] (UdpSocketReceiver& self,
        Out<DatagramPacket<SharedBuffer>> packet, std::size_t size) {
      return self.Receive(Store(packet), size);
    }, call_guard<GilRelease>()).
    def("receive", [] (UdpSocketReceiver& self, Out<BufferBox> buffer,
        Out<IpAddress> address) {
      return self.Receive(Store(buffer), Store(address));
    }, call_guard<GilRelease>()).
    def("receive", [] (UdpSocketReceiver& self, Out<BufferBox> buffer,
        std::size_t size, Out<IpAddress> address) {
      return self.Receive(Store(buffer), size, Store(address));
    }, call_guard<GilRelease>());
}

void Beam::Python::ExportUdpSocketSender(pybind11::module& module) {
  class_<UdpSocketSender>(module, "UdpSocketSender").
    def("send", &UdpSocketSender::Send<BufferBox>, call_guard<GilRelease>());
}

void Beam::Python::ExportUdpSocketWriter(pybind11::module& module) {
  ExportWriter<ToPythonWriter<UdpSocketWriter>>(module, "UdpSocketWriter");
}
