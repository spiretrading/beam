#ifndef BEAM_STOMPFRAMEPARSER_HPP
#define BEAM_STOMPFRAMEPARSER_HPP
#include <deque>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Stomp/StompCommand.hpp"
#include "Beam/Stomp/StompException.hpp"
#include "Beam/Stomp/StompFrame.hpp"
#include "Beam/Stomp/Stomp.hpp"

namespace Beam {
namespace Stomp {
namespace Details {
  inline std::string Unescape(const char* c, unsigned int length) {
    std::string result;
    for(unsigned int i = 0; i < length; ++i) {
      if(c[i] != '\\') {
        result += c[i];
      } else {
        ++i;
        if(i == length) {
          BOOST_THROW_EXCEPTION(StompException{"Invalid escape character"});
        }
        if(c[i] == 'n') {
          result += '\n';
        } else if(c[i] == 'r') {
          result += '\r';
        } else if(c[i] == 'c') {
          result += ':';
        } else if(c[i] == '\\') {
          result += '\\';
        } else {
          BOOST_THROW_EXCEPTION(StompException{"Invalid escape character."});
        }
      }
    }
    return result;
  }
}

  /*! \class StompFrameParser
      \brief Parses StompFrames.
   */
  class StompFrameParser : boost::noncopyable {
    public:

      //! Constructs a StompFrameParser.
      StompFrameParser();

      //! Feeds the parser additional characters to parse.
      /*!
        \param c The first character to feed.
        \param size The number of characters to feed.
      */
      void Feed(const char* c, std::size_t size);

      //! Returns the next StompFrame.
      boost::optional<StompFrame> GetNextFrame();

    private:
      enum class ParserState {
        COMMAND,
        HEADER,
        BODY,
        NULL_TERMINATED_BODY,
        CONTENT_LENGTH_BODY,
        END,
        COMPLETE,
        ERR
      };
      ParserState m_parserState;
      StompCommand m_command;
      std::vector<StompHeader> m_headers;
      IO::SharedBuffer m_body;
      std::int64_t m_contentLength;
      bool m_hasDestinationHeader;
      bool m_hasIdHeader;
      std::deque<StompFrame> m_frames;
      IO::SharedBuffer m_buffer;

      void ParseCommand(const char* c, std::size_t size);
      void ParseHeader(const char* c, std::size_t size);
      void ParseBody(const char* c);
  };

  inline StompFrameParser::StompFrameParser()
      : m_parserState{ParserState::COMMAND},
        m_contentLength{-1},
        m_hasDestinationHeader{false},
        m_hasIdHeader{false} {}

