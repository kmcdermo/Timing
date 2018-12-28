#!/bin/bash

## dir bases
export eosbase="/eos/cms/store/group/phys_exotica/displacedPhotons/nTuples/2017/analysis"
export inbase="${eosbase}/unskimmed"
export tmpbase="/tmp/${USER}"
export outbase="${eosbase}/skims/madv2_v2"

## filenames
export infiles="dispho_*.root"
export outfile="tree.root"
