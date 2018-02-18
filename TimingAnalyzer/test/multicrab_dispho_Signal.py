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

        config.Data.inputDBS     = 'phys03'
        config.Data.inputDataset = None
        config.Data.splitting    = 'EventAwareLumiBased'
        config.Data.unitsPerJob  = 10000

        config.Data.outputDatasetTag = None
        config.Data.publication      = False
        config.Site.storageSite      = 'T2_CH_CERN'
        config.Data.outLFNDirBase    = '/store/user/kmcdermo/nTuples/unskimmed/analysis/'
        #--------------------------------------------------------

        # Will submit one task for each of these input datasets.
        inputDataAndOpts = [
            ['/GMSB_L200TeV_CTau400cm_930/kmcdermo-GMSB_L200TeV_CTau400cm_930_step3-23134fac048c68b5122d77328802e60f/USER', '0.04', '1', '0.81418', 'isGMSB'],

            ['/HVDS_MZP500_MDP20_Ctau100mm/kmcdermo-HVDS_MZP500_MDP20_Ctau100mm_step3-65c6b29ab922da94b788da955c09b417/USER', '0.0298386', '1', '1', 'isHVDS'],
            ['/HVDS_MZP500_MDP20_Ctau5000mm/kmcdermo-HVDS_MZP500_MDP20_Ctau5000mm_step3-901620c07ae079e429c8dec5206b980f/USER', '0.0298836', '1', '1', 'isHVDS'],
            ['/HVDS_MZP500_MDP60_Ctau100mm/kmcdermo-HVDS_MZP500_MDP60_Ctau100mm_step3-05194d539d32b57b7b59c72b87bd3ce4/USER', '0.0298075', '1', '1', 'isHVDS'],
            ['/HVDS_MZP500_MDP60_Ctau5000mm/kmcdermo-HVDS_MZP500_MDP60_Ctau5000mm_step3-d231aac082cf30daa42f4c217757031e/USER', '0.0300165', '1', '1', 'isHVDS'],

            ['/GMSB_L200TeV_CTau10cm/kmcdermo-GMSB_L200TeV_CTau10cm_step3_v2-65c6b29ab922da94b788da955c09b417/USER', '0.0402926', '1', '0.814181864', 'isGMSB'],
            ['/GMSB_L200TeV_CTau400cm/kmcdermo-GMSB_L200TeV_CTau400cm_step3_v2-901620c07ae079e429c8dec5206b980f/USER', '0.0399241', '1', '0.814181805', 'isGMSB'],
            ['/GMSB_L400TeV_CTau10cm/kmcdermo-GMSB_L400TeV_CTau10cm_step3_v2-05194d539d32b57b7b59c72b87bd3ce4/USER', '0.000731795', '1', '0.7768983364', 'isGMSB'],
            ['/GMSB_L400TeV_CTau400cm/kmcdermo-GMSB_L400TeV_CTau400cm_step3_v2-d231aac082cf30daa42f4c217757031e/USER', '0.00113937', '1', '0.768983364', 'isGMSB'],
            ]
 
        for inDO in inputDataAndOpts:
            # inDO[0] is of the form /A/B/C. Since A is unique for each inDO for Monte Carlo, use this in the CRAB request name.
            config.General.requestName   = inDO[0].split('/')[1]
            config.JobType.pyCfgParams   = ['globalTag=94X_mc2017_realistic_v10','splitPho=False','nThreads='+str(config.JobType.numCores),
                                            'xsec='+inDO[1],'filterEff='+inDO[2],'BR='+inDO[3],inDO[4]+'=True',
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
