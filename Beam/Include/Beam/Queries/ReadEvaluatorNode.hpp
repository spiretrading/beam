#ifndef BEAM_READ_EVALUATOR_NODE_HPP
#define BEAM_READ_EVALUATOR_NODE_HPP
#include <memory>
#include "Beam/Queries/EvaluatorNode.hpp"

namespace Beam {

  /**
   * Reads a value from a pointer.
   * @tparam T The type of data to read.
   */
  template<typename T>
  class ReadEvaluatorNode : public EvaluatorNode<T> {
    public:
      using Result = T;

      /**
       * Constructs a ReadEvaluatorNode.
       * @param value The value to read.
       */
      explicit ReadEvaluatorNode(Result* value) noexcept;

      Result eval() override;

    private:
      Result* m_value;
  };

  /**
   * Translates a ReadExpression into a ReadEvaluatorNode.
   * @tparam TypeList The list of types supported.
   */
  template<typename TypeList>
  struct ReadEvaluatorNodeTranslator {
    using type = TypeList;

    template<typename T>
    std::unique_ptr<BaseEvaluatorNode> operator ()(void* address) const {
      return std::make_unique<ReadEvaluatorNode<T>>(static_cast<T*>(address));
    }
  };

  template<typename T>
  ReadEvaluatorNode<T>::ReadEvaluatorNode(Result* value) noexcept
    : m_value(value) {}

  template<typename T>
  typename ReadEvaluatorNode<T>::Result ReadEvaluatorNode<T>::eval() {
    return *m_value;
  }
}

#endif
