#!/bin/bash

## dir bases
export eosbase="/eos/cms/store/user/${USER}/nTuples"
export inbase="${eosbase}/unskimmed"
export tmpbase="/tmp/${USER}"
export outbase="${eosbase}/skims/2017/Zee_mz60to150"

## filenames
export infiles="dispho_*.root"
export outfile="tree.root"
