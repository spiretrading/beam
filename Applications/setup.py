import argparse
import importlib.util
import os

try:
  spec = importlib.util.spec_from_file_location('setup_utils',
    os.path.join('..', 'Python', 'setup_utils.py'))
  setup_utils = importlib.util.module_from_spec(spec)
  spec.loader.exec_module(setup_utils)
except FileNotFoundError:
  spec = importlib.util.spec_from_file_location('setup_utils',
    os.path.join('Python', 'setup_utils.py'))
  setup_utils = importlib.util.module_from_spec(spec)
  spec.loader.exec_module(setup_utils)


def make_sub_args(arg_vars, *args):
  sub_args = []
  for arg in args:
    if arg_vars[arg]:
      sub_args += ['--' + arg, arg_vars[arg]]
  return sub_args


def setup_application(application, arg_vars, *args):
  root_path = os.getcwd()
  try:
    os.chdir(os.path.join(application, 'Application'))
    setup_utils.run_subscript('setup.py', make_sub_args(arg_vars, *args))
  finally:
    os.chdir(root_path)


def setup_application_with_mysql(server, arg_vars):
  if arg_vars['mysql_password'] is None:
    arg_vars = arg_vars.copy()
    arg_vars['mysql_password'] = arg_vars['password']
  setup_application(server, arg_vars, 'mysql_address', 'mysql_username',
    'mysql_password', 'mysql_schema')


def setup_server(server, arg_vars):
  setup_application(server, arg_vars, 'local', 'world', 'address', 'password')


def setup_server_with_mysql(server, arg_vars):
  setup_application(server, arg_vars, 'local', 'world', 'address', 'password',
    'mysql_address', 'mysql_username', 'mysql_password', 'mysql_schema')


def setup_service_locator(arg_vars):
  if arg_vars['mysql_password'] is None:
    arg_vars = arg_vars.copy()
    arg_vars['mysql_password'] = arg_vars['password']
  setup_application('ServiceLocator', arg_vars, 'local', 'world',
    'mysql_address', 'mysql_username', 'mysql_password', 'mysql_schema')


def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    required=False)
  parser.add_argument('-w', '--world', type=str, help='Global interface.',
    required=False)
  parser.add_argument('-a', '--address', type=str, help='Spire address.',
    required=False)
  parser.add_argument('-p', '--password', type=str, help='Password.',
    required=False)
  parser.add_argument('-ma', '--mysql_address', type=str, help='MySQL address.',
    required=False)
  parser.add_argument('-mu', '--mysql_username', type=str,
    help='MySQL username.', required=False)
  parser.add_argument('-mp', '--mysql_password', type=str,
    help='MySQL password.', required=False)
  parser.add_argument('-ms', '--mysql_schema', type=str, help='MySQL schema.',
    required=False)
  arg_vars = vars(parser.parse_args())
  admin_client_arg_vars = arg_vars.copy()
  admin_client_arg_vars['username'] = 'root'
  admin_client_arg_vars['password'] = '""'
  setup_application('AdminClient', admin_client_arg_vars, 'address', 'username',
    'password')
  setup_application('ClientTemplate', arg_vars, 'local')
  setup_application_with_mysql('DataStoreProfiler', arg_vars)
  setup_application('HttpFileServer', arg_vars, 'local', 'world')
  setup_service_locator(arg_vars)
  setup_application('ServiceProtocolProfiler', arg_vars, 'local')
  setup_application('ServletTemplate', arg_vars, 'local')
  setup_server_with_mysql('UidServer', arg_vars)
  setup_application('WebSocketEchoServer', arg_vars, 'local', 'world')


if __name__ == '__main__':
  main()
