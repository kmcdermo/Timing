#!/bin/bash

## input
label=${1}

for bin in 15to20 20to30 30to50 50to80 80to120 120to170 170to300 300toInf
do
    echo "Processing:" ${bin}
    ./mergeEOS_QCDHTBin.sh ${label} ${bin}
done
