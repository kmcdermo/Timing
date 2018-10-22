#! /usr/bin/env python

import os
from optparse import OptionParser

from CRABAPI.RawCommand import crabCommand
from CRABClient.ClientExceptions import ClientException
from httplib import HTTPException

def getOptions():
    """
    Parse and return the arguments provided by the user.
    """
    usage = ("Usage: %prog --crabCmd CMD [--workArea WAD --crabCmdOpts OPTS]"
             "\nThe multicrab command executes 'crab CMD OPTS' for each project directory contained in WAD"
             "\nUse multicrab -h for help")

    parser = OptionParser(usage=usage)

    parser.add_option('-c', '--crabCmd',
                      dest = 'crabCmd',
                      default = '',
                      help = "crab command",
                      metavar = 'CMD')

    parser.add_option('-w', '--workArea',
                      dest = 'workArea',
                      default = 'multicrab_dispho_Signal',
                      help = "work area directory (only if CMD != 'submit')",
                      metavar = 'WAD')

    parser.add_option('-o', '--crabCmdOpts',
                      dest = 'crabCmdOpts',
                      default = '',
                      help = "options for crab command CMD",
                      metavar = 'OPTS')

    (options, arguments) = parser.parse_args()

    if arguments:
        parser.error("Found positional argument(s): %s." % (arguments))
    if not options.crabCmd:
        parser.error("(-c CMD, --crabCmd=CMD) option not provided.")
    if options.crabCmd != 'submit':
        if not options.workArea:
            parser.error("(-w WAR, --workArea=WAR) option not provided.")
        if not os.path.isdir(options.workArea):
            parser.error("'%s' is not a valid directory." % (options.workArea))

    return options


