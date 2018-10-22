#!/bin/bash

## dir bases
export eosbase="/eos/cms/store/user/${USER}/nTuples"
export inbase="${eosbase}/unskimmed"
export tmpbase="/tmp/${USER}"
export outbase="${eosbase}/skims/2017/madv2_test_v1"

## filenames
export infiles="dispho_*.root"
export outfile="tree.root"
