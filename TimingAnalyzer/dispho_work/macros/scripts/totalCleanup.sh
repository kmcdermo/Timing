#!/bin/bash

## source first
source scripts/common_variables.sh

dir=$(pwd)

echo "Clean up in ${dir}"
./scripts/cleanup.sh

pushd limits
dir=$(pwd)

echo "Clean up in ${dir}"
./scripts/cleanup.sh
popd 
