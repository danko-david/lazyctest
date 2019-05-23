#!/bin/bash
gcc -I../../WD/toe/src/c_test -I../../WD/toe/src/c -g -rdynamic -shared -fPIC toe_tests.c -o toe_tests.so

