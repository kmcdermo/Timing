#!/bin/bash

dir=$(pwd)

echo "Clean up in ${dir}"
./scripts/cleanup.sh

pushd limits >/dev/null
dir=$(pwd)

echo "Clean up in ${dir}"
./scripts/cleanup.sh
popd >/dev/null