#ifndef BEAM_SHUTTLEBIMAP_HPP
#define BEAM_SHUTTLEBIMAP_HPP
#include <boost/bimap.hpp>
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Serialization {
  template<typename K, typename V, typename AP1, typename AP2, typename AP3>
  struct IsStructure<boost::bimap<K, V, AP1, AP2, AP3>> : std::false_type {};

  template<typename K, typename V, typename AP1, typename AP2, typename AP3>
  struct Send<boost::bimap<K, V, AP1, AP2, AP3>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const boost::bimap<K, V, AP1, AP2, AP3>& value) const {
      shuttle.StartSequence(name, static_cast<int>(value.size()));
      for(auto& i : value.left) {
        shuttle.Send(std::make_pair(i.first, i.second));
      }
      shuttle.EndSequence();
    }
  };

  template<typename K, typename V, typename AP1, typename AP2, typename AP3>
  struct Receive<boost::bimap<K, V, AP1, AP2, AP3>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::bimap<K, V, AP1, AP2, AP3>& value) const {
      using Pair = std::pair<
        typename boost::bimap<K, V, AP1, AP2, AP3>::left_key_type,
        typename boost::bimap<K, V, AP1, AP2, AP3>::right_key_type>;
      value.clear();
      int count;
      shuttle.StartSequence(name, count);
      for(auto i = 0; i < count; ++i) {
        Pair entry;
        shuttle.Shuttle(entry);
        value.left.insert(std::move(entry));
      }
      shuttle.EndSequence();
    }
  };
}
}

#endif
