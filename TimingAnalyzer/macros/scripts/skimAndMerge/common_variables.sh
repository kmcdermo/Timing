#!/bin/bash

## dir bases
export eosprefix="root://eoscms.cern.ch"
export eosbase="/eos/cms/store/group/phys_exotica/displacedPhotons/nTuples/2017/analysis"
export inbase="${eosbase}/unskimmed"
export tmpbase="/tmp/${USER}"
export outbase="${eosbase}/skims/madv2_v3"

## filenames
export infiles="dispho_*.root"
export outfile="tree.root"
