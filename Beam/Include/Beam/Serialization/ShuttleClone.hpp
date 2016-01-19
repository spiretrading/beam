#ifndef BEAM_SHUTTLECLONE_HPP
#define BEAM_SHUTTLECLONE_HPP
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Serialization {

  //! Constructs a clone using a type's serialization.
  /*!
    \param value The value to clone.
    \param sender A Sender containing a TypeEntry for the value to clone.
    \param receiver A Receiver containing a TypeEntry for the value to clone.
    \return A clone of the <i>value</i> based on its serialization.
  */
  template<typename T, typename SenderType>
  std::unique_ptr<T> ShuttleClone(const T& value, SenderType& sender,
      typename Inverse<SenderType>::type& receiver) {
    static_assert(IsSender<SenderType>::value,
      "SenderType must implement the Sender Concept.");
    typename SenderType::Sink buffer;
    sender.SetSink(Ref(buffer));
    sender.Send(&value);
    receiver.SetSource(Ref(buffer));
    std::unique_ptr<T> clone;
    receiver.Shuttle(clone);
    return clone;
  }
}
}

#endif
