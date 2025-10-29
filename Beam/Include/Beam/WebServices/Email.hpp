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

namespace Beam {
namespace Details {
  inline std::string format_date(boost::posix_time::ptime date) {
    auto ss = std::stringstream();
    ss << date.date().day_of_week().as_short_string() << ", " <<
      date.date().day() << ' ' << date.date().month().as_short_string() <<
      ' ' << date.date().year() << ' ';
    if(date.time_of_day().hours() < 10) {
      ss << '0';
    }
    ss << date.time_of_day().hours() << ':';
    if(date.time_of_day().minutes() < 10) {
      ss << '0';
    }
    ss << date.time_of_day().minutes() << ':';
    if(date.time_of_day().seconds() < 10) {
      ss << '0';
    }
    ss << date.time_of_day().seconds();
    auto time_zone = date.zone_abbrev(true);
    if(time_zone.empty()) {
      ss << " +0000";
    } else {
      ss << ' ' << time_zone;
    }
    return ss.str();
  }

  inline std::string dot_stuff(const std::string& source) {
    auto body = std::string();
    auto is_new_line = true;
    for(auto c : source) {
      if(is_new_line && c == '.') {
        body += "..";
      } else {
        body += c;
      }
      is_new_line = c == '\n';
    }
    return body;
  }
}

  /** Represents an email message. */
  class Email {
    public:

      /** Stores an email header. */
      struct Header {

        /** The header name. */
        std::string m_name;

        /** The header value. */
        std::string m_value;
      };

      /** Stores the body of the message. */
      struct Body {

        /** The content type. */
        std::string m_content_type;

        /** The message. */
        std::string m_message;
      };

      /**
       * Constructs an empty email.
       * @param from The from header.
       */
      explicit Email(EmailAddress from);

      /**
       * Constructs an empty email.
       * @param from The from header.
       * @param to The recipient.
       */
      Email(EmailAddress from, EmailAddress to);

      /** Returns the from address. */
      const EmailAddress& get_from() const;

      /** Sets the from address. */
      void set_from(const EmailAddress& address);

      /** Returns the date when the message was written. */
      boost::posix_time::ptime get_date() const;

      /** Sets the date. */
      void set_date(boost::posix_time::ptime date);

      /** Returns the sender address. */
      const boost::optional<EmailAddress>& get_sender() const;

      /** Sets the sender address. */
      void set_sender(const EmailAddress& address);

      /** Returns the to addresses. */
      const std::vector<EmailAddress>& get_to() const;

      /** Adds a recipient. */
      void add_to(EmailAddress address);

      /** Returns the subject. */
      const std::string& get_subject() const;

      /** Sets the subject. */
      void set_subject(const std::string& subject);

      /** Returns the bodies. */
      const std::vector<Body>& get_bodies() const;

      /** Adds a body. */
      void add_body(std::string message);

      /**
       * Adds a body.
       * @param content_type The content type.
       * @param message The message.
       */
      void add_body(std::string content_type, std::string message);

      /**
       * Finds a header.
       * @param name The name of the header to find.
       * @return The header value.
       */
      boost::optional<std::string> find_header(const std::string& name) const;

      /** Returns all additional headers. */
      const std::vector<Header>& get_additional_headers() const;

      /**
       * Sets a header.
       * @param name The name of the header to set.
       * @param value The header's value.
       */
      void set_header(std::string name, std::string value);

    private:
      EmailAddress m_from;
      boost::posix_time::ptime m_date;
      boost::optional<EmailAddress> m_sender;
      std::vector<EmailAddress> m_to;
      std::string m_subject;
      std::vector<Header> m_additional_headers;
      std::vector<Body> m_bodies;
      std::string m_body_boundary;
  };

