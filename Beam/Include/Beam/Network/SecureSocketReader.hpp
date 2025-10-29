#ifndef BEAM_SECURE_SOCKET_READER_HPP
#define BEAM_SECURE_SOCKET_READER_HPP
#include <boost/throw_exception.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/Network/NetworkDetails.hpp"
#include "Beam/Network/SocketException.hpp"
#include "Beam/Routines/Async.hpp"

namespace Beam {

  /** Reads from an SSL socket. */
  class SecureSocketReader {
    public:
      bool poll() const;
      template<IsBuffer R>
      std::size_t read(Out<R> destination, std::size_t size = -1);

    private:
      friend class SecureSocketChannel;
      std::shared_ptr<Details::SecureSocketEntry> m_socket;

      SecureSocketReader(std::shared_ptr<Details::SecureSocketEntry> socket);
      SecureSocketReader(const SecureSocketReader&) = delete;
      SecureSocketReader& operator =(const SecureSocketReader&) = delete;
  };

  inline bool SecureSocketReader::poll() const {
    auto command = boost::asio::socket_base::bytes_readable(true);
    {
      auto lock = boost::lock_guard(m_socket->m_mutex);
      try {
        m_socket->m_socket.lowest_layer().io_control(command);
      } catch(const std::exception&) {
        return false;
      }
    }
    return command.get() > 0;
  }

  template<IsBuffer R>
  std::size_t SecureSocketReader::read(Out<R> destination, std::size_t size) {
    static const auto DEFAULT_READ_SIZE = std::size_t(8 * 1024);
    auto available_size = destination->grow(std::min(DEFAULT_READ_SIZE, size));
    auto read_result = Async<std::size_t>();
    {
      auto lock = std::lock_guard(m_socket->m_mutex);
      if(!m_socket->m_is_open) {
        boost::throw_with_location(EndOfFileException());
      }
      m_socket->m_is_read_pending = true;
      m_socket->m_socket.async_read_some(boost::asio::buffer(
        get_mutable_suffix(*destination, available_size), available_size),
        [&] (const auto& error, auto read_size) {
          if(error) {
            read_result.get_eval().set_exception(
              SocketException(error.value(), error.message()));
          } else {
            read_result.get_eval().set(read_size);
          }
        });
    }
    try {
      auto result = read_result.get();
      m_socket->end_read_operation();
      destination->shrink(available_size - result);
      return result;
    } catch(const std::exception&) {
      m_socket->end_read_operation();
      std::throw_with_nested(EndOfFileException());
    }
  }

  inline SecureSocketReader::SecureSocketReader(
    std::shared_ptr<Details::SecureSocketEntry> socket)
    : m_socket(std::move(socket)) {}
}

#endif
