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

  const auto & infile  = TFile::Open(Form("%s",infilename.Data()));
  auto outfile = TFile::Open(Form("%s",outfilename.Data()),"UPDATE");

  // samples
  for (const auto & SamplePair : Common::SampleMap)
  {
    const auto & input = SamplePair.first;
    const auto & samplename = Common::ReplaceSlashWithUnderscore(input);

    const auto & hist = (TH1F*)infile->Get(Common::SampleCutFlowHistNameMap[input].Data());
    outfile->cd();
    hist->Write(hist->GetName(),TObject::kWriteDelete);
    delete hist;

    for (const auto & CutFlowPair : Common::CutFlowPairVec)
    {
      const auto & label = CutFlowPair.first;

      const auto & list = (TEntryList*)infile->Get(Form("%s_%s_EntryList",samplename.Data(),label.Data()));
      outfile->cd();
      list->Write(list->GetName(),TObject::kWriteDelete);
      delete list;
    }
  }

  // groups
  for (const auto & GroupPair : Common::GroupMap)
  {
    const auto & sample = GroupPair.first;
    const auto & samplename = Common::ReplaceSlashWithUnderscore(sample);

    const auto & hist = (TH1F*)infile->Get(Common::GroupCutFlowHistNameMap[sample].Data());
    outfile->cd();
    hist->Write(hist->GetName(),TObject::kWriteDelete);
    delete hist;

    const auto & intree = (TTree*)infile->Get(Common::TreeNameMap[sample].Data());
    const auto & outtree = intree->CopyTree("");
    outfile->cd();
    outtree->Write(outtree->GetName(),TObject::kWriteDelete);
    delete outtree;
    delete intree;
  }

  delete outfile;
  delete infile;
}
