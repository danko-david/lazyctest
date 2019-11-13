#!/bin/bash
cd "$(dirname `readlink -f "$0"`)"
set -e
rm /usr/include/lazyctest.h
rm /usr/lib/liblct.so
rm /usr/bin/lct

