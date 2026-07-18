module;
#include "Prelude.hpp"

export module Beam:UidServiceTestEnvironment;

import :UidClient;

export namespace Beam::Tests {

  /**
   * Wraps most components needed to run an instance of the UidService with
   * helper functions.
   */
  class UidServiceTestEnvironment {
    public:

      /** Constructs a UidServiceTestEnvironment. */
      UidServiceTestEnvironment();

      ~UidServiceTestEnvironment();

      /** Makes a UidClient connected to the UidService. */
      UidClient make_client();

      void close();

    private:
      struct Impl;
      std::unique_ptr<Impl> m_impl;

      UidServiceTestEnvironment(const UidServiceTestEnvironment&) = delete;
      UidServiceTestEnvironment& operator =(
        const UidServiceTestEnvironment&) = delete;
  };
}
