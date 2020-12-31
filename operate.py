import argparse
import subprocess
import random
import os
import shutil
import json
parser = argparse.ArgumentParser()
parser.add_argument('--read', action="store_true")
parser.add_argument('--delete', action="store_true")
parser.add_argument('--update', action="store_true")
parser.add_argument('--insert', action="store_true")
parser.add_argument('--dbms', type=str)
parser.add_argument('--number', type=int)
parser.add_argument('--name', type=str)
parser.add_argument('--autoreorganization', type=float)
parser.add_argument('--page-utilization', type=float)


args = parser.parse_args()
config = {
    'autoreorganization': args.autoreorganization if args.autoreorganization else 0.5,
    'page_utilization': args.page_utilization if args.page_utilization else 0.5
}
with open('database.config', 'w') as json_config:
    json.dump(config, json_config)

dbms_dir = os.path.join(os.getcwd(),  args.name)
if os.path.exists(dbms_dir):
    shutil.rmtree(dbms_dir)
keys = random.sample(range(0, 9999999), args.number)
commands = ['insert {} {}'.format(key, random.randint(0, 200)) for key in keys]
with open('inserts.txt', 'w') as file:
    commands = map(lambda x: x+'\n', commands)
    file.writelines(commands)
dbms_call = [args.dbms, '--prefix', args.name, '--commands', 'inserts.txt']
subprocess.run(dbms_call)


if args.read:
    random.shuffle(keys)
    commands = ['read {}'.format(key) for key in keys]
    with open('reads.txt', 'w') as file:
        commands = map(lambda x: x+'\n', commands)
        file.writelines(commands)
    dbms_call = [args.dbms, '--prefix', args.name, '--commands', 'reads.txt']
    subprocess.run(dbms_call)

if args.update:
    random.shuffle(keys)
    commands = ['update {} {}'.format(
        key, random.randint(0, 200)) for key in keys]
    with open('updates.txt', 'w') as file:
        commands = map(lambda x: x+'\n', commands)
        file.writelines(commands)
    dbms_call = [args.dbms, '--prefix', args.name, '--commands', 'updates.txt']
    subprocess.run(dbms_call)

if args.delete:
    random.shuffle(keys)
    commands = ['delete {}'.format(key) for key in keys]
    with open('deletes.txt', 'w') as file:
        commands = map(lambda x: x+'\n', commands)
        file.writelines(commands)
    dbms_call = [args.dbms, '--prefix', args.name, '--commands', 'deletes.txt']
    subprocess.run(dbms_call)