  inline void StompFrameParser::Feed(const char* c, std::size_t size) {
    if(m_parserState == ParserState::END) {
      while(size != 0 && *c == '\0') {
        ++c;
        --size;
      }
      if(size == 0) {
        return;
      }
      if(*c == '\r') {
        ++c;
        --size;
      }
      if(size == 0) {
        return;
      }
      if(*c == '\n') {
        ++c;
        --size;
      }
      m_parserState = ParserState::COMMAND;
    }
    while(m_parserState == ParserState::COMMAND) {
      auto end = static_cast<const char*>(std::memchr(c, '\n', size));
      if(end == nullptr) {
        m_buffer.Append(c, size);
        return;
      }
      if(end == c) {
        ++c;
        --size;
        continue;
      } else if(end == c + 1 && *c == '\r') {
        c += 2;
        size -= 2;
        continue;
      }
      if(m_buffer.IsEmpty()) {
        ParseCommand(c, (end - c));
      } else {
        m_buffer.Append(c, end - c);
        ParseCommand(m_buffer.GetData(), m_buffer.GetSize());
        m_buffer.Reset();
      }
      if(m_parserState == ParserState::ERR) {
        return;
      }
      size -= (end - c) + 1;
      c = end + 1;
      m_parserState = ParserState::HEADER;
    }
    while(m_parserState == ParserState::HEADER) {
      if(size == 0) {
        return;
      }
      auto end = static_cast<const char*>(std::memchr(c, '\n', size));
      if(end == nullptr) {
        m_buffer.Append(c, size);
        return;
      } else if(end == c) {
        ++c;
        --size;
        m_parserState = ParserState::BODY;
        break;
      } else if(end == c + 1 && *c == '\r') {
        c += 2;
        size -= 2;
        m_parserState = ParserState::BODY;
        break;
      }
      if(m_buffer.IsEmpty()) {
        if(*(end - 1) == '\r') {
          ParseHeader(c, (end - c) - 1);
        } else {
          ParseHeader(c, end - c);
        }
      } else {
        m_buffer.Append(c, end - c);
        ParseHeader(m_buffer.GetData(), m_buffer.GetSize());
        m_buffer.Reset();
      }
      if(m_parserState == ParserState::ERR) {
        return;
      }
      size -= (end - c) + 1;
      c = end + 1;
      m_parserState = ParserState::HEADER;
    }
    if(m_parserState == ParserState::BODY) {
      if(m_command != StompCommand::SEND &&
          m_command != StompCommand::MESSAGE &&
          m_command != StompCommand::ERR) {
        m_parserState = ParserState::COMPLETE;
      } else if(m_contentLength == -1) {
        m_parserState = ParserState::NULL_TERMINATED_BODY;
      } else {
        m_parserState = ParserState::CONTENT_LENGTH_BODY;
      }
    }
    if(m_parserState == ParserState::NULL_TERMINATED_BODY) {
      auto end = static_cast<const char*>(std::memchr(c, '\0', size));
      if(end == nullptr) {
        m_buffer.Append(c, size);
        return;
      }
      m_body = std::move(m_buffer);
      m_body.Append(c, static_cast<std::size_t>(end - c));
      m_buffer.Reset();
      size -= (end - c) + 1;
      c = end + 1;
      m_parserState = ParserState::COMPLETE;
    }
    if(m_parserState == ParserState::CONTENT_LENGTH_BODY) {
      if(m_buffer.GetSize() + size < m_contentLength) {
        m_buffer.Append(c, size);
        return;
      }
      if(m_buffer.IsEmpty()) {
        ParseBody(c);
        size -= m_body.GetSize();
        c += m_body.GetSize();
      } else {
        auto length = static_cast<std::size_t>(
          m_contentLength - m_buffer.GetSize());
        m_buffer.Append(c, length);
        ParseBody(m_buffer.GetData());
        m_buffer.Reset();
        size -= length;
        c += length;
      }
      if(m_parserState == ParserState::ERR) {
        return;
      }
      m_parserState = ParserState::COMPLETE;
    }
    if(m_parserState == ParserState::COMPLETE) {
      if(m_command == StompCommand::SEND ||
          m_command == StompCommand::SUBSCRIBE) {
        if(!m_hasDestinationHeader) {
          m_parserState = ParserState::ERR;
          return;
        }
      }
      if(m_command == StompCommand::SUBSCRIBE) {
        if(!m_hasIdHeader) {
          m_parserState = ParserState::ERR;
          return;
        }
      }
      m_frames.emplace_back(m_command, std::move(m_headers), std::move(m_body));
      m_headers.clear();
      m_contentLength = -1;
      m_body.Reset();
      m_hasDestinationHeader = false;
      m_hasIdHeader = false;
      m_parserState = ParserState::END;
      while(size != 0 && *c == '\0') {
        ++c;
        --size;
      }
      if(size != 0) {
        if(*c == '\r') {
          ++c;
          --size;
        }
        if(size == 0) {
          return;
        }
        if(*c == '\n') {
          ++c;
          --size;
        }
        m_parserState = ParserState::COMMAND;
        if(size == 0) {
          return;
        }
        m_buffer.Append(c, size);
        auto tempBuffer = std::move(m_buffer);
        m_buffer = IO::SharedBuffer{};
        Feed(tempBuffer.GetData(), tempBuffer.GetSize());
      }
    }
  }

  inline boost::optional<StompFrame> StompFrameParser::GetNextFrame() {
    if(m_frames.empty()) {
      if(m_parserState == ParserState::ERR) {
        BOOST_THROW_EXCEPTION(StompException{"Invalid frame."});
      }
      return boost::none;
    }
    auto frame = std::move(m_frames.front());
    m_frames.pop_front();
    return std::move(frame);
  }

