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
            
            ['/GMSB_L100TeV_CTau0p1cm_step0/soffi-GMSB_L100TeV_CTau0p1cm_step3-401494f1e1638aa51280a90edc94ebc0/USER', '2.18318', '1', '0.9443385', 'isGMSB'],
            ['/GMSB_L100TeV_CTau10cm_step0/soffi-GMSB_L100TeV_CTau10cm_step3-7b4951d75b0cdfce8b97c445b952682b/USER', '2.19101', '1', '0.9443385', 'isGMSB'],
            ['/GMSB_L100TeV_CTau600cm_step0/soffi-GMSB_L100TeV_CTau600cm_step3-7b4951d75b0cdfce8b97c445b952682b/USER', '2.1437', '1', '0.944338560', 'isGMSB'],
            ['/GMSB_L100TeV_CTau1200cm_step0/soffi-GMSB_L100TeV_CTau1200cm_step3-dbd969891b3cd45cc603b8e4d5285af1/USER', '2.10999', '1', '0.944338381', 'isGMSB'],

            ['/GMSB_L150TeV_CTau0p1cm_step0/soffi-GMSB_L150TeV_CTau0p1cm_step3-401494f1e1638aa51280a90edc94ebc0/USER', '0.217511', '1', '0.857347369', 'isGMSB'],
            ['/GMSB_L150TeV_CTau10cm_step0/soffi-GMSB_L150TeV_CTau10cm_step3-5febe9b5e64145211cc865f9339571bf/USER', '0.21779', '1', '0.857347429', 'isGMSB'],
            ['/GMSB_L150TeV_CTau600cm_step0/soffi-GMSB_L150TeV_CTau600cm_step3-5febe9b5e64145211cc865f9339571bf/USER', '0.21713', '1', '0.857347429', 'isGMSB'],
            ['/GMSB_L150TeV_CTau1200cm_step0/soffi-GMSB_L150TeV_CTau1200cm_step3-85cad58e5cafa48fcf4b651c13572891/USER', '0.217984', '1', '0.857347369', 'isGMSB'],

            ['/GMSB_L200TeV_CTau0p1cm_step0/soffi-GMSB_L200TeV_CTau0p1cm_step3-401494f1e1638aa51280a90edc94ebc0/USER', '0.0401172', '1', '0.814181864', 'isGMSB'],
            ['/GMSB_L200TeV_CTau10cm_step0/soffi-GMSB_L200TeV_CTau10cm_step3-401494f1e1638aa51280a90edc94ebc0/USER', '0.0400735', '1', '0.814181864', 'isGMSB'],
            ['/GMSB_L200TeV_CTau600cm_step0/soffi-GMSB_L200TeV_CTau600cm_step3-401494f1e1638aa51280a90edc94ebc0/USER', '0.0400124', '1', '0.814181924', 'isGMSB'],
            ['/GMSB_L200TeV_CTau1200cm_step0/soffi-GMSB_L200TeV_CTau1200cm_step3-5b7c60e1fdaf50c5a7b959ba90f832d2/USER', '0.0403603', '1', '0.814181864', 'isGMSB'],

            ['/GMSB_L250TeV_CTau0p1cm_step0/soffi-GMSB_L250TeV_CTau0p1cm_step3-7b4951d75b0cdfce8b97c445b952682b/USER', '0.0109216', '1', '0.792751014', 'isGMSB'],
            ['/GMSB_L250TeV_CTau10cm_step0/soffi-GMSB_L250TeV_CTau10cm_step3-54655a0391212dbba2984afe28e21bc3/USER', '0.0109236', '1', '0.792751014', 'isGMSB'],
            ['/GMSB_L250TeV_CTau600cm_step0/soffi-GMSB_L250TeV_CTau600cm_step3-54655a0391212dbba2984afe28e21bc3/USER', '0.0109279', '1', '0.792751074', 'isGMSB'],
            ['/GMSB_L250TeV_CTau1200cm_step0/soffi-GMSB_L250TeV_CTau1200cm_step3-353c732828f8f1a8f78f268049f7b0ed/USER', '0.0109101', '1', '0.792751014', 'isGMSB'],

            ['/GMSB_L300TeV_CTau0p1cm_step0/soffi-GMSB_L300TeV_CTau0p1cm_step3-401494f1e1638aa51280a90edc94ebc0/USER', '0.00387135', '1', '0.780865729', 'isGMSB'],
            ['/GMSB_L300TeV_CTau10cm_step0/soffi-GMSB_L300TeV_CTau10cm_step3-e18eec80fff64b1d3595d955b2a6af85/USER', '0.0038706', '1', '0.780865788', 'isGMSB'],
            ['/GMSB_L300TeV_CTau600cm_step0/soffi-GMSB_L300TeV_CTau600cm_step3-e18eec80fff64b1d3595d955b2a6af85/USER', '0.00388035', '1', '0.780865729', 'isGMSB'],
            ['/GMSB_L300TeV_CTau1200cm_step0/soffi-GMSB_L300TeV_CTau1200cm_step3-c348d322117ba5f6f60a856979fd0e14/USER', '0.0038618', '1', '0.780865729', 'isGMSB'],

            ['/GMSB_L350TeV_CTau0p1cm_step0/soffi-GMSB_L350TeV_CTau0p1cm_step3-7b4951d75b0cdfce8b97c445b952682b/USER', '0.00161162', '1', '0.773663223', 'isGMSB'],
            ['/GMSB_L350TeV_CTau10cm_step0/soffi-GMSB_L350TeV_CTau10cm_step3-7c673dc440a3408d9f3a1cc7f05303ae/USER', '0.00160427', '1', '0.773663163', 'isGMSB'],
            ['/GMSB_L350TeV_CTau600cm_step0/soffi-GMSB_L350TeV_CTau600cm_step3-7c673dc440a3408d9f3a1cc7f05303ae/USER', '0.00161068', '1', '0.773663223', 'isGMSB'],
            ['/GMSB_L350TeV_CTau1200cm_step0/soffi-GMSB_L350TeV_CTau1200cm_step3-20e0fe7c912eb4ba0a9d90a66a270bb4/USER', '0.00160865', '1', '0.773663223', 'isGMSB'],

            ['/GMSB_L400TeV_CTau0p1cm_step0/soffi-GMSB_L400TeV_CTau0p1cm_step3-401494f1e1638aa51280a90edc94ebc0/USER', '0.00073187', '1', '0.768983364', 'isGMSB'],
            ['/GMSB_L400TeV_CTau10cm_step0/soffi-GMSB_L400TeV_CTau10cm_step3-cea60c68d10a6972420825d5e6a4c6f4/USER', '0.000731975', '1', '0.768983364', 'isGMSB'],
            ['/GMSB_L400TeV_CTau600cm_step0/soffi-GMSB_L400TeV_CTau600cm_step3-cea60c68d10a6972420825d5e6a4c6f4/USER', '0.000728273', '1', '0.768983364', 'isGMSB'],
            ['/GMSB_L400TeV_CTau1200cm_step0/soffi-GMSB_L400TeV_CTau1200cm_step3-f3bb7113b9142d3a7446a16dcc0fa766/USER', '0.000731161', '1', '0.768983305', 'isGMSB'],

            ]
 
        for inDO in inputDataAndOpts:
            # inDO[0] is of the form /A/B/C. Since A is unique for each inDO for Monte Carlo, use this in the CRAB request name.
            config.General.requestName   = inDO[0].split('/')[1]
            config.JobType.pyCfgParams   = ['globalTag=94X_mc2017_realistic_v14','splitPho=False','nThreads='+str(config.JobType.numCores),
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
