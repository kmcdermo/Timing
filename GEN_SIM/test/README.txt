## To generate MC, I have a hacked way to produce 2017 MC. Follow the recipe below (currently setup for lxplus, but in principle should be the same for the LPC).

###################
## Initial Setup ##
###################
## cd to your favorite working directory ${dir}

cd ${dir}

## and copy these commands below
cmsrel CMSSW_9_3_3
cd CMSSW_9_3_3/src
cmsenv
git cms-init

git cms-addpkg HLTrigger/Configuration
git cms-addpkg Configuration/Generator
git clone https://github.com/kmcdermo/Timing.git
cp -r Timing/GEN_SIM/Configuration .
scram b -j 8

## Make sure to have a valid grid certificate and be registered with VOMS!
voms-proxy-init --voms cms --valid 168:00
source /cvmfs/cms.cern.ch/crab3/crab.sh

####################
## Step0: GEN-SIM ##
####################

## At this point decide which grid point you want to generate, and get the path to the gen fragment in Configuration/GenProduction/python/ThirteenTeV
## e.g. ${path}/${to}/${fragment} = Configuration/GenProduction/python/ThirteenTeV/GMSB/GMSB_L200TeV_Ctau400cm_Pythia8_13TeV_cff.py
## then do the following below

cd Timing/GEN_SIM/test
cmsDriver.py ${path}/${to}/${fragment} --mc --eventcontent RAWSIM --datatier GEN-SIM --conditions 93X_mc2017_realistic_v3 --beamspot Realistic25ns13TeVEarly2017Collision --step GEN,SIM --geometry DB:Extended --era Run2_2017 --fileout file:step0.root --no_exec --python_filename step0_GEN_SIM.py

## At this point test the config:

cmsRun step0_GEN_SIM.py

## if it works set up a crab config that is like examples/crab_step0.py
## check that you are producing the correct total amount of events (NJOBS * unitsPerJob)
## make sure the output site and LFN to where you have access to write to
## also make sure the base_name is something sensible!
## make sure publication = True, and the publisDBS is 'phys03'
## transferLogs = True will keep the stdout, stderr which we will need for getting the average xsec!

##################################
## Step1: Premixing+RAW+Trigger ##
##################################

## Now we use the test step0.root from the previous step as input. We are using the Global Run menu in this release

cmsDriver.py step1 --mc --eventcontent PREMIXRAW --datatier GEN-SIM-RAW --conditions 93X_mc2017_realistic_v3 --step DIGIPREMIX_S2,DATAMIX,L1,DIGI2RAW,HLT:GRun --nThreads 4 --datamix PreMix --era Run2_2017 --pileup_input dbs:/Neutrino_E-10_gun/RunIISummer17PrePremix-MCv1_92X_upgrade2017_realistic_v8-v1/GEN-SIM-DIGI-RAW --filein file:step0.root --fileout file:step1.root --no_exec

## again test the file to make sure it runs properly

cmsRun step1_DIGIPREMIX_S2_DATAMIX_L1_DIGI2RAW_HLT.py

## check out examples/crab_step1.py for an example grid submission for this step
## the trick here is get the output dataset name from crab status of the last step and use it as the input dataset for this step.
## this will be the proceedure for the following steps as well

##################
## Step 2: RECO ##
##################

## standard stuff here.  this step generates our out-of-time photon collection

cmsDriver.py step2 --mc --eventcontent AODSIM --datatier AODSIM --conditions 93X_mc2017_realistic_v3 --step RAW2DIGI,RECO,EI --nThreads 4 --era Run2_2017 --filein file:step1.root --fileout file:step2.root --no_exec

## Run it

cmsRun step2_RAW2DIGI_RECO_EI.py

## and get the appropriate grid submission script: examples/crab_step2.py

#################
## Step 3: PAT ##
#################

## get things in miniAOD format

cmsDriver.py step3 --mc --eventcontent MINIAODSIM --runUnscheduled --datatier MINIAODSIM --conditions 93X_mc2017_realistic_v3 --step PAT --nThreads 4 --era Run2_2017 --filein file:step2.root --fileout file:step3.root --no_exec

## test

cmsRun step3_PAT.py

## and send to the grid, examples/crab_step3.py

#########################################################################################################################################

## After all is said and done, you can delete from EOS the step1 and step2 output
## You could even delete the step0 output if you are crunched for space, although this is typically the most taxing step
## We need step3 output, miniAOD, to run the nTuples

## If you want to extract the BR for neutralino -> photon + gravitino for GMSB, you can use: Timing/GEN_SIM/GMSB/getBRfromSLHA.sh
## and then modify the script accordingly to point the directory of SLHA files to get the resulting BRs 
## HVDS has in the python config that vPion -> photon + photon BR = 1.0

## To get the xsec for the sample, we have to make sure we had transferLogs = True for step0!
## Then use the following script, modified accordingly to get the right directory: Timing/GEN_SIM/test/xsec/getXSecFromLogs.sh 

## To validate these samples, you can run the analyzer from Timing/TimingAnalyzer/plugins/Dispho.cc
## Use the Timing/TimingAnalyzer/test/dispho.py to run the analyzer
## set the input source to all the files from step3
## make sure to set isGMSB or isHVDS
## example:

cmsRun test/dispho.py isGMSB=True globalTag=93X_mc2017_realistic_v3

## Or, you can use one of the multicrab scripts, versus adding all the files one by one for each grid point (check multicrab_dispho_Signal.py)
## Just make sure to set the parameters correctly (GT, splitPho, isHVDS, etc) as well the input/output dirs
## if you do it this way, then copy the files locally and hadd them

xrdcp -r root://eoscms//store/user/${userName}/${path}/${to}/${files} .

## hadd them

hadd -O dispho.root step3_*.root

## Then the next thing to check is the lifetime from gen info saved in the ntuples (compute travel distance in lab frame, divide by gamma*beta
## fit it to an eponential to extract ctau

## an example code is here, although the names and accessors have slightly changed for these ntuples

https://github.com/kmcdermo/Timing/blob/9XY/TimingAnalyzer/macros/PlotPhotons.cc#L421-L421
https://github.com/kmcdermo/Timing/blob/9XY/TimingAnalyzer/macros/PlotPhotons.cc#L426-L428
https://github.com/kmcdermo/Timing/blob/9XY/TimingAnalyzer/macros/common/common.h#L21
https://github.com/kmcdermo/Timing/blob/9XY/TimingAnalyzer/macros/common/common.h#L23
https://github.com/kmcdermo/Timing/blob/9XY/TimingAnalyzer/macros/PlotPhotons.cc#L434

## contact me with any questions!
