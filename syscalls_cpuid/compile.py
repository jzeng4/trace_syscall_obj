import os
import operator
import sys
import subprocess  
from os import walk

f = []
for (dirpath, dirnames, filenames) in walk('.'):
	f.extend(filenames)
	break

for file in enumerate(f):
	print file[1]
	if file[1][0] == 'T':
		target = 'bin/' + file[1][ 0 : len(file[1])-2]
		cmd = ["gcc", file[1], "-o", target]
		p = subprocess.Popen(cmd)
		p.wait()

