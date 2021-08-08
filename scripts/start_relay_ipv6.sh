#!/bin/bash

# condalf_backend [-h Host] [-p Port] [-r Relay config] [-s Python module]

# unbuffer - force line buffering, otherwise the piped output becomes unresponsive. Part of the expect package.
# 2>&1 - redirects stderr to stdout
unbuffer ./build/src/apps/ConDaLF-Backend/condalf_backend -h :: -p 5683 -r config/relay_conf 2>&1 | unbuffer -p tee $(date "+%y-%m-%d_%H:%M_relay.log")
#./build/src/apps/ConDaLF-Backend/condalf_backend -h :: -p 5683 -r config/relay_conf
