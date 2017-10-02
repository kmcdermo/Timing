#!/bin/bash

dir=${1:-output}

echo "Moving plots, root files, and text files to /afs/cern.ch/user/k/kmcdermo/www/"
mv ${dir} /afs/cern.ch/user/k/kmcdermo/www/

echo "Executing ./makereadable.sh ${dir}"
pushd /afs/cern.ch/user/k/kmcdermo/www/
./makereadable.sh ${dir}
popd

echo "Removing local files"
rm -rf ${dir}
