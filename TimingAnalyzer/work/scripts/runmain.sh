#!/bin/sh

for fit in gaus1 gaus2
do
    ./main --apply-TOF --do-standard --do-timeres --use-DEG --use-DYll --do-stacks --outdir ${fit} --fit-form ${fit}
done

for fit in gaus1 gaus2
do
    ./main --apply-TOF --use-sigman --do-standard --do-timeres --use-DEG --use-DYll --do-stacks --outdir ${fit}_sigman --fit-form ${fit}
done
