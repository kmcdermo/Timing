#!/bin/bash

source exportDataVars.sh

## Hadd on tmp 
hadd -O -k ${tmpdir}/${outfile} ${tmpdir}/2017?/v?/${infiles}
rm -rf ${tmpdir}/2017?/v?/${infiles}
rm -rf ${tmpdir}/2017?/v?/failed/

## Copy back to EOS
eos mkdir -p ${outdir}
xrdcp -r ${tmpdir}/${outfile} ${rootbase}/${outdir}/
rm -rf ${tmpdir}/${outfile}
