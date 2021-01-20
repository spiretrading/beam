#ifndef BEAM_READ_EVALUATOR_NODE_HPP
#define BEAM_READ_EVALUATOR_NODE_HPP
#include <memory>
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam::Queries {

  /**
   * Reads a value from a pointer.
   * @param <T> The type of data to read.
   */
  template<typename T>
  class ReadEvaluatorNode : public EvaluatorNode<T> {
    public:
      using Result = T;

      /**
       * Constructs a ReadEvaluatorNode.
       * @param value The value to read.
       */
      explicit ReadEvaluatorNode(Result* value);

      Result Eval() override;

    private:
      Result* m_value;
  };

  template<typename TypeList>
  struct ReadEvaluatorNodeTranslator {
    template<typename T>
    static std::unique_ptr<BaseEvaluatorNode> Template(void* address) {
      return std::make_unique<ReadEvaluatorNode<T>>(static_cast<T*>(address));
    }

    using SupportedTypes = TypeList;
  };

  template<typename T>
  ReadEvaluatorNode<T>::ReadEvaluatorNode(Result* value)
    : m_value(value) {}

  template<typename T>
  typename ReadEvaluatorNode<T>::Result ReadEvaluatorNode<T>::Eval() {
    return *m_value;
  }
}

#endif
