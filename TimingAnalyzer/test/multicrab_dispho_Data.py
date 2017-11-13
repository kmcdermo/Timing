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
                      default = 'multicrab_dispho_Data',
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
        inputPaths   = 'HLTpathsWPho50.txt'
        inputFilters = 'HLTfilters.txt'
        inputJSON    = 'golden2017-nov3.json'
         
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

        config.Data.inputDataset = None
        config.Data.lumiMask     = inputDir+inputJSON
        config.Data.splitting    = 'EventAwareLumiBased'
        config.Data.unitsPerJob  = 1000000

        config.Data.outputDatasetTag = None
        config.Data.publication      = False
        config.Site.storageSite      = 'T2_CH_CERN'
        config.Data.outLFNDirBase    = '/store/user/kmcdermo/'
        #--------------------------------------------------------

        # Will submit one task for each of these input datasets.
        inputDataAndOpts = [
            ['/SinglePhoton/Run2017B-PromptReco-v1/MINIAOD', '92X_dataRun2_Prompt_v4', 'False'],
            ['/SinglePhoton/Run2017B-PromptReco-v2/MINIAOD', '92X_dataRun2_Prompt_v5', 'False'],
            ['/SinglePhoton/Run2017C-PromptReco-v1/MINIAOD', '92X_dataRun2_Prompt_v6', 'True'],
            ['/SinglePhoton/Run2017C-PromptReco-v2/MINIAOD', '92X_dataRun2_Prompt_v7', 'True'],
            ['/SinglePhoton/Run2017C-PromptReco-v3/MINIAOD', '92X_dataRun2_Prompt_v8', 'True'],
            ['/SinglePhoton/Run2017D-PromptReco-v1/MINIAOD', '92X_dataRun2_Prompt_v8', 'True'],
            ['/SinglePhoton/Run2017E-PromptReco-v1/MINIAOD', '92X_dataRun2_Prompt_v9', 'True'],
            ['/SinglePhoton/Run2017F-PromptReco-v1/MINIAOD', '92X_dataRun2_Prompt_v9', 'True'],
            ]
 
        for inDO in inputDataAndOpts:
            # inDO[0] is of the form /A/B/C. Since B is unique for each inDS, use this in the CRAB request name.
            config.General.requestName   = inDO[0].split('/')[2]
            config.JobType.pyCfgParams   = ['globalTag='+inDO[1],'useOOTPhotons='+inDO[2],'phIDmin=none','splitPho=False',
                                            'applyBlindSF=True','blindSF=10',
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
