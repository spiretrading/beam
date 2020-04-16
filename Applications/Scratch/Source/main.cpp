#include <Beam/Threading/Sync.hpp>
#include "Version.hpp"

using namespace Beam;
using namespace Beam::Threading;

int main(int argc, const char** argv) {
  auto s = Sync<int>(5);
  auto y = s;
  return 0;
}
