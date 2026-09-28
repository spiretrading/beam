import signal
import unittest
from unittest.mock import Mock, patch
import beam


class TestConnect(unittest.TestCase):
    def test_factory_arguments(self):
        value = object()
        factory = Mock(return_value=value)
        with patch('beam.sleep_for') as sleep:
            self.assertIs(beam.connect(factory, value, name='client'), value)
            factory.assert_called_once_with(value, name='client')
            sleep.assert_not_called()

    def test_retry_backoff(self):
        value = object()
        factory = Mock(side_effect=[beam.ConnectException('offline')] * 32 +
            [value])
        with patch('beam.sleep_for') as sleep:
            self.assertIs(beam.connect(factory, value, name='client'), value)
            self.assertEqual([call.args[0].total_seconds()
                for call in sleep.call_args_list],
                list(range(1, 31)) + [30, 30])
        self.assertEqual(factory.call_count, 33)
        for call in factory.call_args_list:
            self.assertEqual(call.args, (value,))
            self.assertEqual(call.kwargs, {'name': 'client'})

    def test_errors(self):
        for error in (beam.AuthenticationException('invalid credentials'),
                ValueError('invalid configuration'), KeyboardInterrupt()):
            with self.subTest(error=type(error)):
                factory = Mock(side_effect=error)
                with patch('beam.sleep_for') as sleep:
                    with self.assertRaises(type(error)) as result:
                        beam.connect(factory)
                    self.assertIs(result.exception, error)
                    factory.assert_called_once_with()
                    sleep.assert_not_called()

    def test_rejected_login(self):
        environment = beam.tests.ServiceLocatorTestEnvironment()
        self.addCleanup(environment.close)
        for arguments, message in (
                (('root', 'invalid'), 'Invalid username or password.'),
                (('invalid', ''), 'Invalid username or password.'),
                (('invalid', 123), 'Session not found.')):
            with self.subTest(arguments=arguments):
                with patch('beam.sleep_for',
                        side_effect=AssertionError('Retry')) as sleep:
                    with self.assertRaises(
                            beam.AuthenticationException) as error:
                        beam.connect(environment.make_client, *arguments)
                    self.assertEqual(str(error.exception), message)
                    sleep.assert_not_called()

    def test_interrupt_wait(self):
        handler = signal.getsignal(signal.SIGINT)
        self.addCleanup(signal.signal, signal.SIGINT, handler)
        signal.signal(signal.SIGINT, signal.default_int_handler)
        factory = Mock(side_effect=beam.ConnectException('offline'))
        with patch('beam.sleep_for',
                side_effect=lambda delay: signal.raise_signal(signal.SIGINT)):
            with self.assertRaises(KeyboardInterrupt):
                beam.connect(factory)
        factory.assert_called_once_with()
        self.assertIs(signal.getsignal(signal.SIGINT),
            signal.default_int_handler)


if __name__ == '__main__':
    unittest.main()
