#!/bin/bash

cutflowconfig=${1:-"cut_config/one_at_a_time/control_qcd.txt"}
filename=${2:-"skims/std_qcd.root"}
grouplabel=${3:-"QCD_Flow"}

## produce entrylists
root -l -b -q runEntryListMaker.C\(\"${cutflowconfig}\",\"${filename}\",\"${grouplabel}\"\)
