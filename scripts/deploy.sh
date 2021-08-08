#!/bin/bash

# First Arg should be the username and Second arg the domain or IP of the server
# The third arg should be the ssh port

ssh $1@$2 -p $3 "rm -rf ~/condalf && mkdir -p ~/condalf"
scp -P $3 -rp $PWD/python $PWD/scripts $PWD/src CMakeLists.txt Doxyfile $1@$2:~/condalf
ssh $1@$2 -p $3
