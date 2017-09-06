#!/bin/sh

isMC=$1
fitdir=$2
name=$3
bins=$4
tbin=$5
tg1=$6
tg2=$7

root -l "macros/refitcanv.C("\"${isMC}\",\"${fitdir}\",\"${name}\",\"${bins}\",\"${tbin}\",\"${tg1}\",\"${tg2}\"")"

