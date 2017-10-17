#ifndef BEAM_TASKSTATEPARSER_HPP
#define BEAM_TASKSTATEPARSER_HPP
#include "Beam/Collections/EnumIterator.hpp"
#include "Beam/Parsers/EnumeratorParser.hpp"
#include "Beam/Tasks/Task.hpp"
#include "Beam/Tasks/Tasks.hpp"

namespace Beam {
namespace Tasks {

  /*! \class TaskStateParser
      \brief Matches a Task's State.
   */
  class TaskStateParser : public Parsers::EnumeratorParser<Task::State> {
    public:

      //! Constructs a TaskStateParser.
      TaskStateParser();
  };

  inline TaskStateParser::TaskStateParser()
      : Parsers::EnumeratorParser<Task::State>{begin(MakeRange<Task::State>()),
          end(MakeRange<Task::State>()),
          static_cast<std::string (*)(Task::State)>(ToString)} {}
}
}

#endif
