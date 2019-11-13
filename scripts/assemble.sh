#!/bin/bash

cd "$(dirname `readlink -f "$0"`)"

mkdir ../WD/bin

set -e

./get_c_deps.sh
../src/c/build.sh
cp ../src/c/lct ../WD/bin
