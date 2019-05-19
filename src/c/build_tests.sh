#!/bin/bash
#g++ -g -rdynamic -shared -fPIC utils.c lazyctest.c tests.cpp -o tests.so
g++ -g -rdynamic -shared -fPIC lazyctest.c tests.cpp -o tests.so
