#include "Beam/Python/Codecs.hpp"
#include "Beam/Codecs/CodedReader.hpp"
#include "Beam/Codecs/CodedWriter.hpp"
#include "Beam/Codecs/DecoderException.hpp"
#include "Beam/Codecs/EncoderException.hpp"
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/Codecs/SizeDeclarativeEncoder.hpp"
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/Python/IO.hpp"
#include "Beam/Python/ToPythonReader.hpp"
#include "Beam/Python/ToPythonWriter.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace pybind11;

namespace {
  auto decoder = std::unique_ptr<class_<Decoder>>();
  auto encoder = std::unique_ptr<class_<Encoder>>();
}

class_<Decoder>& Beam::Python::get_exported_decoder() {
  return *decoder;
}

class_<Encoder>& Beam::Python::get_exported_encoder() {
  return *encoder;
}

void Beam::Python::export_codecs(module& module) {
  decoder = std::make_unique<class_<Decoder>>(
    export_decoder<Decoder>(module, "Decoder"));
  encoder = std::make_unique<class_<Encoder>>(
    export_encoder<Encoder>(module, "Encoder"));
  export_coded_reader(module);
  export_coded_writer(module);
  export_null_decoder(module);
  export_null_encoder(module);
  export_size_declarative_decoder(module);
  export_size_declarative_encoder(module);
  export_zlib_decoder(module);
  export_zlib_encoder(module);
  register_exception<DecoderException>(module, "DecoderException");
  register_exception<EncoderException>(module, "EncoderException");
}

void Beam::Python::export_coded_reader(module& module) {
  export_reader<ToPythonReader<CodedReader<Reader, Decoder>>>(
    module, "CodedReader").
    def(pybind11::init<Reader, Decoder>());
}

void Beam::Python::export_coded_writer(module& module) {
  export_writer<ToPythonWriter<CodedWriter<Writer, Encoder>>>(
    module, "CodedWriter").
    def(pybind11::init<Writer, Encoder>());
}

void Beam::Python::export_null_decoder(module& module) {
  export_decoder<NullDecoder>(module, "NullDecoder");
}

void Beam::Python::export_null_encoder(module& module) {
  export_encoder<NullEncoder>(module, "NullEncoder");
}

void Beam::Python::export_size_declarative_decoder(module& module) {
  export_decoder<SizeDeclarativeDecoder<Decoder>>(
    module, "SizeDeclarativeDecoder").
    def(pybind11::init<Decoder>());
}

void Beam::Python::export_size_declarative_encoder(module& module) {
  export_encoder<SizeDeclarativeEncoder<Encoder>>(
    module, "SizeDeclarativeEncoder").
    def(pybind11::init<Encoder>());
}

void Beam::Python::export_zlib_decoder(module& module) {
  export_decoder<ZLibDecoder>(module, "ZLibDecoder");
}

void Beam::Python::export_zlib_encoder(module& module) {
  export_encoder<ZLibEncoder>(module, "ZLibEncoder");
}
