import argparse
import importlib.util
import json
from pathlib import Path
from string import Template

directory = Path(__file__).resolve().parent
helper_path = directory / 'setup_utils.py'
if not helper_path.is_file():
  helper_path = directory / '..' / '..' / 'Python' / 'setup_utils.py'
  if not helper_path.is_file():
    helper_path = directory / '..' / 'Python' / 'setup_utils.py'
spec = importlib.util.spec_from_file_location('setup_utils', helper_path)
setup_utils = importlib.util.module_from_spec(spec)
spec.loader.exec_module(setup_utils)


def main():
  parser = argparse.ArgumentParser(
    description='v1.0 Copyright (C) 2020 Spire Trading Inc.')
  parser.add_argument('-l', '--local', type=str, help='Local interface.',
    default=setup_utils.get_ip())
  parser.add_argument('-w', '--world', type=str, help='Global interface.',
    required=False)
  parser.add_argument('-a', '--address', type=str, help='Spire address.',
    required=False)
  parser.add_argument('-ma', '--mysql_address', type=str, help='MySQL address.',
    default='127.0.0.1:3306')
  parser.add_argument('-mu', '--mysql_username', type=str,
    help='MySQL username.', default='spireadmin')
  parser.add_argument('-mp', '--mysql_password', type=str,
    help='MySQL password.', required=True)
  parser.add_argument('-ms', '--mysql_schema', type=str, help='MySQL schema.',
    default='spire')
  args = parser.parse_args()
  variables = {}
  variables['local_interface'] = args.local
  variables['global_interface'] = \
    variables['local_interface'] if args.world is None else args.world
  variables['service_locator_address'] = \
    ('%s:20000' % variables['local_interface']) if args.address is None else \
    args.address
  variables['mysql_address'] = args.mysql_address
  variables['mysql_username'] = args.mysql_username
  variables['mysql_password'] = args.mysql_password
  variables['mysql_schema'] = args.mysql_schema
  for key in ['mysql_address', 'mysql_username', 'mysql_password',
      'mysql_schema']:
    variables[key] = json.dumps(variables[key], ensure_ascii=False)
  with open(directory / 'config.default.yml', encoding='utf-8') as file:
    source = Template(file.read()).substitute(variables)
  with open('config.yml', 'w', encoding='utf-8') as file:
    file.write(source)


if __name__ == '__main__':
  main()
