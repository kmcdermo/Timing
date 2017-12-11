#!/bin/bash

## tunes
export bkgdtune="TuneCUETP8M1_13TeV-madgraphMLM-pythia8"

## dir bases
export rootbase="root://eoscms"
export eosbase="/store/user/${USER}"
export tmpbase="/tmp/${USER}"
export outbase="${eosbase}/nTuples/skims/2017"

## filenames
export infiles="dispho_*.root"
export outfile="tree.root"
