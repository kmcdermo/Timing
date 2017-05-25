#!/bin/bash

ptrange=$1

xrdcp -r root://eoscms//store/group/phys_exotica/displacedPhotons/QCD_Pt-${ptrange}_EMEnriched/ pt${ptrange}/
