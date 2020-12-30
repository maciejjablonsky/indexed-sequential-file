import argparse
import subprocess
import random
import os
import shutil

parser = argparse.ArgumentParser()
parser.add_argument('--insert', action="store_true")
parser.add_argument('--read', action="store_true")
parser.add_argument('--delete', action="store_true")
parser.add_argument('--dbms', type=str)
parser.add_argument('--number', type=int)
parser.add_argument('--name', type=str)
args = parser.parse_args()

shutil.rmtree(os.path.join(os.getcwd(),  args.name))
keys = random.sample(range(0, 9999999), args.number)
if args.insert:
    commands = ['insert {} {}'.format(key, random.randint(0, 200)) for key in keys]
    with open('inserts.txt','w') as file:
        commands=map(lambda x:x+'\n', commands)
        file.writelines(commands)
    dbms_call = [args.dbms, '--prefix', args.name, '--commands', 'inserts.txt']
    subprocess.run(dbms_call)


