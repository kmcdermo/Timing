#!/bin/sh

file=$1
hist=$2

root -l -b -q "macros/plotdump.C("\"${file}\",\"${hist}\"")"
