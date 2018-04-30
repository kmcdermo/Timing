#!/bin/bash

datacards={$1:-"datacards_*.txt"}
workspace={$2:-"workspace.root"}

cp ${datacards} ${workspace} /afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_8_1_0/src/HiggsAnalysis/CombinedLimit/working
