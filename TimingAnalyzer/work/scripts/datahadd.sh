#!/bin/sh

for era in D E F #B C D E F G
do
    dir="2016"${era}
    hadd DATA/${dir}/tree.root DATA/${dir}/tree_*.root
    rm DATA/${dir}/tree_*.root
done

#hadd DATA/tree.root DATA/2016?/tree.root

#rm DATA/2016?/tree.root

