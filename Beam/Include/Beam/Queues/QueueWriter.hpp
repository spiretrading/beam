#ifndef BEAM_QUEUEWRITER_HPP
#define BEAM_QUEUEWRITER_HPP
#include "Beam/Queues/BaseQueue.hpp"
#include "Beam/Queues/Queues.hpp"

namespace Beam {

  /*! \class QueueWriter
      \brief Interface for the write-side of a Queue.
      \tparam T The data to write to the Queue.
   */
  template<typename T>
  class QueueWriter : public virtual BaseQueue {
    public:

      //! The type being pushed.
      using Source = T;

      virtual ~QueueWriter() = default;

      //! Adds a value to the end of the Queue.
      /*!
        \param value The value to add to the end of the Queue.
      */
      virtual void Push(const Source& value) = 0;

      //! Adds a value to the end of the Queue.
      /*!
        \param value The value to add to the end of the Queue.
      */
      virtual void Push(Source&& value) = 0;
  };
}

#endif
