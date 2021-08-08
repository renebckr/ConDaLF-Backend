#!/bin/bash

# condalf_backend [-h Host] [-p Port] [-r Relay config] [-s Python module]

./build/src/apps/ConDaLF-Backend/condalf_backend -h 0.0.0.0 -p 5683 -r config/relay_conf 2>&1 | unbuffer -p tee $(date "+%y-%m-%d_%H:%M_relay.log")