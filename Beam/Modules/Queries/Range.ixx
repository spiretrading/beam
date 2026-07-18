module;
#include "Prelude.hpp"

export module Beam:Range;

import :Value;

export namespace Beam {

  /**
   * Defines the range of values to Query over in terms of time or Sequence
   * points.
   */
  class Range {
    public:

      /**
       * Stores a single end-point in a range as either a time or a Sequence.
       */
      using Point = boost::variant<Sequence, boost::posix_time::ptime>;

      /** Returns an empty Range. */
      static const Range EMPTY;

      /** Returns a Range representing all historical Sequences. */
      static const Range HISTORICAL;

      /**
       * Returns a Range representing the entire Sequence including real time
       * data.
       */
      static const Range TOTAL;

      /** Returns a Range representing real time data. */
      static const Range REAL_TIME;

      /** Constructs an empty Range. */
      Range() noexcept;

      /**
       * Constructs a Range given two points.
       * @param start The start of the Range.
       * @param end The end of the Range.
       */
      Range(Point start, Point end) noexcept;

      /**
       * Constructs a Range given two points.
       * @param start The start of the Range.
       * @param end The end of the Range.
       */
      Range(boost::posix_time::special_values start, Point end) noexcept;

      /**
       * Constructs a Range given two points.
       * @param start The start of the Range.
       * @param end The end of the Range.
       */
      Range(Point start, boost::posix_time::special_values end) noexcept;

      /**
       * Constructs a Range given two points.
       * @param start The start of the Range.
       * @param end The end of the Range.
       */
      Range(boost::posix_time::special_values start,
        boost::posix_time::special_values end) noexcept;

      /** Returns the start of the Range. */
      const Point& get_start() const noexcept;

      /** Returns the end of the Range. */
      const Point& get_end() const noexcept;

      bool operator ==(const Range&) const = default;

    private:
      Point m_start;
      Point m_end;

      static bool is_valid(Point point) noexcept;
      static Point validate(Point point) noexcept;
      void initialize(Point start, Point end) noexcept;
  };

  /**
   * Provides access to an object's timestamp field.
   * @tparam T The type of object to access.
   */
  template<typename T>
  struct TimestampAccessor {
    const boost::posix_time::ptime& operator ()(const T& value) const noexcept {
      return value.m_timestamp;
    }

    boost::posix_time::ptime& operator ()(T& value) const noexcept {
      return value.m_timestamp;
    }
  };

  /**
   * Returns a value's timestamp.
   * @param value The value to get the timestamp from.
   * @return The <i>value</i>'s timestamp.
   */
  template<typename T>
  boost::posix_time::ptime& get_timestamp(T& value) noexcept {
    return TimestampAccessor<T>()(value);
  }

  /**
   * Returns a value's timestamp.
   * @param value The value to get the timestamp from.
   * @return The <i>value</i>'s timestamp.
   */
  template<typename T>
  const boost::posix_time::ptime& get_timestamp(const T& value) noexcept {
    return TimestampAccessor<T>()(value);
  }

  /** Defines a comparator for any two timestamped values. */
  struct TimestampComparator {
    template<typename T, typename Q>
    bool operator ()(const T& lhs, const Q& rhs) const noexcept {
      return get_timestamp(lhs) < get_timestamp(rhs);
    }
  };

  /**
   * Tests if a value comes after a given Range Point.
   * @param value The value to test.
   * @param point The Range Point to compare the value to.
   * @return <code>true</code> iff the value comes after the specified
   *         <i>point</i>.
   */
  template<typename T>
  bool range_point_lesser_or_equal(
      const T& value, Range::Point point) noexcept {
    if(auto sequence = boost::get<Sequence>(&point)) {
      return value.get_sequence() <= *sequence;
    }
    auto& timestamp = boost::get<boost::posix_time::ptime>(point);
    return get_timestamp(value) <= timestamp;
  }

