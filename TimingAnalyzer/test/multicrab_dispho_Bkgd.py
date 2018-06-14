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
        config.Data.unitsPerJob  = 500000

        config.Data.outputDatasetTag = None
        config.Data.publication      = False
        config.Site.storageSite      = 'T2_CH_CERN'
        config.Data.outLFNDirBase    = '/store/user/kmcdermo/nTuples/unskimmed/analysis/'
        #--------------------------------------------------------

        # Will submit one task for each of these input datasets.
        inputDataAndOpts = [

            #### GJets HT Binned ####
            ['/GJets_HT-40To100_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM' , '18620'   , '1', '1'],
            ['/GJets_HT-100To200_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v2/MINIAODSIM', '8625'    , '1', '1'],
            ['/GJets_HT-200To400_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v2/MINIAODSIM', '2196'    , '1', '1'],
            ['/GJets_HT-400To600_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v2/MINIAODSIM', '258'     , '1', '1'],
            ['/GJets_HT-600ToInf_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '85.18'   , '1', '1'],
            
            #### QCD HT Binned ####
            ['/QCD_HT100to200_TuneCP5_13TeV-madgraph-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM'  , '23670000', '1', '1'],
            ['/QCD_HT200to300_TuneCP5_13TeV-madgraph-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM'  ,  '1554000', '1', '1'],
            ['/QCD_HT300to500_TuneCP5_13TeV-madgraph-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM'  ,   '323800', '1', '1'],
            ['/QCD_HT500to700_TuneCP5_13TeV-madgraph-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM'  ,    '30010', '1', '1'],
            ['/QCD_HT700to1000_TuneCP5_13TeV-madgraph-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM' ,     '6352', '1', '1'],
            ['/QCD_HT1000to1500_TuneCP5_13TeV-madgraph-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM',     '1096', '1', '1'],
            ['/QCD_HT1500to2000_TuneCP5_13TeV-madgraph-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM',    '99.12', '1', '1'],
            ['/QCD_HT2000toInf_TuneCP5_13TeV-madgraph-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM' ,    '20.20', '1', '1'],

            #### Drell-Yan ####
            ['/DYJetsToLL_M-50_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIFall17MiniAOD-RECOSIMstep_94X_mc2017_realistic_v10-v1/MINIAODSIM', '5349', '1', '1'],
            
            #### Diphoton NLO ####
            ['/DiPhotonJetsBox_M40_80-Sherpa/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v3/MINIAODSIM', '308.3', '1', '1'],
            ['/DiPhotonJetsBox_MGG-80toInf_13TeV-Sherpa/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v3/MINIAODSIM', '87.54', '1', '1'],

            #### Top+X ####
            ['/TGGJets_TuneCP5_PSweights_13TeV-MadGraph-madspin-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', ' 0.007726', '1', '1'],
            ['/TTGJets_TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '4.106', '1', '1'],
            ['/TTJets_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '726.5', '1', '1'],
            ['/ttWJets_TuneCP5_13TeV_madgraphMLM_pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '0.4607', '1', '1'],
            ['/ttZJets_TuneCP5_13TeV_madgraphMLM_pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v2/MINIAODSIM', '0.5426', '1', '1'],

            #### W -> e nu ####
            ['/WToENu_M-100_TuneCP5_13TeV-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '174.1', '1', '1'],
            ['/WToENu_M-200_TuneCP5_13TeV-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '7.266', '1', '1'],
            ['/WToENu_M-500_TuneCP5_13TeV-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '0.2503', '1', '1'],
            ['/WToENu_M-1000_TuneCP5_13TeV-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '0.01495', '1', '1'],

            #### W+X ####
            ['/WGGJets_TuneCP5_13TeV_madgraphMLM_pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '1.693', '1', '1'],
            ['/WJetsToLNu_2J_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIIFall17MiniAOD-PU2017_94X_mc2017_realistic_v11-v1/MINIAODSIM', '1940', '1', '1'], # xsec from McM
            ['/WWTo2L2Nu_NNPDF31_TuneCP5_13TeV-powheg-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '11.08', '1', '1'],
            ['/WWToLNuQQ_NNPDF31_TuneCP5_13TeV-powheg-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '45.99', '1', '1'],
            ['/WZ_TuneCP5_13TeV-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '36.27', '1', '1'],
            ['/WZG_TuneCP5_13TeV-amcatnlo-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '0.04345', '1', '1'],
            
            #### Z+X ####
            ['/ZGGJetsToLLGG_5f_LO_amcatnloMLM_pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '0.6550', '1', '1'],
            ['/ZGGJets_ZToHadOrNu_5f_LO_madgraph_pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '0.3570', '1', '1'],
            ['/ZZ_TuneCP5_13TeV-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '12.14', '1', '1'],
            ['/ZJetsToNuNu_HT-100To200_13TeV-madgraph/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '304.1', '1', '1'],
            ['/ZJetsToNuNu_HT-200To400_13TeV-madgraph/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '91.79', '1', '1'],
            ['/ZJetsToNuNu_HT-400To600_13TeV-madgraph/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '13.12', '1', '1'],
            ['/ZJetsToNuNu_HT-600To800_13TeV-madgraph/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '3.259', '1', '1'],
            ['/ZJetsToNuNu_HT-800To1200_13TeV-madgraph/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '1.499', '1', '1'],
            ['/ZJetsToNuNu_HT-1200To2500_13TeV-madgraph/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '0.343', '1', '1'],
            ['/ZJetsToNuNu_HT-2500ToInf_13TeV-madgraph/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '0.005146', '1', '1'],

            #### extra samples not really needed ####
            # ['/DYJetsToLL_M-50_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIFall17MiniAOD-RECOSIMstep_94X_mc2017_realistic_v10_ext1-v1/MINIAODSIM', '5349', '1', '1'],
            # ['/WToENu_M-2000_TuneCP5_13TeV-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '0.0004366', '1', '1'],
            # ['/WToENu_M-3000_TuneCP5_13TeV-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '0.00002785', '1', '1'],
            # ['/WToENu_M-4000_TuneCP5_13TeV-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '0.000002614', '1', '1'],
            # ['/WToENu_M-5000_TuneCP5_13TeV-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '0.0000003798', '1', '1'],
            # ['/WToENu_M-6000_TuneCP5_13TeV-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '0.00000007366', '1', '1'],
            # ['/W1JetsToLNu_LHEWpT_50-150_TuneCP5_13TeV-amcnloFXFX-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '0', '1', '1'],
            # ['/W1JetsToLNu_LHEWpT_100-150_TuneCP5_13TeV-amcnloFXFX-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '0', '1', '1'],
            # ['/W1JetsToLNu_LHEWpT_150-250_TuneCP5_13TeV-amcnloFXFX-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '0', '1', '1'],
            # ['/W1JetsToLNu_LHEWpT_250-400_TuneCP5_13TeV-amcnloFXFX-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '0', '1', '1'],
            # ['/W1JetsToLNu_LHEWpT_400-inf_TuneCP5_13TeV-amcnloFXFX-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '0', '1', '1'],
            # ['/W2JetsToLNu_LHEWpT_250-400_TuneCP5_13TeV-amcnloFXFX-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '0', '1', '1'],
            # ['/W2JetsToLNu_LHEWpT_400-inf_TuneCP5_13TeV-amcnloFXFX-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '0', '1', '1'],
            # ['/W3JetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v3/MINIAODSIM', '993.6', '1', '1'],
            # ['/W4JetsToLNu_TuneCP5_13TeV-madgraphMLM-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v10-v1/MINIAODSIM', '545.8', '1', '1'],
            # ['/Z1JetsToNuNu_M-50_LHEZpT_150-250_TuneCP5_13TeV-amcnloFXFX-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '18.04', '1', '1'],
            # ['/Z1JetsToNuNu_M-50_LHEZpT_250-400_TuneCP5_13TeV-amcnloFXFX-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '2.051', '1', '1'],
            # ['/Z1JetsToNuNu_M-50_LHEZpT_400-inf_TuneCP5_13TeV-amcnloFXFX-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '0.224', '1', '1'],
            # ['/Z2JetsToNuNu_M-50_LHEZpT_150-250_TuneCP5_13TeV-amcnloFXFX-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '29.66', '1', '1'],
            # ['/Z2JetsToNuNU_M-50_LHEZpT_400-inf_TuneCP5_13TeV-amcnloFXFX-pythia8/RunIIFall17MiniAOD-94X_mc2017_realistic_v11-v1/MINIAODSIM', '0.8458', '1', '1'],

            ]
        
        for inDO in inputDataAndOpts:
            # inDO[0] is of the form /A/B/C. Since A is (mostly) unique for each inDO in Monte Carlo, use this in the CRAB request name.
            datasetName = inDO[0].split('/')[1]
            if "_ext" in inDO[0] : datasetName += "_ext"

            config.General.requestName   = datasetName
            config.JobType.pyCfgParams   = ['globalTag=94X_mc2017_realistic_v14','splitPho=False','nThreads='+str(config.JobType.numCores),
                                            'xsec='+inDO[1],'filterEff='+inDO[2],'BR='+inDO[3],'isBkgd=True',
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
