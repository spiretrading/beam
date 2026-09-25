import gc
import struct
import unittest
import weakref
import beam


class TestDecoder(unittest.TestCase):
    def test_copy_lifetime(self):
        for copies in (1, 2):
            with self.subTest(copies=copies):
                underlying = beam.SizeDeclarativeDecoder(beam.NullDecoder())
                reference = weakref.ref(underlying)
                original = beam.Decoder(underlying)
                decoder = original
                for _ in range(copies):
                    decoder = beam.Decoder(decoder)
                del underlying, original
                gc.collect()
                self.assertIsNotNone(reference())
                source = beam.SharedBuffer(struct.pack('!I', 5) + b'hello')
                destination = beam.SharedBuffer()
                self.assertEqual(decoder.decode(source, destination), 5)
                self.assertEqual(destination.get_data(), b'hello')
                del decoder
                gc.collect()
                self.assertIsNone(reference())


class TestSizeDeclarativeDecoder(unittest.TestCase):
    def test_decoder_lifetime(self):
        for erased in (False, True):
            with self.subTest(erased=erased):
                underlying = beam.NullDecoder()
                if erased:
                    underlying = beam.Decoder(underlying)
                reference = weakref.ref(underlying)
                decoder = beam.SizeDeclarativeDecoder(underlying)
                del underlying
                gc.collect()
                self.assertIsNotNone(reference())
                source = beam.SharedBuffer(struct.pack('!I', 5) + b'hello')
                destination = beam.SharedBuffer()
                self.assertEqual(decoder.decode(source, destination), 5)
                self.assertEqual(destination.get_data(), b'hello')
                del decoder
                gc.collect()
                self.assertIsNone(reference())


class TestCodedReader(unittest.TestCase):
    def test_decoder_lifetime(self):
        for erased in (False, True):
            with self.subTest(erased=erased):
                source = beam.SharedBuffer(b'hello')
                reader = beam.BufferReader(source)
                decoder = beam.NullDecoder()
                if erased:
                    reader = beam.Reader(reader)
                    decoder = beam.Decoder(decoder)
                reader_reference = weakref.ref(reader)
                reference = weakref.ref(decoder)
                coded_reader = beam.CodedReader(reader, decoder)
                del source, reader, decoder
                gc.collect()
                self.assertIsNotNone(reader_reference())
                self.assertIsNotNone(reference())
                destination = beam.SharedBuffer()
                self.assertEqual(coded_reader.read(destination), 5)
                self.assertEqual(destination.get_data(), b'hello')
                del coded_reader
                gc.collect()
                self.assertIsNone(reader_reference())
                self.assertIsNone(reference())


if __name__ == '__main__':
    unittest.main()
