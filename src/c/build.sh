#!/bin/bash
gcc -rdynamic -g -ldl -lpcre -lpthread -lpth main.c -o lct