  /**
   * Tests if a value comes before a given Range Point.
   * @param value The value to test.
   * @param point The Range Point to compare the value to.
   * @return <code>true</code> iff the value comes before the specified
   *         <i>point</i>.
   */
  template<typename T>
  bool range_point_greater_or_equal(
      const T& value, Range::Point point) noexcept {
    if(auto sequence = boost::get<Sequence>(&point)) {
      return value.get_sequence() >= *sequence;
    }
    auto& timestamp = boost::get<boost::posix_time::ptime>(point);
    return get_timestamp(value) >= timestamp;
  }

  inline std::ostream& operator <<(std::ostream& out, const Range& range) {
    if(range == Range::EMPTY) {
      return out << "Empty";
    } else if(range == Range::TOTAL) {
      return out << "Total";
    }
    return out << '(' << range.get_start() << " " << range.get_end() << ')';
  }

  inline std::size_t hash_value(const Range& range) {
    auto seed = std::size_t(0);
    boost::hash_combine(seed, std::hash<Range::Point>()(range.get_start()));
    boost::hash_combine(seed, std::hash<Range::Point>()(range.get_end()));
    return seed;
  }

  inline bool operator ==(Range::Point range, Sequence sequence) noexcept {
    return boost::get<const Sequence>(&range) &&
      boost::get<Sequence>(range) == sequence;
  }

  inline bool operator !=(Range::Point range, Sequence sequence) noexcept {
    return !(range == sequence);
  }

  inline bool operator ==(
      Range::Point range, boost::posix_time::ptime time) noexcept {
    return boost::get<const boost::posix_time::ptime>(&range) &&
      boost::get<boost::posix_time::ptime>(range) == time;
  }

  inline bool operator !=(
      Range::Point range, const boost::posix_time::ptime& time) noexcept {
    return !(range == time);
  }

  inline const Range Range::EMPTY = Range(Sequence::FIRST, Sequence::FIRST);

  inline const Range Range::HISTORICAL =
    Range(Sequence::FIRST, Sequence::PRESENT);

  inline const Range Range::TOTAL = Range(Sequence::FIRST, Sequence::LAST);

  inline const Range Range::REAL_TIME =
    Range(Sequence::PRESENT, Sequence::LAST);

  inline Range::Range() noexcept
    : m_start(Sequence::FIRST),
      m_end(Sequence::FIRST) {}

  inline Range::Range(Point start, Point end) noexcept {
    initialize(start, end);
  }

  inline Range::Range(
      boost::posix_time::special_values start, Point end) noexcept {
    initialize(boost::posix_time::ptime(start), end);
  }

  inline Range::Range(
      Point start, boost::posix_time::special_values end) noexcept {
    initialize(start, boost::posix_time::ptime(end));
  }

  inline Range::Range(boost::posix_time::special_values start,
      boost::posix_time::special_values end) noexcept {
    initialize(boost::posix_time::ptime(start), boost::posix_time::ptime(end));
  }

  inline const Range::Point& Range::get_start() const noexcept {
    return m_start;
  }

  inline const Range::Point& Range::get_end() const noexcept {
    return m_end;
  }

  inline bool Range::is_valid(Point point) noexcept {
    if(auto time = boost::get<const boost::posix_time::ptime>(&point)) {
      if(time->is_special()) {
        if(*time != boost::posix_time::pos_infin &&
            *time != boost::posix_time::neg_infin) {
          return false;
        }
      }
    }
    return true;
  }

  inline Range::Point Range::validate(Point point) noexcept {
    if(auto pointDate = boost::get<const boost::posix_time::ptime>(&point)) {
      if(*pointDate == boost::posix_time::neg_infin) {
        return Sequence::FIRST;
      } else if(*pointDate == boost::posix_time::pos_infin) {
        return Sequence::LAST;
      } else {
        return point;
      }
    }
    return point;
  }

