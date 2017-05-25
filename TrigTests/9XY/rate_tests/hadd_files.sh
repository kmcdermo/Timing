#!/bin/bash

ptrange=$1

hadd -k pt${ptrange}/hltbits.root pt${ptrange}/hltbits_*.root; rm pt${ptrange}/hltbits_*.root
