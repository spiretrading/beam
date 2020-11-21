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

void Beam::Python::ExportCodecs(module& module) {
  auto submodule = module.def_submodule("codecs");
  ExportCodedReader(submodule);
  ExportCodedWriter(submodule);
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
    "SizeDeclarativeDecoder");
}

void Beam::Python::ExportSizeDeclarativeEncoder(module& module) {
  ExportEncoder<SizeDeclarativeEncoder<EncoderBox>>(module,
    "SizeDeclarativeEncoder");
}

void Beam::Python::ExportZLibDecoder(module& module) {
  ExportDecoder<ZLibDecoder>(module, "ZLibDecoder")
    .def(init());
}

void Beam::Python::ExportZLibEncoder(module& module) {
  ExportEncoder<ZLibEncoder>(module, "ZLibEncoder")
    .def(init());
}