  inline void Range::initialize(Point start, Point end) noexcept {
    if(!is_valid(start) || !is_valid(end)) {
      m_start = Sequence::FIRST;
      m_end = Sequence::FIRST;
      return;
    }
    m_start = validate(start);
    m_end = validate(end);
  }

  template<>
  struct Send<Beam::Range> {
    template<IsSender S>
    void operator ()(
        S& sender, const Beam::Range& value, unsigned int version) const {
      sender.send("start", value.get_start());
      sender.send("end", value.get_end());
    }
  };

  template<>
  struct Receive<Beam::Range> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, Beam::Range& value, unsigned int version) const {
      auto start = receive<Range::Point>(receiver, "start");
      auto end = receive<Range::Point>(receiver, "end");
      value = Range(start, end);
    }
  };
}

namespace std {
  template<>
  struct hash<Beam::Range> {
    std::size_t operator ()(const Beam::Range& value) const noexcept {
      return Beam::hash_value(value);
    }
  };
}

export namespace Beam {

  /**
   * Stores a value that is part of a Sequence.
   * @tparam T The value's data type.
   */
  template<typename T>
  class SequencedValue {
    public:

      /** The value's data type. */
      using Value = T;

      /** Constructs a SequenceValue. */
      SequencedValue() noexcept(std::is_nothrow_default_constructible_v<T>);

      /**
       * Converts from one type of SequencedValue to another.
       * @param value The value to convert.
       */
      template<typename U>
      SequencedValue(const SequencedValue<U>& value) noexcept(
        std::is_nothrow_constructible_v<T, const U&>);

      /**
       * Constructs a SequencedValue.
       * @param value The value to store.
       * @param sequence The value's Sequence.
       */
      template<typename VF>
      SequencedValue(VF&& value, Sequence sequence) noexcept(
        std::is_nothrow_constructible_v<T, VF&&>);

      /** Returns the Value. */
      const Value& get_value() const;

      /** Returns the Value. */
      Value& get_value();

      /** Returns the Sequence. */
      Sequence get_sequence() const;

      /** Returns the Sequence. */
      Sequence& get_sequence();

      /** Implicitly converts this to the value it represents. */
      operator const Value& () const;

      /** Implicitly converts this to the value it represents. */
      operator Value& ();

      /** Returns a reference to the Value. */
      const Value& operator *() const;

      /** Returns a pointer to the Value. */
      const Value* operator ->() const;

      /** Returns a reference to the Value. */
      Value& operator *();

      /** Returns a pointer to the Value. */
      Value* operator ->();

      bool operator ==(const SequencedValue&) const = default;

    private:
      friend struct Shuttle<SequencedValue<T>>;
      Value m_value;
      Sequence m_sequence;
  };

  template<typename V>
  SequencedValue(V&& value, const Sequence& sequence) ->
    SequencedValue<std::remove_cvref_t<V>>;

  /** Defines a comparator for any two SequencedValues. */
  struct SequenceComparator {
    template<typename T, typename Q>
    bool operator()(const SequencedValue<T>& lhs,
      const SequencedValue<Q>& rhs) const noexcept;
  };

  template<typename T>
  struct TimestampAccessor<SequencedValue<T>> {
    decltype(auto) operator ()(const SequencedValue<T>& value) const {
      return get_timestamp(value.get_value());
    }
  };

  template<typename T>
  inline std::ostream& operator <<(
      std::ostream& out, const SequencedValue<T>& value) {
    return out << '(' << value.get_value() << " " << value.get_sequence() <<
      ')';
  }

  template<typename T>
  SequencedValue<T>::SequencedValue() noexcept(
    std::is_nothrow_default_constructible_v<T>)
    : m_value() {}

  template<typename T>
  template<typename U>
  SequencedValue<T>::SequencedValue(const SequencedValue<U>& value) noexcept(
    std::is_nothrow_constructible_v<T, const U&>)
    : m_value(value.get_value()),
      m_sequence(value.get_sequence()) {}

