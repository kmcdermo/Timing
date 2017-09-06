#!/bin/sh

file=$1

grep 'BAD FIT' ${file} | cut -d ' ' -f3-6 > reduced_${file}
rm ${file}
more reduced_${file}
#cut -d ' ' -f1 reduced_${file}