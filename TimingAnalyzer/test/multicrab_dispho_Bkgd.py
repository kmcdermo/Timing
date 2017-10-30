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
                      default = 'multicrab_dispho_Bkgd',
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
        inputPaths   = 'HLTprescaledpaths.txt'
        inputFilters = 'HLTfilters.txt'
         
        #--------------------------------------------------------
        # This is the base config:
        #--------------------------------------------------------
        from CRABClient.UserUtilities import config
        config = config()

        config.General.workArea    = options.workArea
        config.General.requestName = None

        config.JobType.pluginName  = 'Analysis'
        config.JobType.psetName    = 'dispho.py'
        config.JobType.pyCfgParams = None
        config.JobType.inputFiles  = [ inputDir+inputPaths , inputDir+inputFilters ]

        config.Data.inputDBS     = None
        config.Data.inputDataset = None
        config.Data.splitting    = 'EventAwareLumiBased'
        config.Data.unitsPerJob  = 10000

        config.Data.outputDatasetTag = None
        config.Data.publication      = False
        config.Site.storageSite      = 'T2_CH_CERN'
        config.Data.outLFNDirBase    = '/store/group/phys_exotica/displacedPhotons/'
        #--------------------------------------------------------

        # Will submit one task for each of these input datasets.
        inputDataAndOpts = [
            ['/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/RunIISummer17MiniAOD-92X_upgrade2017_realistic_v10_ext1-v2/MINIAODSIM', '14420', '1.'],
            ['/QCD_Pt-15to20_EMEnriched_TuneCUETP8M1_13TeV_pythia8/RunIISummer17MiniAOD-92X_upgrade2017_realistic_v10-v3/MINIAODSIM', '1263000000', '0.002'],
            ['/QCD_Pt-20to30_EMEnriched_TuneCUETP8M1_13TeV_pythia8/RunIISummer17MiniAOD-92X_upgrade2017_realistic_v10-v3/MINIAODSIM', '562000000', '0.0086'],
            ['/QCD_Pt-30to50_EMEnriched_TuneCUETP8M1_13TeV_pythia8/RunIISummer17MiniAOD-92X_upgrade2017_realistic_v10-v3/MINIAODSIM', '137000000', '0.05'],
            ['/QCD_Pt-50to80_EMEnriched_TuneCUETP8M1_13TeV_pythia8/RunIISummer17MiniAOD-92X_upgrade2017_realistic_v10-v3/MINIAODSIM', '19000000', '0.1'],
            ['/QCD_Pt-80to120_EMEnriched_TuneCUETP8M1_13TeV_pythia8/RunIISummer17MiniAOD-92X_upgrade2017_realistic_v10-v3/MINIAODSIM', '2800000', '0.1709'],
            ['/QCD_Pt-120to170_EMEnriched_TuneCUETP8M1_13TeV_pythia8/RunIISummer17MiniAOD-92X_upgrade2017_realistic_v10-v3/MINIAODSIM', '480000', '0.1429'],
            ['/QCD_Pt-170to300_EMEnriched_TuneCUETP8M1_13TeV_pythia8/RunIISummer17MiniAOD-92X_upgrade2017_realistic_v10-v2/MINIAODSIM', '122700', '0.17'],
            ]
 
        for inDO in inputDataAndOpts:
            # inDO[0] is of the form /A/B/C. Since B is unique for each inDO, use this in the CRAB request name.
            config.General.requestName   = inDO[0].split('/')[2]
            config.JobType.pyCfgParams   = ['globalTag=92X_upgrade2017_realistic_v10','phIDmin=none','splitPho=True','isBkgd=True',
                                            'xsec='+inDO[1],'filterEff='+inDO[2],
                                            'inputPaths='+inputPaths,'inputFilters='+inputFilters]
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
