#ifndef BEAM_WRITE_EVALUATOR_NODE_HPP
#define BEAM_WRITE_EVALUATOR_NODE_HPP
#include <memory>
#include <utility>
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Utilities/Casts.hpp"

namespace Beam::Queries {

  /**
   * Writes a value to a pointer.
   * @param <T> The type of data to write.
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

      Result Eval() override;

    private:
      Result* m_destination;
      std::unique_ptr<EvaluatorNode<Result>> m_value;
  };

  template<typename TypeList>
  struct WriteEvaluatorNodeTranslator {
    template<typename T>
    static std::unique_ptr<BaseEvaluatorNode> Template(void* destination,
        std::unique_ptr<BaseEvaluatorNode> value) {
      return std::make_unique<WriteEvaluatorNode<T>>(
        static_cast<T*>(destination),
        StaticCast<std::unique_ptr<EvaluatorNode<T>>>(std::move(value)));
    }

    using SupportedTypes = TypeList;
  };

  template<typename T>
  WriteEvaluatorNode<T>::WriteEvaluatorNode(Result* destination,
    std::unique_ptr<EvaluatorNode<Result>> value)
    : m_destination(destination),
      m_value(std::move(value)) {}

  template<typename T>
  typename WriteEvaluatorNode<T>::Result WriteEvaluatorNode<T>::Eval() {
    return *m_destination = m_value->Eval();
  }
}

#endif
