#include "Common.cpp+"

void SetupCommon(const TString & dataname)
{
  Common::SetupPrimaryDataset(dataname);
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
}

void testForTrees()
{
  const TString skimdir = "madv2_v1";
  SetupCommon("SinglePhoton");

  for (const auto & SamplePair : Common::SampleMap)
  {
    // Init
    const auto & input    = SamplePair.first;
    const auto samplename = Common::ReplaceSlashWithUnderscore(input);

    std::cout << "Looking at: " << input << std::endl;

    // Get File
    const TString filename = Form("%s/%s/%s/%s/%s",Common::eosDir.Data(),Common::baseDir.Data(),skimdir.Data(),input.Data(),Common::tupleFileName.Data());
    auto isnullfile = Common::IsNullFile(filename);
    if (isnullfile)
    {
      std::cout << "sample: " << samplename.Data() << " has null file" << std::endl;
      continue;
    }
    auto file = TFile::Open(Form("%s",filename.Data()));
    file->cd();
    
    // Get TTree
    auto tree = (TTree*)file->Get(Form("%s",Common::disphotreename.Data()));
    auto isnulltree = Common::IsNullTree(tree);
    if (isnulltree)
    {
      std::cout << "sample: " << samplename.Data() << " has null tree" << std::endl;
    }
    
    // Get Input Cut Flow Histogram 
    auto hist = (TH1F*)file->Get(Form("%s",Common::h_cutflow_scaledname.Data()));
    auto isnullhist = Common::IsNullHist(hist);
    if (isnullhist)
    {
      std::cout << "sample: " << samplename.Data() << " has null hist" << std::endl;
    }
  }
}
