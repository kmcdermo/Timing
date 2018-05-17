#!/bin/bash

indir=${1:-"output"}
infilename=${2:-"higgsCombineasym_"}
void runLimits1D(const TString & indir, const TString & infilename, const TString & outtext)

root -l -b -q runLimits.C\(\"${indir}\",\"${infilename}\",\"${outtext}\")