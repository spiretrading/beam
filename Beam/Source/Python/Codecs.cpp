#include "Beam/Python/Codecs.hpp"
#include "Beam/Codecs/CodedReader.hpp"
#include "Beam/Codecs/CodedWriter.hpp"
#include "Beam/Codecs/DecoderBox.hpp"
#include "Beam/Codecs/DecoderException.hpp"
#include "Beam/Codecs/EncoderBox.hpp"
#include "Beam/Codecs/EncoderException.hpp"
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/Codecs/SizeDeclarativeEncoder.hpp"
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/Python/Out.hpp"
#include "Beam/Python/ToPythonReader.hpp"
#include "Beam/Python/ToPythonWriter.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Python;
using namespace Beam::Codecs;
using namespace pybind11;

namespace {
  auto encoderBox = std::unique_ptr<class_<EncoderBox>>();
  auto decoderBox = std::unique_ptr<class_<DecoderBox>>();
}

BEAM_EXPORT_DLL class_<EncoderBox>& Beam::Python::GetExportedEncoderBox() {
  return *encoderBox;
}

BEAM_EXPORT_DLL class_<DecoderBox>& Beam::Python::GetExportedDecoderBox() {
  return *decoderBox;
}

void Beam::Python::ExportCodecs(module& module) {
  auto submodule = module.def_submodule("codecs");
  ExportCodedReader(submodule);
  ExportCodedWriter(submodule);
  decoderBox = std::make_unique<class_<DecoderBox>>(
    ExportDecoder<DecoderBox>(module, "Decoder"));
  encoderBox = std::make_unique<class_<EncoderBox>>(
    ExportEncoder<EncoderBox>(module, "Encoder"));
  ExportNullDecoder(submodule);
  ExportNullEncoder(submodule);
  ExportSizeDeclarativeDecoder(submodule);
  ExportSizeDeclarativeEncoder(submodule);
  ExportZLibDecoder(submodule);
  ExportZLibEncoder(submodule);
  register_exception<DecoderException>(submodule, "DecoderException");
  register_exception<EncoderException>(submodule, "EncoderException");
}

void Beam::Python::ExportCodedReader(pybind11::module& module) {}

void Beam::Python::ExportCodedWriter(pybind11::module& module) {}

void Beam::Python::ExportNullDecoder(module& module) {
  ExportDecoder<NullDecoder>(module, "NullDecoder")
    .def(init());
}

void Beam::Python::ExportNullEncoder(module& module) {
  ExportEncoder<NullEncoder>(module, "NullEncoder")
    .def(init());
}

void Beam::Python::ExportSizeDeclarativeDecoder(module& module) {
  ExportDecoder<SizeDeclarativeDecoder<DecoderBox>>(module,
    "SizeDeclarativeDecoder")
    .def(init<DecoderBox>());
}

void Beam::Python::ExportSizeDeclarativeEncoder(module& module) {
  ExportEncoder<SizeDeclarativeEncoder<EncoderBox>>(module,
    "SizeDeclarativeEncoder")
    .def(init<EncoderBox>());
}

void Beam::Python::ExportZLibDecoder(module& module) {
  ExportDecoder<ZLibDecoder>(module, "ZLibDecoder")
    .def(init());
}

void Beam::Python::ExportZLibEncoder(module& module) {
  ExportEncoder<ZLibEncoder>(module, "ZLibEncoder")
    .def(init());
}
