#ifndef BEAM_SCOPED_STREAM_MANIPULATOR_HPP
#define BEAM_SCOPED_STREAM_MANIPULATOR_HPP
#include <ios>
#include <ostream>

namespace Beam {

  /**
   * A scoped IO manipulator that installs a context object of type T into an
   * std::ostream for the duration of an insertion expression.
   * @tparam T The type of the context object to store in the stream.
   */
  template<typename T>
  class ScopedStreamManipulator {
    public:

      /** The type of context object to store in the stream. */
      using Type = T;

      /** Unique slot index in each std::ostream for this context type. */
      inline static const auto ID = std::ios_base::xalloc();

      /**
       * Constructs the manipulator, installing the given context into the
       * provided output stream.
       * @param out The stream into which to install the context.
       * @param context The context object to be stored.
       */
      ScopedStreamManipulator(std::ostream& out, const Type& context) noexcept;

      ~ScopedStreamManipulator();

      operator std::ostream&() const noexcept;

    private:
      std::ostream* m_out;
      void* m_previous_manipulator;

      ScopedStreamManipulator(const ScopedStreamManipulator&) = delete;
      ScopedStreamManipulator& operator =(
        const ScopedStreamManipulator&) = delete;
  };

  template<typename T>
  ScopedStreamManipulator<T>::ScopedStreamManipulator(
      std::ostream& out, const Type& context) noexcept
      : m_out(&out) {
    m_previous_manipulator = m_out->pword(ID);
    m_out->pword(ID) = const_cast<Type*>(&context);
  }

  template<typename T>
  ScopedStreamManipulator<T>::~ScopedStreamManipulator() {
    m_out->pword(ID) = m_previous_manipulator;
  }

  template<typename T>
  ScopedStreamManipulator<T>::operator std::ostream&() const noexcept {
    return *m_out;
  }
}

#endif
