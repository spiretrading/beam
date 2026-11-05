#ifndef BEAM_BUFFER_VIEW_HPP
#define BEAM_BUFFER_VIEW_HPP
#include "Beam/IO/Buffer.hpp"

namespace Beam {

  /** A fixed-size, read-only view of externally owned memory. */
  class BufferView {
    public:

      /**
       * Constructs a view of a byte range.
       * @param data The referenced memory, which must outlive the view.
       * @param size The number of bytes in the range.
       */
      BufferView(const void* data, std::size_t size) noexcept;

      const char* get_data() const;
      std::size_t get_size() const;

    private:
      const char* m_data;
      std::size_t m_size;
  };

  inline BufferView::BufferView(const void* data, std::size_t size) noexcept
    : m_data(static_cast<const char*>(data)),
      m_size(size) {}

  inline const char* BufferView::get_data() const {
    return m_data;
  }

  inline std::size_t BufferView::get_size() const {
    return m_size;
  }
}

#endif
