#!/bin/bash
g++ -g -rdynamic -shared -fPIC tests.cpp -o tests.so
