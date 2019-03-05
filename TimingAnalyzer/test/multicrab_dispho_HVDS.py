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
            ['/HVDS_MZp-300_MDP-20_Ctau-1mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.409e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-20_Ctau-100mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.432e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-20_Ctau-500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.423e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-20_Ctau-1000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.434e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-20_Ctau-2500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.401e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-20_Ctau-5000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.418e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-20_Ctau-10000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.424e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-40_Ctau-1mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.409e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-40_Ctau-100mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.421e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-40_Ctau-500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.417e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-40_Ctau-1000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.421e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-40_Ctau-2500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.427e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-40_Ctau-5000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.410e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-40_Ctau-10000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.432e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-60_Ctau-1mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.414e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-60_Ctau-100mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.419e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-60_Ctau-500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.423e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-60_Ctau-1000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.411e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-60_Ctau-2500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.424e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-60_Ctau-5000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.419e-01', '1', '1'],
            ['/HVDS_MZp-300_MDP-60_Ctau-10000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.419e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-20_Ctau-1mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.426e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-20_Ctau-100mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.415e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-20_Ctau-500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.429e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-20_Ctau-1000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.430e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-20_Ctau-2500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.422e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-20_Ctau-5000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.421e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-20_Ctau-10000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.424e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-40_Ctau-1mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.425e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-40_Ctau-100mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.416e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-40_Ctau-500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.406e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-40_Ctau-1000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.411e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-40_Ctau-2500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.418e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-40_Ctau-5000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.418e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-40_Ctau-10000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.407e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-60_Ctau-1mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.422e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-60_Ctau-100mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.419e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-60_Ctau-500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.490e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-60_Ctau-1000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.416e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-60_Ctau-2500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.415e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-60_Ctau-5000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.432e-01', '1', '1'],
            ['/HVDS_MZp-500_MDP-60_Ctau-10000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.445e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-20_Ctau-1mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.414e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-20_Ctau-100mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.407e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-20_Ctau-500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.421e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-20_Ctau-1000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.422e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-20_Ctau-2500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.422e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-20_Ctau-5000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.428e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-20_Ctau-10000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.415e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-40_Ctau-1mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.419e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-40_Ctau-100mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.419e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-40_Ctau-500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.426e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-40_Ctau-1000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.424e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-40_Ctau-2500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.420e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-40_Ctau-5000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.413e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-40_Ctau-10000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.419e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-60_Ctau-1mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.415e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-60_Ctau-100mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.414e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-60_Ctau-500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.409e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-60_Ctau-1000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.408e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-60_Ctau-2500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.408e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-60_Ctau-5000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.424e-01', '1', '1'],
            ['/HVDS_MZp-800_MDP-60_Ctau-10000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.425e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-20_Ctau-1mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.421e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-20_Ctau-100mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.412e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-20_Ctau-500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.420e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-20_Ctau-1000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.404e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-20_Ctau-2500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.394e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-20_Ctau-5000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.420e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-20_Ctau-10000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.429e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-40_Ctau-1mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.421e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-40_Ctau-100mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.425e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-40_Ctau-500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.412e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-40_Ctau-1000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.421e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-40_Ctau-2500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.410e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-40_Ctau-5000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.407e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-40_Ctau-10000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.414e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-60_Ctau-1mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.428e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-60_Ctau-100mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.415e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-60_Ctau-500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.410e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-60_Ctau-1000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.405e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-60_Ctau-2500mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.418e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-60_Ctau-5000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.426e-01', '1', '1'],
            ['/HVDS_MZp-1000_MDP-60_Ctau-10000mm_TuneCP5_13TeV-pythia8/RunIIFall17MiniAODv2-PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/MINIAODSIM', '3.412e-01', '1', '1'],

            ]
 
        for inDO in inputDataAndOpts:
            # inDO[0] is of the form /A/B/C. Since A is unique for each inDO for Monte Carlo, use this in the CRAB request name.
            config.General.requestName   = inDO[0].split('/')[1]
            config.JobType.pyCfgParams   = ['globalTag=94X_mc2017_realistic_v17','nThreads='+str(config.JobType.numCores),
                                            'xsec='+inDO[1],'filterEff='+inDO[2],'BR='+inDO[3],'isHVDS=True',
                                            'inputPaths='+inputPaths,'inputFilters='+inputFilters,'inputFlags='+inputFlags]
            config.Data.inputDataset     = inDO[0]
            config.Data.outputDatasetTag = '%s_%s' % (config.General.workArea, config.General.requestName)
            # Submit.
            try:
                print "Submitting for input dataset %s" % (inDO[0])
                crabCommand(options.crabCmd, config = config, *options.crabCmdOpts.split())
                os.system("rm -rf %s/crab_%s/inputs" % (config.General.workArea, config.General.requestName))
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
