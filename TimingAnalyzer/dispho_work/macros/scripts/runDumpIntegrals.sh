#!/bin/bash

outfiletext=${1:-"phopt_0"}

root -l -b -q runDumpIntegrals.C\(\"${outfiletext}\"\)
