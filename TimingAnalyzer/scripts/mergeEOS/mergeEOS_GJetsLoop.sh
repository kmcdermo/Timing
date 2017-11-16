#!/bin/bash

## input
label=${1}

for bin in 40To100 100To200 200To400 400To600 600ToInf
do
    echo "Processing:" ${bin}
    ./mergeEOS_GJetsBin.sh ${label} ${bin}
done
