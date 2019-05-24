#include "Common.cpp+"

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TEntryList.h"

#include <vector>

void SetupConfig(const TString & sampleconfig, const TString & cutflowconfig)
{
  std::vector<TString> samples;

  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupWhichSamples(sampleconfig,samples);
  Common::KeepOnlySamples(samples);
  Common::SetupGroups();
  Common::SetupTreeNames();
  Common::SetupSampleCutFlowHistNames();
  Common::SetupGroupCutFlowHistNames();
  Common::SetupCutFlow(cutflowconfig);
}

void copyIntoFile(const TString & sampleconfig, const TString & cutflowconfig, const TString & infilename, const TString & outfilename)
{
  SetupConfig(sampleconfig,cutflowconfig);

  // check inputs
  const auto & infile  = TFile::Open(Form("%s",infilename.Data()));
  Common::CheckValidFile(infile,infilename);

  auto outfile = TFile::Open(Form("%s",outfilename.Data()),"UPDATE");
  Common::CheckValidFile(outfile,outfilename);

  // samples
  std::cout << "Copying sample lists and hists first..." << std::endl; 
  for (const auto & SamplePair : Common::SampleMap)
  {
    const auto & input = SamplePair.first;
    const auto & samplename = Common::ReplaceSlashWithUnderscore(input);

    std::cout << "Working on input: " << input.Data() << std::endl;

    const auto & histname = Common::SampleCutFlowHistNameMap[input];
    const auto & hist = (TH1F*)infile->Get(histname.Data());
    Common::CheckValidHist(hist,histname,infilename);

    Common::Write(outfile,hist);
    delete hist;

    for (const auto & CutFlowPair : Common::CutFlowPairVec)
    {
      const auto & label = CutFlowPair.first;

      std::cout << "Copying list for cut: " << label.Data() << std::endl;

      const auto & entrylistname = samplename+"_"+label+"%_EntryList";
      const auto & entrylist = (TEntryList*)infile->Get(entrylistname.Data());
      Common::CheckValidEntryList(entrylist,entrylistname,infilename);

      Common::Write(outfile,entrylist);
      delete entrylist;
    }
  }

  // groups
  std::cout << "Copying group hists and trees second..." << std::endl; 
  for (const auto & GroupPair : Common::GroupMap)
  {
    const auto & sample = GroupPair.first;

    std::cout << "Working on group: " << sample.Data() << std::endl;

    const auto & histname = Common::GroupCutFlowHistNameMap[sample];
    const auto & hist = (TH1F*)infile->Get(histname.Data());
    Common::CheckValidHist(hist,histname,infilename);

    Common::Write(outfile,hist);
    delete hist;

    const auto & intreename = Common::TreeNameMap[sample];
    const auto & intree = (TTree*)infile->Get(intreename.Data());
    Common::CheckValidTree(intree,intreename,infilename);

    const auto & outtree = intree->CopyTree("");
    Common::Write(outfile,outtree);
    delete outtree;
    delete intree;
  }

  delete outfile;
  delete infile;
}
