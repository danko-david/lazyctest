#!/bin/bash
gcc -I../../WD/toe/src/c -rdynamic -g -ldl -lpcre -lpthread -lpth main.c -o lct
gcc -fPIC -shared lazyctest.c -o liblct.so
