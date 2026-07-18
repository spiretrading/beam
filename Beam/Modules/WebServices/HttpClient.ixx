module;
#include "Prelude.hpp"

export module Beam:HttpClient;

import :HttpRequest;
import :HttpResponseParser;
import :Uri;

export namespace Beam {

  /**
   * A client that can submit HTTP requests to a server.
   * @tparam C The type Channel used to connect to the server.
   */
  template<typename C> requires IsChannel<dereference_t<C>>
  class HttpClient {
    public:

      /** The type of Channel used to connect to the server. */
      using Channel = dereference_t<C>;

      /**
       * The factory used to build Channels.
       * @param uri The URI to connect to.
       * @return A Channel able to connect to the specified uri.
       */
      using ChannelBuilder = std::function<C (const Uri& uri)>;

      /**
       * Constructs an HttpClient.
       * @param channel_builder Constructs the Channel used to connect to the
       *        server.
       */
      explicit HttpClient(ChannelBuilder channel_builder);

      /**
       * Sends a request.
       * @param request The HttpRequest to send.
       * @return The response to the specified request.
       */
      HttpResponse send(const HttpRequest& request);

    private:
      struct ChannelEntry {
        IpAddress m_end;
        C m_channel;

        ChannelEntry(IpAddress end, C channel);
      };
      std::unordered_map<std::string, std::vector<Cookie>> m_cookies;
      ChannelBuilder m_channel_builder;
      boost::optional<ChannelEntry> m_channel;

      static SharedBuffer decompress(const SharedBuffer& body, int window_bits);
      HttpClient(const HttpClient&) = delete;
      HttpClient& operator =(const HttpClient&) = delete;
  };

  template<typename F>
  HttpClient(F) -> HttpClient<std::invoke_result_t<F, const Uri&>>;

  template<typename C> requires IsChannel<dereference_t<C>>
  HttpClient<C>::ChannelEntry::ChannelEntry(IpAddress end, C channel)
    : m_end(std::move(end)),
      m_channel(std::move(channel)) {}

  template<typename C> requires IsChannel<dereference_t<C>>
  HttpClient<C>::HttpClient(ChannelBuilder channel_builder)
    : m_channel_builder(std::move(channel_builder)) {}

  template<typename C> requires IsChannel<dereference_t<C>>
  HttpResponse HttpClient<C>::send(const HttpRequest& request) {
    auto end =
      IpAddress(request.get_uri().get_hostname(), request.get_uri().get_port());
    auto is_new_channel = false;
    auto& host_cookies = m_cookies[request.get_uri().get_hostname()];
    auto modified_request = boost::optional<HttpRequest>();
    auto& proper_request = [&] () -> const HttpRequest& {
      if(!request.get_header("Accept-Encoding") || !host_cookies.empty()) {
        modified_request.emplace(request);
        if(!request.get_header("Accept-Encoding")) {
          modified_request->add(HttpHeader("Accept-Encoding", "gzip, deflate"));
        }
        for(auto& host_cookie : host_cookies) {
          modified_request->add(host_cookie);
        }
        return *modified_request;
      }
      return request;
    }();
    if(!m_channel || m_channel->m_end != end) {
      m_channel.reset();
      auto channel = m_channel_builder(request.get_uri());
      m_channel.emplace(end, std::move(channel));
      is_new_channel = true;
    }
    {
      auto write_buffer = SharedBuffer();
      proper_request.encode(out(write_buffer));
      try {
        m_channel->m_channel->get_writer().write(write_buffer);
      } catch(const std::exception&) {
        m_channel.reset();
        if(is_new_channel) {
          throw;
        }
        return send(request);
      }
    }
    auto parser = HttpResponseParser();
    auto response = parser.get_next_response();
    while(!response) {
      auto read_buffer = SharedBuffer();
      try {
        m_channel->m_channel->get_reader().read(out(read_buffer));
      } catch(const std::exception&) {
        m_channel.reset();
        throw;
      }
      parser.feed(
        std::string_view(read_buffer.get_data(), read_buffer.get_size()));
      response = parser.get_next_response();
    }
    if(auto encoding = response->get_header("Content-Encoding")) {
      if(boost::iequals(*encoding, "gzip")) {
        response->set_body(decompress(response->get_body(), 16 + MAX_WBITS));
      } else if(boost::iequals(*encoding, "deflate")) {
        response->set_body(decompress(response->get_body(), -MAX_WBITS));
      }
    }
    if(auto connection_header = response->get_header("Connection")) {
      if(!boost::iequals(*connection_header, "keep-alive")) {
        m_channel->m_channel->get_connection().close();
        m_channel.reset();
      }
    } else if(response->get_version() == HttpVersion::version_1_0()) {
      m_channel->m_channel->get_connection().close();
      m_channel.reset();
    }
    auto& cookies = response->get_cookies();
    for(auto& cookie : cookies) {
      auto is_found = false;
      for(auto& host_cookie : host_cookies) {
        if(host_cookie.get_name() == cookie.get_name()) {
          host_cookie.set_value(cookie.get_value());
          is_found = true;
          break;
        }
      }
      if(!is_found) {
        host_cookies.push_back(cookie);
      }
    }
    return *response;
  }

  template<typename C> requires IsChannel<dereference_t<C>>
  SharedBuffer HttpClient<C>::decompress(
      const SharedBuffer& body, int window_bits) {
    auto stream = z_stream();
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = static_cast<uInt>(body.get_size());
    stream.next_in =
      reinterpret_cast<Bytef*>(const_cast<char*>(body.get_data()));
    if(inflateInit2(&stream, window_bits) != Z_OK) {
      boost::throw_with_location(
        std::runtime_error("Failed to initialize decompression."));
    }
    auto output = SharedBuffer();
    auto chunk = std::array<char, 16384>();
    auto status = 0;
    do {
      stream.avail_out = static_cast<uInt>(chunk.size());
      stream.next_out = reinterpret_cast<Bytef*>(chunk.data());
      status = inflate(&stream, Z_NO_FLUSH);
      if(status == Z_STREAM_ERROR || status == Z_DATA_ERROR ||
          status == Z_MEM_ERROR) {
        inflateEnd(&stream);
        boost::throw_with_location(
          std::runtime_error("Failed to decompress response body."));
      }
      auto have = chunk.size() - stream.avail_out;
      if(have > 0) {
        append(output, chunk.data(), have);
      }
    } while(status != Z_STREAM_END);
    inflateEnd(&stream);
    return output;
  }
}

