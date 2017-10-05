#!/bin/bash

dir=${1:-output}

host=kmcdermo@lxplus.cern.ch
whost=${host}":/afs/cern.ch/user/k/kmcdermo/www/"
echo "Moving plots, root files, and text files remotely to ${whost}"
scp -r ${dir} ${whost}

echo "Executing remotely ./makereadable.sh ${dir}"
ssh ${host} bash -c "'
cd www
./makereadable.sh ${dir}
exit
'"

echo "Removing local files"
rm -rf ${dir}
