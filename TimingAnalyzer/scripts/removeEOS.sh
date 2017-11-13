#!/bin/bash

base=${1}

./removeEOS_Data.sh ${base}
./removeEOS_Bkgd.sh ${base}
./removeEOS_Signal.sh ${base}
