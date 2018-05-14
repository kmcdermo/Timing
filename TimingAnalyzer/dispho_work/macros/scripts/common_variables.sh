#!/bin/bash

## dir bases
export eosbase="/eos/cms/store/user/${USER}"
export inbase="${eosbase}/nTuples/unskimmed"
export tmpbase="/tmp/${USER}"
export outbase="${eosbase}/nTuples/skims/2017/rereco_v3"

## filenames
export infiles="dispho_*.root"
export outfile="tree.root"

## redo photon id
export redoPhotonID=0