def main():

    options = getOptions()

    # The submit command needs special treatment.
    if options.crabCmd == 'submit':

        # External files needed by CRAB
        inputDir     = '/afs/cern.ch/user/k/kmcdermo/public/input/'
        inputPaths   = 'HLTpathsWExtras.txt'
        inputFilters = 'HLTfilters.txt'
        inputFlags   = 'METflags.txt'
         
        #--------------------------------------------------------
        # This is the base config:
        #--------------------------------------------------------
        from CRABClient.UserUtilities import config
        config = config()

        config.General.workArea    = options.workArea
        config.General.requestName = None

        config.JobType.pluginName  = 'Analysis'
        config.JobType.psetName    = 'dispho.py'
        config.JobType.numCores    = 8
        config.JobType.pyCfgParams = None
        config.JobType.inputFiles  = [ inputDir+inputPaths , inputDir+inputFilters , inputDir+inputFlags ]

        config.Data.inputDataset = None
        config.Data.splitting    = 'EventAwareLumiBased'
        config.Data.unitsPerJob  = 10000

        config.Data.allowNonValidInputDataset = True

        config.Data.outputDatasetTag = None
        config.Data.publication      = False
        config.Site.storageSite      = 'T2_CH_CERN'
        config.Data.outLFNDirBase    = '/store/group/phys_exotica/displacedPhotons/nTuples/2017/analysis/unskimmed'
        #--------------------------------------------------------

        # Will submit one task for each of these input datasets.
        inputDataAndOpts = [
            ['/GMSB_L-100TeV_Ctau-0p001cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.274e+00', '1', '9.44338500E-01'],
            ['/GMSB_L-100TeV_Ctau-0_1cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.168e+00', '1', '9.44338500E-01'],
            ['/GMSB_L-100TeV_Ctau-10cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.153e+00', '1', '9.44338500E-01'],
            ['/GMSB_L-100TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.174e+00', '1', '9.44338500E-01'],
            ['/GMSB_L-100TeV_Ctau-400cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.167e+00', '1', '9.44338441E-01'],
            ['/GMSB_L-100TeV_Ctau-600cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.189e+00', '1', '9.44338560E-01'],
            ['/GMSB_L-100TeV_Ctau-800cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.172e+00', '1', '9.44338500E-01'],
            ['/GMSB_L-100TeV_Ctau-1000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.163e+00', '1', '9.44338441E-01'],
            ['/GMSB_L-100TeV_Ctau-1200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.150e+00', '1', '9.44338381E-01'],
            ['/GMSB_L-100TeV_Ctau-10000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.268e+00', '1', '9.44338441E-01'],
            ['/GMSB_L-150TeV_Ctau-0p001cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.349e-01', '1', '8.57347429E-01'],
            ['/GMSB_L-150TeV_Ctau-0_1cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.274e-01', '1', '8.57347369E-01'],
            ['/GMSB_L-150TeV_Ctau-10cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.285e-01', '1', '8.57347429E-01'],
            ['/GMSB_L-150TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.283e-01', '1', '8.57347310E-01'],
            ['/GMSB_L-150TeV_Ctau-400cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.281e-01', '1', '8.57347369E-01'],
            ['/GMSB_L-150TeV_Ctau-600cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.257e-01', '1', '8.57347429E-01'],
            ['/GMSB_L-150TeV_Ctau-800cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.281e-01', '1', '8.57347310E-01'],
            ['/GMSB_L-150TeV_Ctau-1000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.287e-01', '1', '8.57347369E-01'],
            ['/GMSB_L-150TeV_Ctau-1200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.282e-01', '1', '8.57347369E-01'],
            ['/GMSB_L-150TeV_Ctau-10000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '2.346e-01', '1', '8.57347310E-01'],
            ['/GMSB_L-200TeV_Ctau-0p001cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.517e-02', '1', '8.14181805E-01'],
            ['/GMSB_L-200TeV_Ctau-0_1cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.454e-02', '1', '8.14181864E-01'],
            ['/GMSB_L-200TeV_Ctau-10cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.449e-02', '1', '8.14181864E-01'],
            ['/GMSB_L-200TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.445e-02', '1', '8.14181864E-01'],
            ['/GMSB_L-200TeV_Ctau-400cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.450e-02', '1', '8.14181805E-01'],
            ['/GMSB_L-200TeV_Ctau-600cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.448e-02', '1', '8.14181924E-01'],
            ['/GMSB_L-200TeV_Ctau-800cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.464e-02', '1', '8.14181864E-01'],
            ['/GMSB_L-200TeV_Ctau-1000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.437e-02', '1', '8.14181864E-01'],
            ['/GMSB_L-200TeV_Ctau-1200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.426e-02', '1', '8.14181864E-01'],
            ['/GMSB_L-200TeV_Ctau-10000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.532e-02', '1', '8.14181805E-01'],
            ['/GMSB_L-250TeV_Ctau-0p001cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.282e-02', '1', '7.92751014E-01'],
            ['/GMSB_L-250TeV_Ctau-0_1cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.257e-02', '1', '7.92751014E-01'],
            ['/GMSB_L-250TeV_Ctau-10cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.261e-02', '1', '7.92751014E-01'],
            ['/GMSB_L-250TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.262e-02', '1', '7.92751014E-01'],
            ['/GMSB_L-250TeV_Ctau-400cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.260e-02', '1', '7.92751014E-01'],
            ['/GMSB_L-250TeV_Ctau-600cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.261e-02', '1', '7.92751074E-01'],
            ['/GMSB_L-250TeV_Ctau-800cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.261e-02', '1', '7.92751014E-01'],
            ['/GMSB_L-250TeV_Ctau-1000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.258e-02', '1', '7.92751074E-01'],
            ['/GMSB_L-250TeV_Ctau-1200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.258e-02', '1', '7.92751014E-01'],
            ['/GMSB_L-250TeV_Ctau-10000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.272e-02', '1', '7.92751014E-01'],
            ['/GMSB_L-300TeV_Ctau-0p001cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.495e-03', '1', '7.80865729E-01'],
            ['/GMSB_L-300TeV_Ctau-0_1cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.458e-03', '1', '7.80865729E-01'],
            ['/GMSB_L-300TeV_Ctau-10cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.452e-03', '1', '7.80865788E-01'],
            ['/GMSB_L-300TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.429e-03', '1', '7.80865729E-01'],
            ['/GMSB_L-300TeV_Ctau-400cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.449e-03', '1', '7.80865729E-01'],
            ['/GMSB_L-300TeV_Ctau-600cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.442e-03', '1', '7.80865729E-01'],
            ['/GMSB_L-300TeV_Ctau-800cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.448e-03', '1', '7.80865729E-01'],
            ['/GMSB_L-300TeV_Ctau-1000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.454e-03', '1', '7.80865729E-01'],
            ['/GMSB_L-300TeV_Ctau-1200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.466e-03', '1', '7.80865729E-01'],
            ['/GMSB_L-300TeV_Ctau-10000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.475e-03', '1', '7.80865729E-01'],
            ['/GMSB_L-350TeV_Ctau-0p001cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.799e-03', '1', '7.73663223E-01'],
            ['/GMSB_L-350TeV_Ctau-0_1cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.784e-03', '1', '7.73663223E-01'],
            ['/GMSB_L-350TeV_Ctau-10cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.787e-03', '1', '7.73663163E-01'],
            ['/GMSB_L-350TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.790e-03', '1', '7.73663223E-01'],
            ['/GMSB_L-350TeV_Ctau-400cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.786e-03', '1', '7.73663223E-01'],
            ['/GMSB_L-350TeV_Ctau-600cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.791e-03', '1', '7.73663223E-01'],
            ['/GMSB_L-350TeV_Ctau-800cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.785e-03', '1', '7.73663223E-01'],
            ['/GMSB_L-350TeV_Ctau-1000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.790e-03', '1', '7.73663223E-01'],
            ['/GMSB_L-350TeV_Ctau-1200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.784e-03', '1', '7.73663223E-01'],
            ['/GMSB_L-350TeV_Ctau-10000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.802e-03', '1', '7.73663223E-01'],
            ['/GMSB_L-400TeV_Ctau-0p001cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '7.817e-04', '1', '7.68983364E-01'],
            ['/GMSB_L-400TeV_Ctau-0_1cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '7.764e-04', '1', '7.68983364E-01'],
            ['/GMSB_L-400TeV_Ctau-10cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '7.785e-04', '1', '7.68983364E-01'],
            ['/GMSB_L-400TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '7.784e-04', '1', '7.68983364E-01'],
            ['/GMSB_L-400TeV_Ctau-400cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '7.786e-04', '1', '7.68983364E-01'],
            ['/GMSB_L-400TeV_Ctau-600cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '7.776e-04', '1', '7.68983364E-01'],
            ['/GMSB_L-400TeV_Ctau-800cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '7.758e-04', '1', '7.68983364E-01'],
            ['/GMSB_L-400TeV_Ctau-1000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '7.779e-04', '1', '7.68983364E-01'],
            ['/GMSB_L-400TeV_Ctau-1200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '7.777e-04', '1', '7.68983305E-01'],
            ['/GMSB_L-400TeV_Ctau-10000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '7.865e-04', '1', '7.68983305E-01'],
            ['/GMSB_L-500TeV_Ctau-0p001cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.703e-04', '1', '7.63475120E-01'],
            ['/GMSB_L-500TeV_Ctau-0p1cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.700e-04', '1', '7.63475120E-01'],
            ['/GMSB_L-500TeV_Ctau-10cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.694e-04', '1', '7.63475120E-01'],
            ['/GMSB_L-500TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.702e-04', '1', '7.63475120E-01'],
            ['/GMSB_L-500TeV_Ctau-400cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.700e-04', '1', '7.63475120E-01'],
            ['/GMSB_L-500TeV_Ctau-600cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.698e-04', '1', '7.63475120E-01'],
            ['/GMSB_L-500TeV_Ctau-800cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.698e-04', '1', '7.63475120E-01'],
            ['/GMSB_L-500TeV_Ctau-1000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.701e-04', '1', '7.63475120E-01'],
            ['/GMSB_L-500TeV_Ctau-1200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.698e-04', '1', '7.63475120E-01'],
            ['/GMSB_L-500TeV_Ctau-10000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '1.703e-04', '1', '7.63475120E-01'],
            ['/GMSB_L-600TeV_Ctau-0p001cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.234e-05', '1', '7.60478437E-01'],
            ['/GMSB_L-600TeV_Ctau-0p1cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.243e-05', '1', '7.60478497E-01'],
            ['/GMSB_L-600TeV_Ctau-10cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.234e-05', '1', '7.60478497E-01'],
            ['/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.238e-05', '1', '7.60478556E-01'],
            ['/GMSB_L-600TeV_Ctau-400cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.246e-05', '1', '7.60478497E-01'],
            ['/GMSB_L-600TeV_Ctau-600cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.232e-05', '1', '7.60478497E-01'],
            ['/GMSB_L-600TeV_Ctau-800cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.246e-05', '1', '7.60478497E-01'],
            ['/GMSB_L-600TeV_Ctau-1000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.233e-05', '1', '7.60478497E-01'],
            ['/GMSB_L-600TeV_Ctau-1200cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.241e-05', '1', '7.60478497E-01'],
            ['/GMSB_L-600TeV_Ctau-10000cm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '4.252e-05', '1', '7.60478556E-01'],

            ]
 
        for inDO in inputDataAndOpts:
            # inDO[0] is of the form /A/B/C. Since A is unique for each inDO for Monte Carlo, use this in the CRAB request name.
            config.General.requestName   = inDO[0].split('/')[1]
            config.JobType.pyCfgParams   = ['globalTag=94X_mc2017_realistic_v14','nThreads='+str(config.JobType.numCores),
                                            'xsec='+inDO[1],'filterEff='+inDO[2],'BR='+inDO[3],'isGMSB=True',
                                            'inputPaths='+inputPaths,'inputFilters='+inputFilters,'inputFlags='+inputFlags]
            config.Data.inputDataset     = inDO[0]
            config.Data.outputDatasetTag = '%s_%s' % (config.General.workArea, config.General.requestName)
            # Submit.
            try:
                print "Submitting for input dataset %s" % (inDO[0])
                crabCommand(options.crabCmd, config = config, *options.crabCmdOpts.split())
            except HTTPException as hte:
                print "Submission for input dataset %s failed: %s" % (inDO[0], hte.headers)
            except ClientException as cle:
                print "Submission for input dataset %s failed: %s" % (inDO[0], cle)

    # All other commands can be simply executed.
    elif options.workArea:

        for dir in os.listdir(options.workArea):
            projDir = os.path.join(options.workArea, dir)
            if not os.path.isdir(projDir):
                continue
            # Execute the crab command.
            msg = "Executing (the equivalent of): crab %s --dir %s %s" % (options.crabCmd, projDir, options.crabCmdOpts)
            print "-"*len(msg)
            print msg
            print "-"*len(msg)
            try:
                crabCommand(options.crabCmd, dir = projDir, *options.crabCmdOpts.split())
            except HTTPException as hte:
                print "Failed executing command %s for task %s: %s" % (options.crabCmd, projDir, hte.headers)
            except ClientException as cle:
                print "Failed executing command %s for task %s: %s" % (options.crabCmd, projDir, cle)


if __name__ == '__main__':
    main()
