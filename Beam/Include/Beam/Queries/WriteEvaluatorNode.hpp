#ifndef BEAM_WRITE_EVALUATOR_NODE_HPP
#define BEAM_WRITE_EVALUATOR_NODE_HPP
#include <memory>
#include <utility>
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Utilities/Casts.hpp"

namespace Beam {

  /**
   * Writes a value to a pointer.
   * @tparam T The type of data to write.
   */
  template<typename T>
  class WriteEvaluatorNode : public EvaluatorNode<T> {
    public:
      using Result = T;

      /**
       * Constructs a WriteEvaluatorNode.
       * @param destination The destination to write to.
       * @param value The value to write.
       */
      WriteEvaluatorNode(
        Result* destination, std::unique_ptr<EvaluatorNode<Result>> value);

      Result eval() override;

    private:
      Result* m_destination;
      std::unique_ptr<EvaluatorNode<Result>> m_value;
  };

  /**
   * Translates a WriteExpression into a WriteEvaluatorNode.
   * @tparam TypeList The list of types supported.
   */
  template<typename TypeList>
  struct WriteEvaluatorNodeTranslator {
    using type = TypeList;

    template<typename T>
    std::unique_ptr<BaseEvaluatorNode> operator ()(
        void* destination, std::unique_ptr<BaseEvaluatorNode> value) const {
      return std::make_unique<WriteEvaluatorNode<T>>(
        static_cast<T*>(destination),
        static_pointer_cast<EvaluatorNode<T>>(std::move(value)));
    }
  };

  template<typename R, typename Q>
  WriteEvaluatorNode(R*, std::unique_ptr<Q>) -> WriteEvaluatorNode<R>;

  template<typename T>
  WriteEvaluatorNode<T>::WriteEvaluatorNode(Result* destination,
    std::unique_ptr<EvaluatorNode<Result>> value)
    : m_destination(destination),
      m_value(std::move(value)) {}

  template<typename T>
  typename WriteEvaluatorNode<T>::Result WriteEvaluatorNode<T>::eval() {
    return *m_destination = m_value->eval();
  }
}

#endif
