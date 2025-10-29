#ifndef BEAM_PYTHON_CODECS_HPP
#define BEAM_PYTHON_CODECS_HPP
#include <pybind11/pybind11.h>
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/IO/BufferRef.hpp"
#include "Beam/Python/Out.hpp"
#include "Beam/Python/Utilities.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam::Python {

  /** Returns the exported Decoder class. */
  BEAM_EXPORT_DLL pybind11::class_<Decoder>& get_exported_decoder();

  /** Returns the exported Encoder class. */
  BEAM_EXPORT_DLL pybind11::class_<Encoder>& get_exported_encoder();

  /**
   * Exports the Codecs namespace.
   * @param module The module to export to.
   */
  void export_codecs(pybind11::module& module);

  /**
   * Exports the CodedReader class.
   * @param module The module to export to.
   */
  void export_coded_reader(pybind11::module& module);

  /**
   * Exports the CodedWriter class.
   * @param module The module to export to.
   */
  void export_coded_writer(pybind11::module& module);

  /**
   * Exports a Decoder type and related free functions.
   * @param module The module to export to.
   * @param name The name of the class to export.
   */
  template<IsDecoder D>
  auto export_decoder(pybind11::module& module, std::string_view name) {
    auto decoder_class = pybind11::class_<D>(module, name.data()).
      def("decode", &D::template decode<BufferRef, BufferRef>,
        pybind11::arg("source"), pybind11::arg("destination")).
      def("__repr__", [name = std::string(name)] (const D& self) {
        return "<" + name + ">";
      });
    export_default_methods(decoder_class);
    if constexpr(!std::is_same_v<D, Decoder>) {
      get_exported_decoder().def(
        pybind11::init<D*>(), pybind11::keep_alive<1, 2>());
      pybind11::implicitly_convertible<D, Decoder>();
    }
    return decoder_class;
  }

  /**
   * Exports an Encoder type and related free functions.
   * @param module The module to export to.
   * @param name The name of the class to export.
   */
  template<IsEncoder E>
  auto export_encoder(pybind11::module& module, std::string_view name) {
    auto encoder_class = pybind11::class_<E>(module, name.data()).
      def("encode", &E::template encode<BufferRef, BufferRef>,
        pybind11::arg("source"), pybind11::arg("destination")).
      def("__repr__", [name = std::string(name)] (const E& self) {
        return "<" + name + ">";
      });
    export_default_methods(encoder_class);
    if constexpr(!std::is_same_v<E, Encoder>) {
      get_exported_encoder().def(
        pybind11::init<E*>(), pybind11::keep_alive<1, 2>());
      pybind11::implicitly_convertible<E, Encoder>();
    }
    return encoder_class;
  }

  /**
   * Exports the NullDecoder class.
   * @param module The module to export to.
   */
  void export_null_decoder(pybind11::module& module);

  /**
   * Exports the NullEncoder class.
   * @param module The module to export to.
   */
  void export_null_encoder(pybind11::module& module);

  /**
   * Exports the SizeDeclarativeDecoder class.
   * @param module The module to export to.
   */
  void export_size_declarative_decoder(pybind11::module& module);

  /**
   * Exports the SizeDeclarativeEncoder class.
   * @param module The module to export to.
   */
  void export_size_declarative_encoder(pybind11::module& module);

  /**
   * Exports the ZLibDecoder class.
   * @param module The module to export to.
   */
  void export_zlib_decoder(pybind11::module& module);

  /**
   * Exports the ZLibEncoder class.
   * @param module The module to export to.
   */
  void export_zlib_encoder(pybind11::module& module);
}

#endif
