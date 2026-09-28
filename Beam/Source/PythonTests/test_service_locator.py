import gc
import unittest
import weakref
import beam


class TestServiceUpdate(unittest.TestCase):
    def test_add_and_remove(self):
        properties = beam.JsonObject()
        properties['scope'] = 'TSX'
        service = beam.ServiceEntry('quotes', properties, 42,
            beam.DirectoryEntry.make_account(12, 'provider'))
        addition = beam.ServiceUpdate.add(service)
        self.assertEqual(addition.service.id, 42)
        self.assertEqual(addition.service.properties, properties)
        self.assertEqual(addition.type, beam.ServiceUpdate.Type.ADDED)
        removal = beam.ServiceUpdate.remove(service)
        self.assertEqual(removal.type, beam.ServiceUpdate.Type.REMOVED)
        self.assertEqual(str(removal),
            '((quotes 42 (ACCOUNT 12 provider) {"scope":"TSX"}) REMOVED)')
        queue = beam.ServiceUpdateQueue()
        queue.push(removal)
        self.assertEqual(queue.pop(), removal)


class TestServiceMonitoring(unittest.TestCase):
    def test_client_environment_lifetime(self):
        for overload in ('default', 'credentials', 'session'):
            with self.subTest(overload=overload):
                environment = beam.tests.ServiceLocatorTestEnvironment()
                reference = weakref.ref(environment)
                if overload == 'default':
                    client = environment.make_client()
                elif overload == 'credentials':
                    client = environment.make_client('root', '')
                else:
                    key = 123
                    session = environment.get_root().get_encrypted_session_id(
                        key)
                    client = environment.make_client(session, key)
                del environment
                gc.collect()
                try:
                    self.assertIsNotNone(reference())
                    self.assertEqual(client.locate('missing'), [])
                finally:
                    client.close()
                del client
                gc.collect()
                self.assertIsNone(reference())

    def test_registrations(self):
        for erased in (False, True):
            with self.subTest(erased=erased):
                environment = beam.tests.ServiceLocatorTestEnvironment()
                self.addCleanup(environment.close)
                provider = environment.make_client()
                self.addCleanup(provider.close)
                subscriber = environment.make_client()
                self.addCleanup(subscriber.close)
                if erased:
                    subscriber = beam.ServiceLocatorClient(subscriber)
                properties = beam.JsonObject()
                properties['scope'] = 'TSX'
                first = provider.add('quotes', properties)
                queue = beam.ServiceUpdateQueue()
                subscriber.monitor('quotes', queue)
                update = queue.pop()
                self.assertEqual(update, beam.ServiceUpdate.add(first))
                self.assertEqual(update.service.properties, properties)
                provider.add('orders', beam.JsonObject())
                second = provider.add('quotes', beam.JsonObject())
                self.assertEqual(queue.pop(), beam.ServiceUpdate.add(second))
                provider.remove(first)
                self.assertEqual(queue.pop(), beam.ServiceUpdate.remove(first))
                provider.close()
                self.assertEqual(queue.pop(), beam.ServiceUpdate.remove(second))
                subscriber.close()
                with self.assertRaises(beam.PipeBrokenException):
                    queue.pop()

    def test_account_monitor_overload(self):
        environment = beam.tests.ServiceLocatorTestEnvironment()
        self.addCleanup(environment.close)
        client = environment.make_client()
        self.addCleanup(client.close)
        queue = beam.AccountUpdateQueue()
        client.monitor(queue)
        update = queue.pop()
        self.assertEqual(update.account, beam.DirectoryEntry.ROOT_ACCOUNT)
        self.assertEqual(update.type, beam.AccountUpdate.Type.ADDED)


if __name__ == '__main__':
    unittest.main()
