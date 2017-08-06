#! /bin/sh

#input == lxplus batch directory, STDOUT is the standard output from cmsRun blah.py
#output == output directory for grepped and stripped files

input=$1
output=$2

mkdir -p $output

grep "particleFlowOOTRecHitECAL PFRecHitProducer" ${input} | cut -d ' ' -f 6 > ${output}/recHitECAL.txt
grep "particleFlowClusterOOTECALUncorrected PFClusterProducer" ${input} | cut -d ' ' -f 6 > ${output}/uncorrectedclusterECAL.txt
grep "particleFlowClusterOOTECAL CorrectedECALPFClusterProducer" ${input} | cut -d ' ' -f 6 > ${output}/correctedclusterECAL.txt
grep "particleFlowSuperClusterOOTECAL PFECALSuperClusterProducer" ${input} | cut -d ' ' -f 6 > ${output}/superclusterECAL.txt
grep "mustacheOOTPhotonCore PhotonCoreProducer"  ${input} | cut -d ' ' -f 6 > ${output}/photoncore.txt
grep "mustacheOOTPhotons PhotonProducer"  ${input} | cut -d ' ' -f 6 > ${output}/photon.txt

grep "TimeEvent>" ${input} | cut -d ' ' -f 4 > ${output}/event.txt

paste -d' ' ${output}/recHitECAL.txt ${output}/uncorrectedclusterECAL.txt ${output}/correctedclusterECAL.txt ${output}/superclusterECAL.txt ${output}/photoncore.txt ${output}/photon.txt ${output}/event.txt > ${output}/total.txt