  template<typename T>
  template<typename VF>
  SequencedValue<T>::SequencedValue(VF&& value, Sequence sequence) noexcept(
    std::is_nothrow_constructible_v<T, VF&&>)
    : m_value(std::forward<VF>(value)),
      m_sequence(sequence) {}

  template<typename T>
  const typename SequencedValue<T>::Value&
      SequencedValue<T>::get_value() const {
    return m_value;
  }

  template<typename T>
  typename SequencedValue<T>::Value& SequencedValue<T>::get_value() {
    return m_value;
  }

  template<typename T>
  Sequence SequencedValue<T>::get_sequence() const {
    return m_sequence;
  }

  template<typename T>
  Sequence& SequencedValue<T>::get_sequence() {
    return m_sequence;
  }

  template<typename T>
  SequencedValue<T>::operator
      const typename SequencedValue<T>::Value& () const {
    return m_value;
  }

  template<typename T>
  SequencedValue<T>::operator typename SequencedValue<T>::Value& () {
    return m_value;
  }

  template<typename T>
  const typename SequencedValue<T>::Value&
      SequencedValue<T>::operator *() const {
    return m_value;
  }

  template<typename T>
  const typename SequencedValue<T>::Value*
      SequencedValue<T>::operator ->() const {
    return &m_value;
  }

  template<typename T>
  typename SequencedValue<T>::Value& SequencedValue<T>::operator *() {
    return m_value;
  }

  template<typename T>
  typename SequencedValue<T>::Value* SequencedValue<T>::operator ->() {
    return &m_value;
  }

  template<typename T, typename Q>
  bool SequenceComparator::operator()(const SequencedValue<T>& lhs,
      const SequencedValue<Q>& rhs) const noexcept {
    return lhs.get_sequence() < rhs.get_sequence();
  }

  template<typename T>
  struct Shuttle<Beam::SequencedValue<T>> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Beam::SequencedValue<T>& value,
        unsigned int version) const {
      shuttle.shuttle("value", value.m_value);
      shuttle.shuttle("sequence", value.m_sequence);
    }
  };
}


export namespace Beam {

  /**
   * Stores a value and its index.
   * @tparam V The value's data type.
   * @tparam I The index's data type.
   */
  template<typename V, typename I>
  class IndexedValue {
    public:

      /** The value's data type. */
      using Value = V;

      /** The index's data type. */
      using Index = I;

      /** Constructs an IndexedValue. */
      IndexedValue() noexcept(std::is_nothrow_default_constructible_v<Value> &&
        std::is_nothrow_default_constructible_v<Index>);

      /**
       * Converts from one type of IndexedValue to another.
       * @param value The value to convert.
       */
      template<typename U, typename K>
      IndexedValue(const IndexedValue<U, K>& value) noexcept(
        std::is_nothrow_constructible_v<Value, const U&> &&
          std::is_nothrow_constructible_v<Index, const K&>);

      /**
       * Constructs an IndexedValue.
       * @param value The value to store.
       * @param index The <i>value</i>'s index.
       */
      template<typename VF, typename IF>
      IndexedValue(VF&& value, IF&& index) noexcept(
        std::is_nothrow_constructible_v<Value, VF&&> &&
          std::is_nothrow_constructible_v<Index, IF&&>);

      /** Returns the Value. */
      Value& get_value();

      /** Returns the Value. */
      const Value& get_value() const;

      /** Returns the Index. */
      Index& get_index();

      /** Returns the Index. */
      const Index& get_index() const;

      /** Implicitly converts this to the value it represents. */
      operator const Value& () const;

      /** Implicitly converts this to the value it represents. */
      operator Value& ();

      /** Returns a reference to the Value. */
      const Value& operator *() const;

      /** Returns a pointer to the Value. */
      const Value* operator ->() const;

