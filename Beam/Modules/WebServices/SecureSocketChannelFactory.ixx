module;
#include "Prelude.hpp"

export module Beam:SecureSocketChannelFactory;

import :SecureSocketChannel;
import :Uri;

export namespace Beam {

  /** Implements a Channel factory for a SecureSocketChannel. */
  class SecureSocketChannelFactory {
    public:

      /**
       * Returns a new SecureSocketChannel.
       * @param uri The URI that the Channel should connect to.
       */
      std::unique_ptr<SecureSocketChannel> operator ()(const Uri& uri) const;
  };

  inline std::unique_ptr<SecureSocketChannel>
      SecureSocketChannelFactory::operator ()(const Uri& url) const {
    auto address = IpAddress(url.get_hostname(), url.get_port());
    return std::make_unique<SecureSocketChannel>(std::move(address));
  }
}

