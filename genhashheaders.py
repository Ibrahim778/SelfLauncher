import re
import ast
import argparse
from hashlib import sha1
import ntpath

def getFileName(path):
    head, tail = ntpath.split(path)
    return tail or ntpath.basename(head)

parser = argparse.ArgumentParser()
parser.add_argument('xml')
parser.add_argument('output')

args = parser.parse_args()

fileName = getFileName(args.output).split('.')[0]

print("Creating " + args.output)

xmlf = open(args.xml, 'rt')
fileTxt = xmlf.read()
xmlf.close()

outh = open(args.output, 'wt')
outh.write(
"""\
#ifndef _{name}_H_
#define _{name}_H_

/* This file was generated automatically by genhashheaders.py */

""".format(name = fileName)
)
matches=re.findall(r'id="(?:(?:(?!(?<!\\)").)*)"',fileTxt)
for match in matches:
    outh.write("#define {id} 0x{hash}\n".format(id = match[3:].strip('"').replace('-', '_'), hash = sha1(match[3:].strip('"').encode()).hexdigest()[:8].upper()))

outh.write("\n#endif\n")  

outh.close()