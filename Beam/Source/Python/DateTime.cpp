#include "Beam/Python/DateTime.hpp"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Python/BoostPython.hpp"
#include <datetime.h>

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace boost::python;
using namespace std;

namespace {
  struct PtimeToPython {
    static PyObject* convert(const ptime& value) {
      auto day = value.date();
      auto timeOfDay = value.time_of_day();
      auto usec = static_cast<int>(timeOfDay.total_microseconds() % 1000000);
      auto result = PyDateTime_FromDateAndTime(day.year(), day.month(),
        day.day(), static_cast<int>(timeOfDay.hours()),
        static_cast<int>(timeOfDay.minutes()),
        static_cast<int>(timeOfDay.seconds()), usec);
      return incref(result);
    }
  };

  struct PtimeFromPythonConverter {
    static void* convertible(PyObject* object) {
      if(object == Py_None || PyDateTime_Check(object)) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto storage = reinterpret_cast<converter::rvalue_from_python_storage<
          ptime>*>(data)->storage.bytes;
      if(object == Py_None) {
        new(storage) ptime{not_a_date_time};
      } else {
        date d(PyDateTime_GET_YEAR(object), PyDateTime_GET_MONTH(object),
          PyDateTime_GET_DAY(object));
        time_duration timeOfDay(PyDateTime_DATE_GET_HOUR(object),
          PyDateTime_DATE_GET_MINUTE(object),
          PyDateTime_DATE_GET_SECOND(object));
        timeOfDay += microsec(PyDateTime_DATE_GET_MICROSECOND(object));
        new(storage) ptime(d, timeOfDay);
      }
      data->convertible = storage;
    }
  };

  struct TimeDurationToPython {
    static PyObject* convert(const time_duration& value) {
      auto totalMicroseconds = std::abs(value.total_microseconds());
      auto totalSeconds = totalMicroseconds / 1000000;
      auto days = totalSeconds / 86400;
      auto seconds = totalSeconds - (86400 * days);
      auto microseconds = totalMicroseconds - 1000000 * totalSeconds;
      if(days != 0) {
        if(totalMicroseconds < 0) {
          days = -days;
        }
      } else if(seconds != 0) {
        if(totalMicroseconds < 0) {
          seconds = -seconds;
        }
      } else {
        if(totalMicroseconds < 0) {
          microseconds = -microseconds;
        }
      }
      return PyDelta_FromDSU(static_cast<int>(days), static_cast<int>(seconds),
        static_cast<int>(microseconds));
    }
  };

  struct TimeDurationFromPythonConverter {
    static void* convertible(PyObject* object) {
      if(PyDelta_Check(object)) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      auto timeDelta = reinterpret_cast<PyDateTime_Delta*>(object);
      auto days = timeDelta->days;
      auto isNegative = (days < 0);
      if(isNegative) {
        days = -days;
      }
      time_duration duration = hours(24) * days + seconds(timeDelta->seconds) +
        microseconds(timeDelta->microseconds);
      if(isNegative) {
        duration = duration.invert_sign();
      }
      auto storage = reinterpret_cast<
        converter::rvalue_from_python_storage<time_duration>*>(
        data)->storage.bytes;
      new(storage) time_duration(duration);
      data->convertible = storage;
    }
  };
}

void Beam::Python::ExportPtime() {
  PyDateTime_IMPORT;
  to_python_converter<ptime, PtimeToPython>();
  converter::registry::push_back(&PtimeFromPythonConverter::convertible,
    &PtimeFromPythonConverter::construct, type_id<ptime>());
}

void Beam::Python::ExportTimeDuration() {
  to_python_converter<time_duration, TimeDurationToPython>();
  converter::registry::push_back(&TimeDurationFromPythonConverter::convertible,
    &TimeDurationFromPythonConverter::construct, type_id<time_duration>());
}
