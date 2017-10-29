#include "Beam/Python/Network.hpp"
#include "Beam/IO/VirtualChannel.hpp"
#include "Beam/IO/VirtualChannelIdentifier.hpp"
#include "Beam/IO/VirtualConnection.hpp"
#include "Beam/IO/VirtualReader.hpp"
#include "Beam/IO/VirtualWriter.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Network/SocketIdentifier.hpp"
#include "Beam/Network/TcpSocketChannel.hpp"
#include "Beam/Network/TcpSocketConnection.hpp"
#include "Beam/Network/TcpSocketReader.hpp"
#include "Beam/Network/TcpSocketWriter.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Copy.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/PythonBindings.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace boost;
using namespace boost::python;
using namespace std;

namespace {
  IpAddress GetSocketIdentifierAddress(
      WrapperChannelIdentifier<SocketIdentifier>& identifier) {
    return identifier.GetIdentifier().GetAddress();
  }

  WrapperChannelIdentifier<SocketIdentifier>* MakeEmptySocketIdentifier() {
    return new WrapperChannelIdentifier<SocketIdentifier>{SocketIdentifier{}};
  }

  WrapperChannelIdentifier<SocketIdentifier>* MakeSocketIdentifier(
      const IpAddress& address) {
    return new WrapperChannelIdentifier<SocketIdentifier>{
      SocketIdentifier{address}};
  }

  int GetTcpSocketConnectionWriteBufferSize(
      WrapperConnection<TcpSocketConnection*>& connection) {
    return connection.GetConnection().GetWriteBufferSize();
  }

  void SetTcpSocketConnectionWriteBufferSize(
      WrapperConnection<TcpSocketConnection*>& connection, int size) {
    return connection.GetConnection().SetWriteBufferSize(size);
  }

  class PythonTcpSocketChannel : public VirtualChannel {
    public:

      PythonTcpSocketChannel(const IpAddress& address)
          : m_channel{address, Ref(*GetSocketThreadPool())},
            m_identifier{m_channel.GetIdentifier()},
            m_connection{&m_channel.GetConnection()},
            m_reader{&m_channel.GetReader()},
            m_writer{&m_channel.GetWriter()} {}

      PythonTcpSocketChannel(const IpAddress& address,
          const IpAddress& interface)
          : m_channel{address, interface, Ref(*GetSocketThreadPool())},
            m_identifier{m_channel.GetIdentifier()},
            m_connection{&m_channel.GetConnection()},
            m_reader{&m_channel.GetReader()},
            m_writer{&m_channel.GetWriter()} {}

      PythonTcpSocketChannel(const std::vector<IpAddress>& addresses)
          : m_channel{addresses, Ref(*GetSocketThreadPool())},
            m_identifier{m_channel.GetIdentifier()},
            m_connection{&m_channel.GetConnection()},
            m_reader{&m_channel.GetReader()},
            m_writer{&m_channel.GetWriter()} {}

      PythonTcpSocketChannel(const std::vector<IpAddress>& addresses,
          const IpAddress& interface)
          : m_channel{addresses, interface, Ref(*GetSocketThreadPool())},
            m_identifier{m_channel.GetIdentifier()},
            m_connection{&m_channel.GetConnection()},
            m_reader{&m_channel.GetReader()},
            m_writer{&m_channel.GetWriter()} {}

      virtual const WrapperChannelIdentifier<SocketIdentifier>&
          GetIdentifier() const override {
        return m_identifier;
      }

      virtual WrapperConnection<TcpSocketConnection*>& GetConnection() {
        return m_connection;
      }

      virtual WrapperReader<TcpSocketReader*>& GetReader() {
        return m_reader;
      }

      virtual WrapperWriter<TcpSocketWriter*>& GetWriter() {
        return m_writer;
      }

