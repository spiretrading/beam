#include <iostream>
#include "Beam/Queues/AggregateQueueReader.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace std::chrono_literals;

int main() {
  try {
    auto source = std::make_shared<Queue<int>>();
    auto queues = std::vector<ScopedQueueReader<int>>();
    queues.push_back(source);
    auto queue = AggregateQueueReader(std::move(queues));
    source->Push(123);
    std::cout << "A" << std::endl;
    queue.Pop();
    std::cout << "B" << std::endl;
    source->Push(321);
    std::cout << "C" << std::endl;
    source->Break();
    std::cout << "D" << std::endl;
    queue.Pop();
    std::cout << "E" << std::endl;
    queue.Pop();
    std::cout << "F" << std::endl;
  } catch(const PipeBrokenException&) {
  }
  std::cout << "G" << std::endl;
  return 0;
}
