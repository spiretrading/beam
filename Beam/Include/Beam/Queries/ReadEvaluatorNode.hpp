#ifndef BEAM_READEVALUATORNODE_HPP
#define BEAM_READEVALUATORNODE_HPP
#include <memory>
#include "Beam/Queries/EvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam {
namespace Queries {

  /*! \class ReadEvaluatorNode
      \brief Reads a value from a pointer.
      \tparam T The type of data to read.
   */
  template<typename T>
  class ReadEvaluatorNode : public EvaluatorNode<T> {
    public:
      using Result = T;

      //! Constructs a ReadEvaluatorNode.
      /*!
        \param value The value to read.
      */
      ReadEvaluatorNode(T* value);

      virtual Result Eval();

    private:
      T* m_value;
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
  ReadEvaluatorNode<T>::ReadEvaluatorNode(T* value)
      : m_value(value) {}

  template<typename T>
  typename ReadEvaluatorNode<T>::Result ReadEvaluatorNode<T>::Eval() {
    return *m_value;
  }
}
}

#endif
