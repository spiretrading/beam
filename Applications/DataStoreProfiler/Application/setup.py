import argparse
import importlib.util
import os
import shutil

try:
  spec = importlib.util.spec_from_file_location('setup_utils',
    os.path.join('..', '..', 'Python', 'setup_utils.py'))
  setup_utils = importlib.util.module_from_spec(spec)
  spec.loader.exec_module(setup_utils)
except FileNotFoundError:
  spec = importlib.util.spec_from_file_location('setup_utils',
    os.path.join('..', 'Python', 'setup_utils.py'))
  setup_utils = importlib.util.module_from_spec(spec)
  spec.loader.exec_module(setup_utils)


def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-ma', '--mysql_address', type=str, help='MySQL address.',
    default='127.0.0.1:3306')
  parser.add_argument('-mu', '--mysql_username', type=str,
    help='MySQL username.', default='spireadmin')
  parser.add_argument('-mp', '--mysql_password', type=str,
    help='MySQL password.', required=True)
  parser.add_argument('-ms', '--mysql_schema', type=str, help='MySQL schema.',
    default='data_store_profiler')
  args = parser.parse_args()
  variables = {}
  variables['mysql_address'] = args.mysql_address
  variables['mysql_username'] = args.mysql_username
  variables['mysql_password'] = args.mysql_password
  variables['mysql_schema'] = args.mysql_schema
  shutil.copy('config.default.yml', 'config.yml')
  with open('config.yml', 'r+') as file:
    source = setup_utils.translate(file.read(), variables)
    file.seek(0)
    file.write(source)
    file.truncate()


if __name__ == '__main__':
  main()
