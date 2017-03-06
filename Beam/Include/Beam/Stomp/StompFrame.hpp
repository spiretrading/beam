#ifndef BEAM_STOMPFRAME_HPP
#define BEAM_STOMPFRAME_HPP
#include <algorithm>
#include <vector>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Stomp/Stomp.hpp"
#include "Beam/Stomp/StompCommand.hpp"
#include "Beam/Stomp/StompHeader.hpp"

namespace Beam {
namespace Stomp {

  /*! \class StompFrame
      \brief Represents a STOMP frame.
   */
  class StompFrame {
    public:

      //! Constructs a STOMP frame.
      /*!
        \param command The StompCommand represented.
      */
      StompFrame(StompCommand command);

      //! Constructs a STOMP frame.
      /*!
        \param command The StompCommand represented.
        \param headers The list of StompHeaders.
      */
      StompFrame(StompCommand command, std::vector<StompHeader> headers);

      //! Returns the command.
      StompCommand GetCommand() const;

      //! Constructs a STOMP frame.
      /*!
        \param command The StompCommand represented.
        \param headers The list of StompHeaders.
        \param body The body of the frame.
      */
      template<typename Buffer>
      StompFrame(StompCommand command, std::vector<StompHeader> headers,
        Buffer&& body);

      //! Returns the list of headers.
      const std::vector<StompHeader>& GetHeaders() const;

      //! Adds a header.
      /*!
        \param header The StompHeader to add.
      */
      void AddHeader(StompHeader header);

      //! Finds a header with a specified name.
      /*!
        \param name The name of the header.
        \return The value associated with the header with the specified
                <i>name</i>.
      */
      boost::optional<const std::string&> FindHeader(
        const std::string& name) const;

      //! Returns the body.
      const IO::SharedBuffer& GetBody() const;

      //! Sets the body.
      /*!
        \param body The body.
      */
      template<typename Buffer>
      void SetBody(Buffer&& body);

      //! Sets the body.
      /*!
        \param contentType The content-type of the body.
        \param body The body.
      */
      template<typename Buffer>
      void SetBody(const std::string& contentType, Buffer&& body);

    private:
      StompCommand m_command;
      std::vector<StompHeader> m_headers;
      IO::SharedBuffer m_body;
  };

  template<typename Buffer>
  void Serialize(const StompFrame& frame, Out<Buffer> buffer) {
    auto& command = ToString(frame.GetCommand());
    buffer->Append(command.c_str(), command.size());
    buffer->Append("\n", 1);
    for(auto& header : frame.GetHeaders()) {
      buffer->Append(header.GetName().c_str(), header.GetName().size());
      buffer->Append(":", 1);
      buffer->Append(header.GetValue().c_str(), header.GetValue().size());
      buffer->Append("\n", 1);
    }
    buffer->Append("\n", 1);
    buffer->Append(frame.GetBody());
    buffer->Append("\n", 1);
  }

  inline StompFrame::StompFrame(StompCommand command)
      : m_command{command} {}

  inline StompFrame::StompFrame(StompCommand command,
      std::vector<StompHeader> headers)
      : m_command{command},
        m_headers{std::move(headers)} {}

  template<typename Buffer>
  StompFrame::StompFrame(StompCommand command, std::vector<StompHeader> headers,
      Buffer&& body)
      : m_command{command},
        m_headers{std::move(headers)} {
    SetBody(std::move(body));
  }

  inline StompCommand StompFrame::GetCommand() const {
    return m_command;
  }

  inline const std::vector<StompHeader>& StompFrame::GetHeaders() const {
    return m_headers;
  }

  inline void StompFrame::AddHeader(StompHeader header) {
    m_headers.push_back(std::move(header));
  }

  inline boost::optional<const std::string&> StompFrame::FindHeader(
      const std::string& name) const {
    auto header = std::find_if(m_headers.begin(), m_headers.end(),
      [&] (const StompHeader& header) {
        return header.GetName() == name;
      });
    if(header == m_headers.end()) {
      return boost::none;
    }
    return header->GetValue();
  }

  inline const IO::SharedBuffer& StompFrame::GetBody() const {
    return m_body;
  }

  template<typename Buffer>
  void StompFrame::SetBody(Buffer&& body) {
    m_body = std::move(body);
    if(m_body.GetSize() != 0) {
      AddHeader({"content-length", m_body.GetSize()});
    }
  }

  template<typename Buffer>
  void StompFrame::SetBody(const std::string& contentType, Buffer&& body) {
    SetBody(std::move(body));
    AddHeader({"content-type", contentType});
  }
}
}

#endif
