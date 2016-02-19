# Export the following to shornten the command prompt
#
# export PROMPT_COMMAND='PS1="$(python ~/.short.pwd.py)"'

import os
from commands import getoutput
from socket import gethostname
hostname = gethostname()
username = os.environ['USER']
pwd = os.getcwd()
homedir = os.path.expanduser('~')
pwd = pwd.replace(homedir, '~', 1)
if len(pwd) > 30:
    pwd = pwd[:10]+'...'+pwd[-20:] # first 10 chars+last 20 chars
    print '[%s@%s:%s]\n>' % (username, hostname, pwd)
