#ifndef BEAM_EMAIL_HPP
#define BEAM_EMAIL_HPP
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/algorithm/string/case_conv.hpp>
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

      //! Returns the body/message.
      const std::string& GetBody() const;

      //! Sets the body.
      void SetBody(std::string body);

      //! Finds a header.
      /*!
        \param name The name of the header to find.
        \return The header value.
      */
      boost::optional<std::string> FindHeader(const std::string& name) const;

    private:
      std::unordered_map<std::string, std::string> m_headers;
      EmailAddress m_from;
      boost::posix_time::ptime m_date;
      boost::optional<EmailAddress> m_sender;
      std::vector<EmailAddress> m_to;
      std::string m_subject;
      std::string m_body;
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
    sink << "\r\n" << Details::DotStuff(email.GetBody()) << "\r\n.\r\n";
    return sink;
  }

  inline Email::Email(EmailAddress from)
      : m_from{std::move(from)},
        m_date{boost::posix_time::second_clock::universal_time()} {}

  inline Email::Email(EmailAddress from, EmailAddress to)
      : m_from{std::move(from)},
        m_date{boost::posix_time::second_clock::universal_time()} {
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

  inline const std::string& Email::GetBody() const {
    return m_body;
  }

  inline void Email::SetBody(std::string body) {
    m_body = std::move(body);
  }

  inline boost::optional<std::string> Email::FindHeader(
      const std::string& name) const {
    auto lowercaseName = boost::algorithm::to_lower_copy(name);
    if(lowercaseName == "from") {
      return boost::lexical_cast<std::string>(m_from);
    } else if(lowercaseName == "date") {
      return std::string{};
    }
    return boost::none;
  }
}
}

#endif
