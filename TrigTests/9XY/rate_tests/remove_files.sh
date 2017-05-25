#!/bin/bash

ptrange=$1

ls -lth pt${ptrange} | grep "zh   63" | cut -d' ' -f 11 > input_files.txt

while read line
do
    rm pt${ptrange}/$line
done < input_files.txt

rm input_files.txt
