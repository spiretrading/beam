#include "Beam/Python/IO.hpp"
#include "Beam/IOTests/TestReader.hpp"
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/QueueWriter.hpp"
#include "Beam/Python/ServiceResult.hpp"
#include "Beam/Python/SharedObject.hpp"
#include "Beam/Python/ToPythonReader.hpp"
#include "Beam/Queues/CallbackQueueWriter.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Tests;
using namespace pybind11;

namespace {
  using PythonReader = ToPythonReader<TestReader>;
  using Operation = TestReader::Operation;

  std::unique_ptr<PythonReader> make_reader(
      std::shared_ptr<QueueWriter<object>> queue) {
    auto operations = SharedObject(cast(queue));
    return std::make_unique<PythonReader>(callback<std::shared_ptr<Operation>>(
      [=] (const auto& operation) {
        auto lock = GilLock();
        std::visit([&] (auto& value) {
          using Value = std::remove_cvref_t<decltype(value)>;
          operations->cast<QueueWriter<object>&>().push(
            cast(std::shared_ptr<Value>(operation, &value)));
        }, *operation);
      }, [=] (const std::exception_ptr& e) {
        auto lock = GilLock();
        operations->cast<QueueWriter<object>&>().close(e);
      }));
  }
}

void Beam::Python::export_test_reader(module& module) {
  export_service_result<bool>(module, "BoolServiceResult");
  export_service_result<SharedBuffer>(module, "BufferServiceResult");
  auto reader = export_reader<PythonReader>(module, "TestReader");
  reader.def(init(&make_reader), arg("operations")).
    def("close", [] (PythonReader& self) {
      self.get().close();
    }, call_guard<GilRelease>());
  using Poll = TestReader::PollOperation;
  class_<Poll, std::shared_ptr<Poll>>(reader, "PollOperation").
    def_readonly("result", &Poll::m_result);
  using Read = TestReader::ReadOperation;
  class_<Read, std::shared_ptr<Read>>(reader, "ReadOperation").
    def_readonly("size", &Read::m_size).
    def_readonly("result", &Read::m_result);
}
