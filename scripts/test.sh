#!/bin/bash

# First Arg should be path to the testfile

./build/_deps/libcoap-build/coap-client -b 16 -m get coap://127.0.0.1/condalf/test
./build/_deps/libcoap-build/coap-client -b 16 -f $1 -m put coap://127.0.0.1/condalf/data
