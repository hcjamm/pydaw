#!/usr/bin/python
#Force a core dump when something isn't quite right, but the program won't
#crash on it's own.  This only works when debug.sh was used to run PyDAW


import commands

f_val = commands.getoutput('ps -ef | grep \./pydaw4 | grep -v grep | grep -v /bin/sh')

f_arr = f_val.split()

f_test = int(f_arr[1])

f_kill = "/bin/kill -s SIGSEGV " + f_arr[1]

print(f_kill)

f_result = commands.getoutput(f_kill)

print(f_result)
