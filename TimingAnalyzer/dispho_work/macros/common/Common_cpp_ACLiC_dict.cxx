// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME dIafsdIcerndOchdIworkdIkdIkmcdermodIprivatedIdisphodICMSSW_9_4_4dIsrcdITimingdITimingAnalyzerdIdispho_workdImacrosdIcommondICommon_cpp_ACLiC_dict

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Since CINT ignores the std namespace, we need to do so in this file.
namespace std {} using namespace std;

// Header files passed as explicit arguments
#include "/afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_9_4_4/src/Timing/TimingAnalyzer/dispho_work/macros/common/Common.cpp"

// Header files passed via #pragma extra_include

namespace Config {
   namespace ROOT {
      inline ::ROOT::TGenericClassInfo *GenerateInitInstance();
      static TClass *Config_Dictionary();

      // Function generating the singleton type initializer
      inline ::ROOT::TGenericClassInfo *GenerateInitInstance()
      {
         static ::ROOT::TGenericClassInfo 
            instance("Config", 0 /*version*/, "common/Common.hh", 26,
                     ::ROOT::Internal::DefineBehavior((void*)0,(void*)0),
                     &Config_Dictionary, 0);
         return &instance;
      }
      // Insure that the inline function is _not_ optimized away by the compiler
      ::ROOT::TGenericClassInfo *(*_R__UNIQUE_DICT_(InitFunctionKeeper))() = &GenerateInitInstance;  
      // Static variable to force the class initialization
      static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstance(); R__UseDummy(_R__UNIQUE_DICT_(Init));

      // Dictionary for non-ClassDef classes
      static TClass *Config_Dictionary() {
         return GenerateInitInstance()->GetClass();
      }

   }
}

