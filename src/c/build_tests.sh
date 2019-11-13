#!/bin/bash
cd "$(dirname `readlink -f "$0"`)"
g++ -g -rdynamic -shared -fPIC tests.cpp -o tests.so