  inline void StompFrameParser::ParseCommand(const char* c, std::size_t size) {
    if(size >= 9 && std::memcmp(c, "CONNECTED", 9) == 0) {
      m_command = StompCommand::CONNECTED;
      c += 9;
      size -= 9;
    } else if(size >= 7 && std::memcmp(c, "CONNECT", 7) == 0) {
      m_command = StompCommand::CONNECT;
      c += 7;
      size -= 7;
    } else if(size >= 5 && std::memcmp(c, "STOMP", 5) == 0) {
      m_command = StompCommand::STOMP;
      c += 5;
      size -= 5;
    } else if(size >= 5 && std::memcmp(c, "ERROR", 5) == 0) {
      m_command = StompCommand::ERR;
      c += 5;
      size -= 5;
    } else if(size >= 4 && std::memcmp(c, "SEND", 4) == 0) {
      m_command = StompCommand::SEND;
      c += 4;
      size -= 4;
    } else if(size >= 9 && std::memcmp(c, "SUBSCRIBE", 9) == 0) {
      m_command = StompCommand::SUBSCRIBE;
      c += 9;
      size -= 9;
    } else if(size >= 11 && std::memcmp(c, "UNSUBSCRIBE", 11) == 0) {
      m_command = StompCommand::SUBSCRIBE;
      c += 11;
      size -= 11;
    } else if(size >= 3 && std::memcmp(c, "ACK", 3) == 0) {
      m_command = StompCommand::ACK;
      c += 3;
      size -= 3;
    } else if(size >= 4 && std::memcmp(c, "NACK", 4) == 0) {
      m_command = StompCommand::NACK;
      c += 4;
      size -= 4;
    } else if(size >= 5 && std::memcmp(c, "BEGIN", 5) == 0) {
      m_command = StompCommand::BEGIN;
      c += 5;
      size -= 5;
    } else if(size >= 6 && std::memcmp(c, "COMMIT", 6) == 0) {
      m_command = StompCommand::COMMIT;
      c += 6;
      size -= 6;
    } else if(size >= 5 && std::memcmp(c, "ABORT", 5) == 0) {
      m_command = StompCommand::ABORT;
      c += 5;
      size -= 5;
    } else if(size >= 10 && std::memcmp(c, "DISCONNECT", 10) == 0) {
      m_command = StompCommand::DISCONNECT;
      c += 10;
      size -= 10;
    } else if(size >= 7 && std::memcmp(c, "MESSAGE", 7) == 0) {
      m_command = StompCommand::MESSAGE;
      c += 7;
      size -= 7;
    } else if(size >= 7 && std::memcmp(c, "RECEIPT", 7) == 0) {
      m_command = StompCommand::RECEIPT;
      c += 7;
      size -= 7;
    } else {
      m_parserState = ParserState::ERR;
      return;
    }
  }

  inline void StompFrameParser::ParseHeader(const char* c, std::size_t size) {
    auto nameEnd = static_cast<const char*>(std::memchr(c, ':', size));
    if(nameEnd == nullptr) {
      m_parserState = ParserState::ERR;
      return;
    }
    auto nameLength = static_cast<unsigned int>(nameEnd - c);
    auto name = Details::Unescape(c, nameLength);
    c += nameLength + 1;
    size -= nameLength + 1;
    auto value = Details::Unescape(c, size);
    if(m_contentLength == -1 && name == "content-length") {
      m_contentLength = std::stoul(value);
    } else {
      if(m_command == StompCommand::SEND) {
        if(!m_hasDestinationHeader && name == "destination") {
          m_hasDestinationHeader = true;
        }
      } else if(m_command == StompCommand::SUBSCRIBE) {
        if(!m_hasDestinationHeader && name == "destination") {
          m_hasDestinationHeader = true;
        } else if(!m_hasIdHeader && name == "id") {
          m_hasIdHeader = true;
        }
      }
      m_headers.emplace_back(std::move(name), std::move(value));
    }
  }

  inline void StompFrameParser::ParseBody(const char* c) {
    if(m_contentLength != 0) {
      m_body.Append(c, static_cast<std::size_t>(m_contentLength));
    }
  }
}
}

#endif
