#!/bin/sh

# disable filename globbing
set -f

echo Content-type: text/plain
echo
echo ok
./index.cgi | echo
