#!/bin/bash
cd "$(dirname `readlink -f "$0"`)"

gcc -I../../WD/toe/src/c -rdynamic -g -ldl -lpcre -lpthread -lpth main.c -o lct
gcc -fPIC -shared lazyctest.c -o liblct.so