  inline std::ostream& operator <<(std::ostream& sink, const Email& email) {
    sink << "From: " << email.get_from() << "\r\n";
    sink << "Date: " << Details::format_date(email.get_date()) << "\r\n";
    if(email.get_sender().is_initialized()) {
      sink << "Sender: " << *email.get_sender() << "\r\n";
    }
    auto& to = email.get_to();
    if(!to.empty()) {
      sink << "To: ";
      auto is_first = true;
      for(auto& recipient : to) {
        if(!is_first) {
          sink << ", ";
        }
        sink << recipient;
        is_first = false;
      }
      sink << "\r\n";
    }
    if(!email.get_subject().empty()) {
      sink << "Subject: " << email.get_subject() << "\r\n";
    }
    for(auto& header : email.get_additional_headers()) {
      sink << header.m_name << ": " << header.m_value << "\r\n";
    }
    auto& bodies = email.get_bodies();
    auto boundary = std::string();
    if(bodies.size() == 1) {
      sink << "Content-Type: " << bodies.front().m_content_type << "\r\n";
    } else if(bodies.size() > 1) {
      for(auto i = 0; i < 45; ++i) {
        boundary += 'a' + static_cast<char>(rand() % 26);
      }
      sink << "Content-Type: multipart/alternative; boundary=\"" << boundary <<
        "\"\r\n";
    }
    sink << "\r\n";
    if(bodies.size() == 1) {
      sink << Details::dot_stuff(bodies.front().m_message);
    } else {
      for(auto& body : bodies) {
        sink << "--" << boundary << "\r\n";
        sink << "Content-Type: " << body.m_content_type << "\r\n\r\n";
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
      : m_from(std::move(from)),
        m_date(boost::posix_time::second_clock::universal_time()) {
    set_header("Mime-Version", "1.0");
  }

  inline Email::Email(EmailAddress from, EmailAddress to)
      : Email(std::move(from)) {
    m_to.push_back(std::move(to));
  }

  inline const EmailAddress& Email::get_from() const {
    return m_from;
  }

  inline void Email::set_from(const EmailAddress& address) {
    m_from = address;
  }

  inline boost::posix_time::ptime Email::get_date() const {
    return m_date;
  }

  inline void Email::set_date(boost::posix_time::ptime date) {
    m_date = date;
  }

  inline const boost::optional<EmailAddress>& Email::get_sender() const {
    return m_sender;
  }

  inline void Email::set_sender(const EmailAddress& address) {
    m_sender = address;
  }

  inline const std::vector<EmailAddress>& Email::get_to() const {
    return m_to;
  }

  inline void Email::add_to(EmailAddress address) {
    m_to.push_back(std::move(address));
  }

  inline const std::string& Email::get_subject() const {
    return m_subject;
  }

  inline void Email::set_subject(const std::string& subject) {
    m_subject = subject;
  }

  inline const std::vector<Email::Body>& Email::get_bodies() const {
    return m_bodies;
  }

  inline void Email::add_body(std::string message) {
    add_body("text/plain", std::move(message));
  }

  inline void Email::add_body(std::string content_type, std::string message) {
    m_bodies.push_back(Body(std::move(content_type), std::move(message)));
  }

  inline boost::optional<std::string> Email::find_header(
      const std::string& name) const {
    auto header_iterator = std::find_if(m_additional_headers.begin(),
      m_additional_headers.end(), [&] (auto& header) {
        return boost::algorithm::iequals(name, header.m_name);
      });
    if(header_iterator == m_additional_headers.end()) {
      return boost::none;
    }
    return header_iterator->m_value;
  }

  inline const std::vector<Email::Header>&
      Email::get_additional_headers() const {
    return m_additional_headers;
  }

  inline void Email::set_header(std::string name, std::string value) {
    auto header_iterator = std::find_if(m_additional_headers.begin(),
      m_additional_headers.end(), [&] (auto& header) {
        return boost::algorithm::iequals(name, header.m_name);
      });
    if(header_iterator == m_additional_headers.end()) {
      m_additional_headers.push_back(Header(std::move(name), std::move(value)));
    } else {
      header_iterator->m_value = std::move(value);
    }
  }
}

#endif
