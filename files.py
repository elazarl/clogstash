#!/usr/bin/python
import platform
import os
import sys
targets = ['linux', 'darwin']
target = platform.system().lower()
irrelevant_targets = []
for t in targets:
    if t != target:
        irrelevant_targets.append(t)
addtests = False
if len(sys.argv) > 1 and sys.argv[1] == 'tests':
    addtests = True

cfiles = []
for f in os.listdir('.'):
    if not f.endswith('.c'):
        continue
    for t in targets:
        if t.endswith('_' + t + '.c'):
            continue
    if f == 'main.c' and addtests:
        continue
    if f == 'tap.c' and not addtests:
        continue
    if f.endswith('_test.c') and not addtests:
        continue
    cfiles.append(f)

print '\n'.join(cfiles)
