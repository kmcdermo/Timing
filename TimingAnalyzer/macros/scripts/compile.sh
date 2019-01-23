#!/bin/bash

# source first
source scripts/common_variables.sh

# compile main code
root -l -b -q compile.C

# compile limit code
pushd "${limitdir}"
./scripts/compile.sh
popd
