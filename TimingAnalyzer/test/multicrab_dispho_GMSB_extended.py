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
        config.JobType.pyCfgParams = None
        config.JobType.inputFiles  = [ inputDir+inputPaths , inputDir+inputFilters , inputDir+inputFlags ]

        config.Data.inputDataset = None
        config.Data.inputDBS = 'phys03'

        config.Data.outputDatasetTag = None
        config.Data.publication      = False
        config.Site.storageSite      = 'T2_CH_CERN'
        config.Data.outLFNDirBase    = '/store/group/phys_exotica/displacedPhotons/nTuples/2017/analysis/unskimmed'
        #--------------------------------------------------------

        # Will submit one task for each of these input datasets.
        inputDataAndOpts = [
            ['/GMSB_L100TeV_Ctau1cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L100TeV_Ctau1cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'    , '1.866', '1', '9.44338560E-01'],
            ['/GMSB_L100TeV_Ctau50cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L100TeV_Ctau50cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'  , '1.876', '1', '9.44338560E-01'],
            ['/GMSB_L100TeV_Ctau100cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L100TeV_Ctau100cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER', '1.879', '1', '9.44338441E-01'],

            ['/GMSB_L150TeV_Ctau1cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L150TeV_Ctau1cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'    , '0.2014', '1', '8.57347310E-01'],
            ['/GMSB_L150TeV_Ctau50cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L150TeV_Ctau50cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'  , '0.2012', '1', '8.57347310E-01'],
            ['/GMSB_L150TeV_Ctau100cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L150TeV_Ctau100cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER', '0.2022', '1', '8.57347369E-01'],
            
            ['/GMSB_L200TeV_Ctau1cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L200TeV_Ctau1cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'    , '4.024e-2', '1', '8.14181864E-01'],
            ['/GMSB_L200TeV_Ctau50cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L200TeV_Ctau50cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'  , '4.037e-2', '1', '8.14181864E-01'],
            ['/GMSB_L200TeV_Ctau100cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L200TeV_Ctau100cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER', '4.028e-2', '1', '8.14181805E-01'],
            
            ['/GMSB_L250TeV_Ctau1cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L250TeV_Ctau1cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'    , '1.159e-2', '1', '7.92750955E-01'],
            ['/GMSB_L250TeV_Ctau50cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L250TeV_Ctau50cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'  , '1.161e-2', '1', '7.92751014E-01'],
            ['/GMSB_L250TeV_Ctau100cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L250TeV_Ctau100cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER', '1.163e-2', '1', '7.92751074E-01'],
            
            ['/GMSB_L300TeV_Ctau1cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L300TeV_Ctau1cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'    , '4.184e-3', '1', '7.80865729E-01'],
            ['/GMSB_L300TeV_Ctau50cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L300TeV_Ctau50cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'  , '4.193e-3', '1', '7.80865729E-01'],
            ['/GMSB_L300TeV_Ctau100cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L300TeV_Ctau100cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER', '4.189e-3', '1', '7.80865729E-01'],
            
            ['/GMSB_L350TeV_Ctau1cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L350TeV_Ctau1cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'    , '1.713e-3', '1', '7.73663223E-01'],
            ['/GMSB_L350TeV_Ctau50cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L350TeV_Ctau50cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'  , '1.726e-3', '1', '7.73663223E-01'],
            ['/GMSB_L350TeV_Ctau100cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L350TeV_Ctau100cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER', '1.713e-3', '1', '7.73663223E-01'],
            
            ['/GMSB_L400TeV_Ctau1cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L400TeV_Ctau1cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'    , '7.494e-4', '1', '7.68983364E-01'],
            ['/GMSB_L400TeV_Ctau50cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L400TeV_Ctau50cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER'  , '7.474e-4', '1', '7.68983364E-01'],
            ['/GMSB_L400TeV_Ctau100cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L400TeV_Ctau100cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER', '7.503e-4', '1', '7.68983364E-01'],

            ['/GMSB_L200TeV_Ctau200cm_13TeV-pythia8/zhicaiz-crab_CMSSW_9_4_7_GMSB_L200TeV_Ctau200cm_MINIAODSIM_CaltechT2_10May2019-5f646ecd4e1c7a39ab0ed099ff55ceb9/USER', '4.0014e-2', '1', '8.14181864E-01']
            ]

        for inDO in inputDataAndOpts:
            # inDO[0] is of the form /A/B/C. Since A is unique for each inDO for Monte Carlo, use this in the CRAB request name.
            config.General.requestName   = inDO[0].split('/')[1]
            config.JobType.pyCfgParams   = ['globalTag=94X_mc2017_realistic_v17',
                                            'xsec='+inDO[1],'filterEff='+inDO[2],'BR='+inDO[3],'isGMSB=True',
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