      /** Returns a reference to the Value. */
      Value& operator *();

      /** Returns a pointer to the Value. */
      Value* operator ->();

      bool operator ==(const IndexedValue&) const = default;

    private:
      friend struct Shuttle<IndexedValue<V, I>>;
      Value m_value;
      Index m_index;
  };

  template<typename V, typename I>
  IndexedValue(V&& value, I&& index) ->
    IndexedValue<std::remove_cvref_t<V>, std::remove_cvref_t<I>>;

  template<typename Value, typename Index>
  struct TimestampAccessor<IndexedValue<Value, Index>> {
    decltype(auto) operator ()(const IndexedValue<Value, Index>& value) const {
      return get_timestamp(value.get_value());
    }
  };

  template<typename Value, typename Index>
  inline std::ostream& operator <<(
      std::ostream& out, const IndexedValue<Value, Index>& value) {
    return out << '(' << value.get_index() << ' ' << value.get_value() << ')';
  }

  template<typename V, typename I>
  IndexedValue<V, I>::IndexedValue() noexcept(
      std::is_nothrow_default_constructible_v<Value> &&
        std::is_nothrow_default_constructible_v<Index>)
    : m_value(),
      m_index() {}

  template<typename V, typename I>
  template<typename U, typename K>
  IndexedValue<V, I>::IndexedValue(const IndexedValue<U, K>& value) noexcept(
      std::is_nothrow_constructible_v<Value, const U&> &&
        std::is_nothrow_constructible_v<Index, const K&>)
    : m_value(value.get_value()),
      m_index(value.get_index()) {}

  template<typename V, typename I>
  template<typename VF, typename IF>
  IndexedValue<V, I>::IndexedValue(VF&& value, IF&& index) noexcept(
      std::is_nothrow_constructible_v<Value, VF&&> &&
        std::is_nothrow_constructible_v<Index, IF&&>)
    : m_value(std::forward<VF>(value)),
      m_index(std::forward<IF>(index)) {}

  template<typename V, typename I>
  typename IndexedValue<V, I>::Value& IndexedValue<V, I>::get_value() {
    return m_value;
  }

  template<typename V, typename I>
  const typename IndexedValue<V, I>::Value&
      IndexedValue<V, I>::get_value() const {
    return m_value;
  }

  template<typename V, typename I>
  typename IndexedValue<V, I>::Index& IndexedValue<V, I>::get_index() {
    return m_index;
  }

  template<typename V, typename I>
  const typename IndexedValue<V, I>::Index&
      IndexedValue<V, I>::get_index() const {
    return m_index;
  }

  template<typename V, typename I>
  IndexedValue<V, I>::operator
      const typename IndexedValue<V, I>::Value& () const {
    return m_value;
  }

  template<typename V, typename I>
  IndexedValue<V, I>::operator typename IndexedValue<V, I>::Value& () {
    return m_value;
  }

  template<typename V, typename I>
  const typename IndexedValue<V, I>::Value&
      IndexedValue<V, I>::operator *() const {
    return m_value;
  }

  template<typename V, typename I>
  const typename IndexedValue<V, I>::Value*
      IndexedValue<V, I>::operator ->() const {
    return &m_value;
  }

  template<typename V, typename I>
  typename IndexedValue<V, I>::Value& IndexedValue<V, I>::operator *() {
    return m_value;
  }

  template<typename V, typename I>
  typename IndexedValue<V, I>::Value* IndexedValue<V, I>::operator ->() {
    return &m_value;
  }

  template<typename Value, typename Index>
  struct Shuttle<Beam::IndexedValue<Value, Index>> {
    template<IsShuttle S>
    void operator ()(S& shuttle, Beam::IndexedValue<Value, Index>& value,
        unsigned int version) const {
      shuttle.shuttle("value", value.m_value);
      shuttle.shuttle("index", value.m_index);
    }
  };
}


