#!/bin/bash

ptrange=$1

./eoscp.sh ${ptrange}
./remove_files.sh ${ptrange}
./hadd_files.sh ${ptrange}
