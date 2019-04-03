#!/bin/bash

## dir bases
export eosprefix="root://eoscms.cern.ch"
export eosbase="/eos/cms/store/group/phys_exotica/displacedPhotons/nTuples/2017/analysis"
export inbase="${eosbase}/unskimmed"
export tmpbase="/tmp/${USER}"
export outbase="${eosbase}/skims/madv2_v4"

## filenames
export infiles="dispho_*.root"
export outfile="tree.root"

## which skim to make
export inskimconfig="skim_config/standard_nominal.txt"
