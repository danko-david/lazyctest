#!/bin/bash
gcc -rdynamic -g -ldl -lpcre -lpthread -lpth main.c -o lct
gcc -fPIC -shared lazyctest.c -o liblct.so