    private:
      TcpSocketChannel m_channel;
      WrapperChannelIdentifier<SocketIdentifier> m_identifier;
      WrapperConnection<TcpSocketConnection*> m_connection;
      WrapperReader<TcpSocketReader*> m_reader;
      WrapperWriter<TcpSocketWriter*> m_writer;
  };

  PythonTcpSocketChannel* MakeTcpSocketChannelFromAddressList(
      const boost::python::list addresses) {
    std::vector<IpAddress> properAddresses;
    for(int i = 0; i < boost::python::len(addresses); ++i) {
      properAddresses.push_back(
        boost::python::extract<IpAddress>(addresses[i]));
    }
    return new PythonTcpSocketChannel{properAddresses};
  }

  PythonTcpSocketChannel* MakeTcpSocketChannelFromAddressListInterface(
      const boost::python::list addresses, const IpAddress& interface) {
    std::vector<IpAddress> properAddresses;
    for(int i = 0; i < boost::python::len(addresses); ++i) {
      properAddresses.push_back(
        boost::python::extract<IpAddress>(addresses[i]));
    }
    return new PythonTcpSocketChannel{properAddresses, interface};
  }
}

BEAM_DEFINE_PYTHON_POINTER_LINKER(WrapperChannelIdentifier<SocketIdentifier>);

void Beam::Python::ExportIpAddress() {
  class_<IpAddress>("IpAddress", init<>())
    .def(init<std::string, unsigned short>())
    .def("__copy__", &MakeCopy<IpAddress>)
    .def("__deepcopy__", &MakeDeepCopy<IpAddress>)
    .def("__str__", &Convert<string, IpAddress>)
    .add_property("host", make_function(&IpAddress::GetHost,
      return_value_policy<copy_const_reference>()))
    .add_property("port", &IpAddress::GetPort)
    .def(self == self)
    .def(self != self);
}

void Beam::Python::ExportNetwork() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".network");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("network") = nestedModule;
  scope parent = nestedModule;
  ExportIpAddress();
  ExportSocketIdentifier();
  ExportTcpSocketConnection();
  ExportTcpSocketReader();
  ExportTcpSocketWriter();
  ExportTcpSocketChannel();
  ExportException<SocketException, IOException>("SocketException")
    .def(init<int, const string&>())
    .add_property("code", &SocketException::GetCode);
}

void Beam::Python::ExportSocketIdentifier() {
  class_<WrapperChannelIdentifier<SocketIdentifier>, noncopyable,
      bases<VirtualChannelIdentifier>>("SocketIdentifier", no_init)
    .def("__init__", make_constructor(&MakeEmptySocketIdentifier))
    .def("__init__", make_constructor(&MakeSocketIdentifier))
    .add_property("address", &GetSocketIdentifierAddress);
}

void Beam::Python::ExportTcpSocketChannel() {
  class_<PythonTcpSocketChannel, noncopyable, bases<VirtualChannel>>(
    "TcpSocketChannel", init<const IpAddress&>())
    .def(init<const IpAddress&, const IpAddress&>())
    .def("__init__", make_constructor(&MakeTcpSocketChannelFromAddressList))
    .def("__init__", make_constructor(
      &MakeTcpSocketChannelFromAddressListInterface));
}

void Beam::Python::ExportTcpSocketConnection() {
  class_<WrapperConnection<TcpSocketConnection*>, noncopyable,
      bases<VirtualConnection>>("TcpSocketConnection", no_init)
    .add_property("write_buffer_size", &GetTcpSocketConnectionWriteBufferSize,
      &SetTcpSocketConnectionWriteBufferSize);
}

void Beam::Python::ExportTcpSocketReader() {
  class_<WrapperReader<TcpSocketReader*>, noncopyable, bases<VirtualReader>>(
    "TcpSocketReader", no_init);
}

void Beam::Python::ExportTcpSocketWriter() {
  class_<WrapperWriter<TcpSocketWriter*>, noncopyable, bases<VirtualWriter>>(
    "TcpSocketWriter", no_init);
}
