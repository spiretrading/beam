module;
#include "Prelude.hpp"

export module Beam:ServiceLocatorTestEnvironment;

import :ServiceLocatorClient;

export namespace Beam::Tests {

  /**
   * Wraps most components needed to run an instance of the ServiceLocator with
   * helper functions.
   */
  class ServiceLocatorTestEnvironment {
    public:

      /** Constructs a ServiceLocatorTestEnvironment. */
      ServiceLocatorTestEnvironment();

      ~ServiceLocatorTestEnvironment();

      /** Closes the servlet. */
      void close();

      /** Returns a ServiceLocatorClient logged in as the root account. */
      ServiceLocatorClient& get_root();

      /** Makes a new ServiceLocatorClient. */
      ServiceLocatorClient make_client(
        std::string username, std::string password);

      /** Makes a new ServiceLocatorClient from an existing session. */
      ServiceLocatorClient make_client(
        const std::string& session_id, unsigned int key);

      /** Makes a new ServiceLocatorClient. */
      ServiceLocatorClient make_client();

    private:
      struct Impl;
      std::unique_ptr<Impl> m_impl;

      ServiceLocatorTestEnvironment(
        const ServiceLocatorTestEnvironment&) = delete;
      ServiceLocatorTestEnvironment& operator =(
        const ServiceLocatorTestEnvironment&) = delete;
  };
}
