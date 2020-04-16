#ifndef BEAM_TESTSERVICES_HPP
#define BEAM_TESTSERVICES_HPP
#include "Beam/Services/Service.hpp"
#include "Beam/ServicesTests/ServicesTests.hpp"

namespace Beam::Services::Tests {
  BEAM_DEFINE_SERVICES(TestServices,
    (VoidService, "Beam.Services.Tests.VoidService", void, int, n),
    (IdentityService, "Beam.Services.Tests.IdentityService", int, int, n));
}

#endif