namespace {
  void TriggerDictionaryInitialization_Common_cpp_ACLiC_dict_Impl() {
    static const char* headers[] = {
"common/Common.cpp",
0
    };
    static const char* includePaths[] = {
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/lcg/root/6.10.08/include",
"/afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_9_4_4/src",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/cms/cmssw/CMSSW_9_4_4/src",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/cms/coral/CORAL_2_3_21-fmblme4/include/LCG",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/mctester/1.25.0a-fmblme3/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/lcg/root/6.10.08/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/qt/4.8.7/include/QtDesigner",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/tauolapp/1.1.5-fmblme2/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/charybdis/1.003-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/thepeg/1.9.2p1-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/sherpa/2.2.4-fmblme2/include/SHERPA-MC",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/qt/4.8.7/include/QtOpenGL",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/qt/4.8.7/include/QtGui",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/pythia8/230/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/herwig/6.521-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/rivet/2.5.4/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/qt/4.8.7/include/Qt3Support",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/lwtnn/1.0-fmblme3/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/classlib/3.1.3-oenich/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/lhapdf/6.2.1-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/cgal/4.2-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/tkonlinesw/4.1.0-1-fmblme3/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/starlight/r193-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/qt/4.8.7/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/qt/4.8.7/include/Qt",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/qt/4.8.7/include/QtCore",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/qt/4.8.7/include/QtXml",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/mcdb/1.0.3/interface",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/libungif/4.1.4/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/libtiff/4.0.3-oenich/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/libpng/1.6.16-oenich/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/geant4/10.02.p02-fmblme3/include/Geant4",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/frontier_client/2.8.20-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/pcre/8.37-oenich/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/boost/1.63.0-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/xrootd/4.6.1-fmblme/include/xrootd/private",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/cms/vdt/0.3.9/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/valgrind/3.12.0-oenich/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/utm/utm_0.6.5-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/toprex/4.23/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/tbb/2018/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/tauola/27.121.5/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/sigcpp/2.6.2-oenich/include/sigc++-2.0",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/sqlite/3.16.2/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/protobuf/3.4.0-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/pacparser/1.3.5/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/oracle/12.1.0.2.0/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/meschach/1.2.pCMS1/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/madgraph5amcatnlo/2.4.3-fmblme3",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/libhepml/0.2.1/interface",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/ktjet/1.06-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/jimmy/4.2-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/jemalloc/4.5.0/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/xz/5.2.2-oenich/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/heppdt/3.03.00-fmblme2/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/hector/1.3.4_patch1-fmblme3/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/gsl/2.2.1/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/libjpeg-turbo/1.3.1/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/giflib/4.2.3-oenich/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/xerces-c/3.1.3/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/gdbm/1.10/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/freetype/2.5.3-oenich/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/fftw3/3.3.2/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/fftjet/1.5.0/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/fastjet/3.1.0/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/expat/2.1.0/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/hepmc/2.06.07-oenich/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/dpm/1.8.0.1/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/dcap/2.47.8-oenich/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/libxml2/2.9.1-oenich/include/libxml2",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/curl/7.52.1/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/cppunit/1.12.1/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/clhep/2.3.4.2-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/openssl/1.0.2d/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/pythia6/426/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/photos/215.5/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/zlib-x86_64/1.2.11/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/libuuid/2.22.2/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/castor/2.1.13.9/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/castor/2.1.13.9/include/shift",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/cascade/2.2.04-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/bz2lib/1.0.6/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/python/2.7.11-fmblme/include/python2.7",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/ittnotify/16.06.18-fmblme/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/gcc/6.3.0/include/c++/6.3.0",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/gcc/6.3.0/include/c++/6.3.0/x86_64-unknown-linux-gnu",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/gcc/6.3.0/include/c++/6.3.0/backward",
"/usr/local/include",
"/usr/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/lcg/root/6.10.08/etc",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/lcg/root/6.10.08/etc/cling",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/lcg/root/6.10.08/include",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/gcc/6.3.0/bin/../lib/gcc/x86_64-unknown-linux-gnu/6.3.0/../../../../include/c++/6.3.0",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/gcc/6.3.0/bin/../lib/gcc/x86_64-unknown-linux-gnu/6.3.0/../../../../include/c++/6.3.0/x86_64-unknown-linux-gnu",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/gcc/6.3.0/bin/../lib/gcc/x86_64-unknown-linux-gnu/6.3.0/../../../../include/c++/6.3.0/backward",
"/cvmfs/cms.cern.ch/slc6_amd64_gcc630/lcg/root/6.10.08/include",
"/afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_9_4_4/src/Timing/TimingAnalyzer/dispho_work/macros/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "Common_cpp_ACLiC_dict dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_Autoloading_Map;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "Common_cpp_ACLiC_dict dictionary payload"

#ifndef G__VECTOR_HAS_CLASS_ITERATOR
  #define G__VECTOR_HAS_CLASS_ITERATOR 1
#endif
#ifndef __ACLIC__
  #define __ACLIC__ 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
#include "common/Common.cpp"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[]={
"Config::CMSLumi", payloadCode, "@",
"Config::CheckValidFile", payloadCode, "@",
"Config::CheckValidTH1F", payloadCode, "@",
"Config::CheckValidTH2F", payloadCode, "@",
"Config::CheckValidTree", payloadCode, "@",
"Config::ColorMap", payloadCode, "@",
"Config::CutMap", payloadCode, "@",
"Config::GroupMap", payloadCode, "@",
"Config::HistNameMap", payloadCode, "@",
"Config::LabelMap", payloadCode, "@",
"Config::LabelOffset", payloadCode, "@",
"Config::LabelSize", payloadCode, "@",
"Config::Ndivisions", payloadCode, "@",
"Config::RemoveDelim", payloadCode, "@",
"Config::ReplaceDelimWithSpace", payloadCode, "@",
"Config::ReplaceSlashWithUnderscore", payloadCode, "@",
"Config::SampleMap", payloadCode, "@",
"Config::SetTDRStyle", payloadCode, "@",
"Config::SetupBins", payloadCode, "@",
"Config::SetupColors", payloadCode, "@",
"Config::SetupCuts", payloadCode, "@",
"Config::SetupGroups", payloadCode, "@",
"Config::SetupHistNames", payloadCode, "@",
"Config::SetupLabels", payloadCode, "@",
"Config::SetupSamples", payloadCode, "@",
"Config::SetupScale", payloadCode, "@",
"Config::TickLength", payloadCode, "@",
"Config::TitleFF", payloadCode, "@",
"Config::TitleSize", payloadCode, "@",
"Config::TitleXOffset", payloadCode, "@",
"Config::TitleYOffset", payloadCode, "@",
"Config::WeightString", payloadCode, "@",
"Config::baseDir", payloadCode, "@",
"Config::bottom_lp", payloadCode, "@",
"Config::bottom_up", payloadCode, "@",
"Config::configtreename", payloadCode, "@",
"Config::disphotreename", payloadCode, "@",
"Config::eosDir", payloadCode, "@",
"Config::etaEBcutoff", payloadCode, "@",
"Config::etaEEmax", payloadCode, "@",
"Config::extraText", payloadCode, "@",
"Config::h_cutflowname", payloadCode, "@",
"Config::height_lp", payloadCode, "@",
"Config::height_up", payloadCode, "@",
"Config::invfbToinvpb", payloadCode, "@",
"Config::left_lp", payloadCode, "@",
"Config::left_up", payloadCode, "@",
"Config::lumi", payloadCode, "@",
"Config::nEvCheck", payloadCode, "@",
"Config::nGMSBs", payloadCode, "@",
"Config::nHVDSs", payloadCode, "@",
"Config::nJets", payloadCode, "@",
"Config::nPhotons", payloadCode, "@",
"Config::nToys", payloadCode, "@",
"Config::puwgtFileName", payloadCode, "@",
"Config::puwgtHistName", payloadCode, "@",
"Config::right_lp", payloadCode, "@",
"Config::right_up", payloadCode, "@",
"Config::rootdir", payloadCode, "@",
"Config::top_lp", payloadCode, "@",
"Config::top_up", payloadCode, "@",
"Config::tupleFileName", payloadCode, "@",
nullptr};

    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("Common_cpp_ACLiC_dict",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_Common_cpp_ACLiC_dict_Impl, {}, classesHeaders);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_Common_cpp_ACLiC_dict_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_Common_cpp_ACLiC_dict() {
  TriggerDictionaryInitialization_Common_cpp_ACLiC_dict_Impl();
}
