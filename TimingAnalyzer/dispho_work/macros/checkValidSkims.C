#include "Common.cpp+"
#include <iomanip>

void checkValidSkims(const TString & pdname, const TString & inskimdir)
{
  Common::SetupPrimaryDataset(pdname);
  Common::SetupSamples();

  for (const auto & SamplePair : Common::SampleMap)
  {
    // Init
    const auto & input    = SamplePair.first;
    const auto samplename = Common::ReplaceSlashWithUnderscore(input);

    // Get File
    const TString filename = Form("%s/%s/%s/%s/%s",Common::eosDir.Data(),Common::baseDir.Data(),inskimdir.Data(),input.Data(),Common::tupleFileName.Data());
    if (Common::IsNullFile(filename))
    {
      std::cerr << "Bad File: " << filename.Data() << std::endl;
      continue;
    }
    auto file = TFile::Open(Form("%s",filename.Data()));
    file->cd();
    
    // Get TTree
    auto tree = (TTree*)file->Get(Form("%s",Common::configtreename.Data()));
    if (Common::IsNullTree(tree))
    {
      std::cerr << "Bad Tree: " << Common::configtreename.Data() << std::endl;
      continue;
    }
    
    // Get Branches
    auto sumWgts = 0.f, sampleWeight = 0.f; 
    TBranch * b_sumWgts, * b_sampleWeight;
    const std::string s_sumWgts = "sumWgts", s_sampleWeight = "sampleWeight";
    tree->SetBranchAddress(s_sumWgts.c_str(),&sumWgts,&b_sumWgts);
    tree->SetBranchAddress(s_sampleWeight.c_str(),&sampleWeight,&b_sampleWeight);
    b_sumWgts->GetEntry(0);
    b_sampleWeight->GetEntry(0);
    //    const auto tree_integral = sumWgts * sampleWeight;
    const auto tree_integral = sumWgts;

    // Get Input Cut Flow Histogram
    //    auto hist = (TH1F*)file->Get(Form("%s",Common::h_cutflow_scaledname.Data()));
    auto hist = (TH1F*)file->Get(Form("%s",Common::h_cutflow_wgtname.Data()));
    if (Common::IsNullHist(hist))
    {
      std::cerr << "Bad Hist: " << Common::h_cutflow_scaledname.Data() << std::endl;
      continue;
    }
   
    // Find bin and get content
    const TString binlabel = "Good Photon";
    auto hist_integral = 0.f;
    for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
    {
      if (binlabel.EqualTo(hist->GetXaxis()->GetBinLabel(ibinX),TString::kExact))
      {
	hist_integral = hist->GetBinContent(ibinX);
	break;
      }
    }
    
    // Now check!
    const auto percent = (std::abs(tree_integral - hist_integral) / tree_integral) * 100.f;
    if (percent > 1e-4)
    {
      std::cout << samplename.Data() << " is invalid! Missing: "
		<< std::setw(5) << percent << "%" << std::endl;
    }
  }
}
