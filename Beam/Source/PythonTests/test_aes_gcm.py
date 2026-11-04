import gc
import threading
import unittest
import weakref
import beam


PRIVATE_KEY = """-----BEGIN PRIVATE KEY-----
MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQglWOQIPNZYrr3ez7w
MD5pqZ1MAkC4n8sko0y4+7rMCTehRANCAATIIaCwH1WHFWi3/bfB3OWVQmed7ijj
er5QB+8ikyklveHL3XBqaX58GY0ZtbLeDlHqMpGk7TfD4qAliecBCIFV
-----END PRIVATE KEY-----
"""
PUBLIC_KEY = """-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEyCGgsB9VhxVot/23wdzllUJnne4o
43q+UAfvIpMpJb3hy91waml+fBmNGbWy3g5R6jKRpO03w+KgJYnnAQiBVQ==
-----END PUBLIC KEY-----
"""


class TestAesGcmCodec(unittest.TestCase):
    def test_known_vector(self):
        key = beam.SharedBuffer(bytes(32))
        nonce = beam.SharedBuffer(bytes(12))
        encoder = beam.AesGcmEncoder(key, nonce)
        decoder = beam.AesGcmDecoder(key, nonce)
        plaintext = beam.SharedBuffer(bytes(16))
        encoded = beam.SharedBuffer()
        self.assertEqual(encoder.encode(plaintext, encoded), 32)
        self.assertEqual(encoded.get_data(), bytes.fromhex(
            'cea7403d4d606b6e074ec5d3baf39d18'
            'd0d1c8a799996bf0265b98b5d48ab919'))
        decoded = beam.SharedBuffer()
        self.assertEqual(decoder.decode(encoded, decoded), 16)
        self.assertEqual(decoded.get_data(), bytes(16))

    def test_authentication_failure(self):
        key = beam.SharedBuffer(bytes(32))
        nonce = beam.SharedBuffer(bytes(12))
        encoder = beam.AesGcmEncoder(key, nonce)
        decoder = beam.AesGcmDecoder(key, nonce)
        encoded = beam.SharedBuffer()
        encoder.encode(beam.SharedBuffer(b'payload'), encoded)
        data = bytearray(encoded.get_data())
        data[-1] ^= 1
        decoded = beam.SharedBuffer(b'unchanged')
        with self.assertRaises(beam.DecoderException):
            decoder.decode(beam.SharedBuffer(bytes(data)), decoded)
        self.assertEqual(decoded.get_data(), b'unchanged')


class TestAesGcmChannel(unittest.TestCase):
    def test_communication_and_transport_lifetime(self):
        for framed in (False, True):
            with self.subTest(framed=framed):
                self._test_communication_and_transport_lifetime(framed)

    def _test_communication_and_transport_lifetime(self, framed):
        listener = beam.LocalServerConnection()
        server = {}
        errors = []
        adapters = []

        def frame(transport):
            if framed:
                reader = beam.Reader(
                    beam.SizeDeclarativeReader(transport.reader))
                writer = beam.Writer(
                    beam.SizeDeclarativeWriter(transport.writer))
                adapters.append((transport, reader, writer))
                return beam.BasicChannel(transport.identifier,
                    transport.connection, reader, writer)
            return transport

        def accept():
            try:
                transport = frame(beam.ServerConnection(listener).accept())
                server['reference'] = weakref.ref(transport)
                server['channel'] = beam.AesGcmChannel(transport,
                    beam.AesGcmServerConfig(PRIVATE_KEY))
            except Exception as error:
                errors.append(error)

        thread = threading.Thread(target=accept)
        thread.start()
        transport = frame(beam.LocalClientChannel('aes', listener))
        reference = weakref.ref(transport)
        config = beam.AesGcmClientConfig(PUBLIC_KEY)
        try:
            client = beam.AesGcmChannel(transport, config)
        finally:
            thread.join()
            listener.close()
        self.assertEqual(errors, [])
        del transport
        gc.collect()
        self.assertIsNotNone(reference())
        self.assertIsNotNone(server['reference']())
        peer = server['channel']
        try:
            client.writer.write(beam.SharedBuffer(b'client message'))
            received = beam.SharedBuffer()
            self.assertEqual(peer.reader.read(received), 14)
            self.assertEqual(received.get_data(), b'client message')
            peer.writer.write(beam.SharedBuffer(b'server message'))
            received = beam.SharedBuffer()
            self.assertEqual(client.reader.read(received), 14)
            self.assertEqual(received.get_data(), b'server message')
        finally:
            client.connection.close()
            peer.connection.close()
        del client, peer
        server.clear()
        gc.collect()
        self.assertIsNone(reference())

    def test_missing_public_key(self):
        with self.assertRaises(beam.ConnectException):
            beam.AesGcmChannel(beam.NullChannel(),
                beam.AesGcmClientConfig(''))


if __name__ == '__main__':
    unittest.main()
