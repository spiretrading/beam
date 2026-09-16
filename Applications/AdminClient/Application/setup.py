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
  parser.add_argument('-a', '--address', type=str, help='Spire address.',
    default=setup_utils.get_ip() + ':20000')
  parser.add_argument('-u', '--username', type=str, help='Username.',
    default='root')
  parser.add_argument('-p', '--password', type=str, help='Password.',
    default='')
  args = parser.parse_args()
  variables = {}
  variables['username'] = args.username
  variables['service_locator_address'] = args.address
  variables['admin_password'] = args.password
  with open(directory / 'config.default.yml', encoding='utf-8') as file:
    source = Template(file.read()).substitute(
      {key: json.dumps(value, ensure_ascii=False)
        for key, value in variables.items()})
  with open('config.yml', 'w', encoding='utf-8') as file:
    file.write(source)


if __name__ == '__main__':
  main()
