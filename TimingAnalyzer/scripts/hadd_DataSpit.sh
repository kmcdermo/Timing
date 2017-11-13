#!/bin/bash

for era in B C D E F
do
    hadd -O -k 2017${era}/hltplots-2017${era}.root 2017${era}/v?/hltplots_*.root
done

rm -rf 2017?/v?/*
