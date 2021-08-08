# ConDaLF-Backend

# Requirements

- libgnutls28-dev (min version 3.6.13)
- python3-dev (python 3.7)
- pip install cbor2
- pip install influxdb
- python 3.7

# How to build

- Clone repository
- cd into repository directory (cd condalf-backend)
- run ./scripts/build.sh

# How to run

There are multiple start scripts that start the backend with different command line arguments.
The start_relay.sh will run the ConDaLF Backend in relay-mode with the default configuration file being ./conf/relay_conf.
The start_relay_ipv6.sh will do the same but the CoAP Server will now listen to IPv6 instead of IPv4.
The start_backend.sh will run the ConDaLF Backend with data processing in python being enabled. Thus data can be inserted into influxdb this way.

# To-Do

- DTLS Support
