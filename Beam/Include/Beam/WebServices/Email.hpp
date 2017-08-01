#ifndef BEAM_EMAIL_HPP
#define BEAM_EMAIL_HPP
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/WebServices/EmailAddress.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {
namespace Details {
  inline std::string FormatDate(boost::posix_time::ptime date) {
    std::stringstream ss;
    ss << date.date().day_of_week().as_short_string() << ", " <<
      date.date().day() << " " << date.date().month().as_short_string() <<
      " " << date.date().year() << " ";
    if(date.time_of_day().hours() < 10) {
      ss << '0';
    }
    ss << date.time_of_day().hours() << ":";
    if(date.time_of_day().minutes() < 10) {
      ss << '0';
    }
    ss << date.time_of_day().minutes() << ":";
    if(date.time_of_day().seconds() < 10) {
      ss << '0';
    }
    ss << date.time_of_day().seconds();
    auto timeZone = date.zone_abbrev(true);
    if(timeZone.empty()) {
      ss << " +0000";
    } else {
      ss << " " << timeZone;
    }
    return ss.str();
  }

  inline std::string DotStuff(const std::string& source) {
    std::string body;
    auto isNewLine = true;
    for(auto c : source) {
      if(isNewLine && c == '.') {
        body += "..";
      } else {
        body += c;
      }
      isNewLine = c == '\n';
    }
    return body;
  }
}

  /*! \class Email
      \brief Represents an email message.
   */
  class Email {
    public:

      /*! \struct Header
          \brief Stores an email header.
       */
      struct Header {

        //! The header name.
        std::string m_name;

        //! The header value.
        std::string m_value;
      };

      /*! \struct Body
          \brief Stores the body of the message.
       */
      struct Body {

        //! The content type.
        std::string m_contentType;

        //! The message.
        std::string m_message;
      };

      //! Constructs an empty email.
      /*!
        \param from The from header.
       */
      Email(EmailAddress from);

      //! Constructs an empty email.
      /*!
        \param from The from header.
        \param to The recipient.
       */
      Email(EmailAddress from, EmailAddress to);

      //! Returns the from address.
      const EmailAddress& GetFrom() const;

      //! Sets the from address.
      void SetFrom(EmailAddress address);

      //! Returns the date when the message was written.
      boost::posix_time::ptime GetDate() const;

      //! Sets the date.
      void SetDate(boost::posix_time::ptime date);

      //! Returns the sender address.
      const boost::optional<EmailAddress>& GetSender() const;

      //! Sets the sender address.
      void SetSender(EmailAddress address);

      //! Returns the to addresses.
      const std::vector<EmailAddress>& GetTo() const;

      //! Adds a recipient.
      void AddTo(EmailAddress address);

      //! Returns the subject.
      const std::string& GetSubject() const;

      //! Sets the subject.
      void SetSubject(std::string subject);

      //! Returns the bodies.
      const std::vector<Body>& GetBodies() const;

      //! Adds a body.
      void AddBody(std::string message);

      //! Adds a body.
      void AddBody(std::string contentType, std::string message);

      //! Finds a header.
      /*!
        \param name The name of the header to find.
        \return The header value.
      */
      boost::optional<std::string> FindHeader(const std::string& name) const;

      //! Returns all additional headers.
      const std::vector<Header>& GetAdditionalHeaders() const;

      //! Sets a header.
      /*!
        \param name The name of the header to set.
        \param value The header's value.
      */
      void SetHeader(std::string name, std::string value);

    private:
      EmailAddress m_from;
      boost::posix_time::ptime m_date;
      boost::optional<EmailAddress> m_sender;
      std::vector<EmailAddress> m_to;
      std::string m_subject;
      std::vector<Header> m_additionalHeaders;
      std::vector<Body> m_bodies;
      std::string m_bodyBoundary;
  };

  inline std::ostream& operator <<(std::ostream& sink, const Email& email) {
    sink << "From: " << email.GetFrom() << "\r\n";
    sink << "Date: " << Details::FormatDate(email.GetDate()) << "\r\n";
    if(email.GetSender().is_initialized()) {
      sink << "Sender: " << *email.GetSender() << "\r\n";
    }
    auto& to = email.GetTo();
    if(!to.empty()) {
      sink << "To: ";
      auto isFirst = true;
      for(auto& recipient : to) {
        if(!isFirst) {
          sink << ", ";
        }
        sink << recipient;
        isFirst = false;
      }
      sink << "\r\n";
    }
    if(!email.GetSubject().empty()) {
      sink << "Subject: " << email.GetSubject() << "\r\n";
    }
    for(auto& header : email.GetAdditionalHeaders()) {
      sink << header.m_name << ": " << header.m_value << "\r\n";
    }
    auto& bodies = email.GetBodies();
    std::string boundary;
    if(bodies.size() == 1) {
      sink << "Content-Type: " << bodies.front().m_contentType << "\r\n";
    } else if(bodies.size() > 1) {
      for(auto i = 0; i < 45; ++i) {
        boundary += 'a' + static_cast<char>(rand() % 26);
      }
      sink << "Content-Type: multipart/alternative; boundary=\"" << boundary <<
        "\"\r\n";
    }
    sink << "\r\n";
    if(bodies.size() == 1) {
      sink << Details::DotStuff(bodies.front().m_message);
    } else {
      for(auto& body : bodies) {
        sink << "--" << boundary << "\r\n";
        sink << "Content-Type: " << body.m_contentType << "\r\n\r\n";
        sink << body.m_message << "\r\n\r\n";
      }
    }
    if(bodies.size() > 1) {
      sink << "--" << boundary << "--\r\n";
    }
    sink << "\r\n.\r\n";
    return sink;
  }

  inline Email::Email(EmailAddress from)
      : m_from{std::move(from)},
        m_date{boost::posix_time::second_clock::universal_time()} {
    SetHeader("Mime-Version", "1.0");
  }

  inline Email::Email(EmailAddress from, EmailAddress to)
      : Email{std::move(from)} {
    m_to.push_back(std::move(to));
  }

  inline const EmailAddress& Email::GetFrom() const {
    return m_from;
  }

  inline void Email::SetFrom(EmailAddress address) {
    m_from = std::move(address);
  }

  inline boost::posix_time::ptime Email::GetDate() const {
    return m_date;
  }

  inline void Email::SetDate(boost::posix_time::ptime date) {
    m_date = date;
  }

  inline const boost::optional<EmailAddress>& Email::GetSender() const {
    return m_sender;
  }

  inline void Email::SetSender(EmailAddress address) {
    m_sender = std::move(address);
  }

  inline const std::vector<EmailAddress>& Email::GetTo() const {
    return m_to;
  }

  inline void Email::AddTo(EmailAddress address) {
    m_to.push_back(std::move(address));
  }

  inline const std::string& Email::GetSubject() const {
    return m_subject;
  }

  inline void Email::SetSubject(std::string subject) {
    m_subject = std::move(subject);
  }

  inline const std::vector<Email::Body>& Email::GetBodies() const {
    return m_bodies;
  }

  inline void Email::AddBody(std::string message) {
    AddBody("text/plain", message);
  }

  inline void Email::AddBody(std::string contentType, std::string message) {
    m_bodies.push_back({std::move(contentType), std::move(message)});
  }

  inline boost::optional<std::string> Email::FindHeader(
      const std::string& name) const {
    auto headerIterator = std::find_if(m_additionalHeaders.begin(),
      m_additionalHeaders.end(),
      [&] (auto& header) {
        return boost::algorithm::iequals(name, header.m_name);
      });
    if(headerIterator == m_additionalHeaders.end()) {
      return boost::none;
    }
    return headerIterator->m_value;
  }

  inline const std::vector<Email::Header>& Email::GetAdditionalHeaders() const {
    return m_additionalHeaders;
  }

  inline void Email::SetHeader(std::string name, std::string value) {
    auto headerIterator = std::find_if(m_additionalHeaders.begin(),
      m_additionalHeaders.end(),
      [&] (auto& header) {
        return boost::algorithm::iequals(name, header.m_name);
      });
    if(headerIterator == m_additionalHeaders.end()) {
      m_additionalHeaders.push_back({std::move(name), std::move(value)});
    } else {
      headerIterator->m_value = std::move(value);
    }
  }
}
}

#endif
