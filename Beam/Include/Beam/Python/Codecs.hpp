#ifndef BEAM_PYTHON_CODECS_HPP
#define BEAM_PYTHON_CODECS_HPP
#include <pybind11/pybind11.h>
#include "Beam/IO/BufferBox.hpp"
#include "Beam/IO/BufferView.hpp"

namespace Beam::Python {

  /**
   * Exports the Codecs namespace.
   * @param module The module to export to.
   */
  void ExportCodecs(pybind11::module& module);

  /**
   * Exports the CodedReader class.
   * @param module The module to export to.
   */
  void ExportCodedReader(pybind11::module& module);

  /**
   * Exports the CodedWriter class.
   * @param module The module to export to.
   */
  void ExportCodedWriter(pybind11::module& module);

  /**
   * Exports the NullDecoder class.
   * @param module The module to export to.
   */
  void ExportNullDecoder(pybind11::module& module);

  /**
   * Exports the NullEncoder class.
   * @param module The module to export to.
   */
  void ExportNullEncoder(pybind11::module& module);

  /**
   * Exports the SizeDeclarativeDecoder class.
   * @param module The module to export to.
   */
  void ExportSizeDeclarativeDecoder(pybind11::module& module);

  /**
   * Exports the SizeDeclarativeEncoder class.
   * @param module The module to export to.
   */
  void ExportSizeDeclarativeEncoder(pybind11::module& module);

  /**
   * Exports the ZLibDecoder class.
   * @param module The module to export to.
   */
  void ExportZLibDecoder(pybind11::module& module);

  /**
   * Exports the ZLibEncoder class.
   * @param module The module to export to.
   */
  void ExportZLibEncoder(pybind11::module& module);

  /**
   * Exports a decoder class.
   * @param <Decoder> The type of decoder to export.
   * @param module The module to export the decoder to.
   * @param name The name of the class.
   */
  template<typename Decoder>
  void ExportDecoder(pybind11::module& module, const std::string& name) {
    pybind11::class_<Decoder>(module, name.c_str())
      .def("decode", static_cast<std::size_t (Decoder::*)(const void*,
        std::size_t, void*, std::size_t)>(&Decoder::Decode))
      .def("decode", static_cast<std::size_t (Decoder::*)(
        const IO::BufferView&, void*, std::size_t)>(
        &Decoder::Decode<IO::BufferView>))
      .def("decode", static_cast<std::size_t (Decoder::*)(const void*,
        std::size_t, Out<IO::BufferBox>)>(&Decoder::Decode<IO::BufferBox>))
      .def("decode", static_cast<std::size_t (Decoder::*)(
        const IO::BufferView&, Out<IO::BufferBox>)>(
        &Decoder::Decode<IO::BufferView, IO::BufferBox>));
  }

  /**
   * Exports an encoder class.
   * @param <Encoder> The type of encoder to export.
   * @param module The module to export the encoder to.
   * @param name The name of the class.
   */
  template<typename Encoder>
  void ExportEncoder(pybind11::module& module, const std::string& name) {
    pybind11::class_<Encoder>(module, name.c_str())
      .def("encode", static_cast<std::size_t (Encoder::*)(const void*,
        std::size_t, void*, std::size_t)>(&Encoder::Encode))
      .def("encode", static_cast<std::size_t (Encoder::*)(
        const IO::BufferView&, void*, std::size_t)>(
        &Encoder::Encode<IO::BufferView>))
      .def("encode", static_cast<std::size_t (Encoder::*)(const void*,
        std::size_t, Out<IO::BufferBox>)>(&Encoder::Encode<IO::BufferBox>))
      .def("encode", static_cast<std::size_t (Encoder::*)(
        const IO::BufferView&, Out<IO::BufferBox>)>(
        &Encoder::Encode<IO::BufferView, IO::BufferBox>));
  }
}

#endif
