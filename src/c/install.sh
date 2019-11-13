#!/bin/bash
cd "$(dirname `readlink -f "$0"`)"
set -e
cp lazyctest.h /usr/include/
cp liblct.so /usr/lib/
cp lct /usr/bin

