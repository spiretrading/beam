#include "Beam/IOTests/PipedReaderWriterTester.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::IO::Tests;
using namespace Beam::Routines;
using namespace boost;
using namespace std;

void PipedReaderWriterTester::TestWriteThenRead() {
  PipedReader<SharedBuffer> reader;
  PipedWriter<SharedBuffer> writer(Ref(reader));
  RoutineHandler task = Spawn(
    [&] {
      writer.Write(BufferFromString<SharedBuffer>("hello world"));
    });
  SharedBuffer buffer;
  reader.Read(Store(buffer));
  task.Wait();
}
