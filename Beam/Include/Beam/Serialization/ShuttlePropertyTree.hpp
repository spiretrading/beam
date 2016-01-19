#ifndef BEAM_SHUTTLEPROPERTYTREE_HPP
#define BEAM_SHUTTLEPROPERTYTREE_HPP
#include <sstream>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<typename Key, typename Data, typename KeyCompare>
  struct IsStructure<boost::property_tree::basic_ptree<Key, Data, KeyCompare>>
    : std::false_type {};

  template<typename Key, typename Data, typename KeyCompare>
  struct Send<boost::property_tree::basic_ptree<Key, Data, KeyCompare>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const boost::property_tree::basic_ptree<Key, Data, KeyCompare>& value)
        const {
      std::stringstream destination;
      boost::property_tree::write_json(destination, value, false);
      shuttle.Send(name, destination.str());
    }
  };

  template<typename Key, typename Data, typename KeyCompare>
  struct Receive<boost::property_tree::basic_ptree<Key, Data, KeyCompare>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::property_tree::basic_ptree<Key, Data, KeyCompare>& value) const {
      std::string data;
      shuttle.Shuttle(name, data);
      std::stringstream source(data);
      boost::property_tree::read_json(source, value);
    }
  };
}
}

#endif
