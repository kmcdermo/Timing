#!/bin/bash

## source first
source scripts/common_variables.sh

## clean main working space
echo "Cleaning main working space"
./scripts/cleanup.sh

## clean limits dir
pushd "${limitdir}"
echo "Cleaning ${limitdir} dir"
./scripts/cleanup.sh
popd 

echo "Finished total cleanup"
