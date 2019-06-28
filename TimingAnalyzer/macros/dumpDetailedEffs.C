#include "Common.cpp+"

void dumpDetailedEffs(const TString & category, const TString ctau)
{
  // get files
  const TString indir = "skims/v4p3/signals";

  const auto sign_file_name = indir+"/"+category+".root";
  auto sign_file = TFile::Open(sign_file_name);
  Common::CheckValidFile(sign_file,sign_file_name);
  
  std::vector<TString> names = {"GMSB_L100_CTau"+ctau,"GMSB_L200_CTau"+ctau,"GMSB_L300_CTau"+ctau,"GMSB_L400_CTau"+ctau};
  const auto nnames = names.size();

  // vector of hist names
  std::vector<TString> histnames = names;
  for (auto & histname : histnames) histname += "_"+Common::h_cutflow_scaledname;

  // vector of hists
  std::vector<TH1F*> hists(nnames);
  for (auto iname = 0U; iname < nnames; iname++)
  {
    const auto & histname = histnames[iname];
    auto & hist = hists[iname];
    hist = (TH1F*)sign_file->Get(histname.Data());
    Common::CheckValidHist(hist,histname,sign_file_name);
  }

  // vector of tree names
  std::vector<TString> treenames = names;
  for (auto & treename : treenames) treename += "_Tree";

  std::vector<TTree*> trees(nnames);
  for (auto iname = 0U; iname < nnames; iname++)
  {
    const auto & treename = treenames[iname];
    auto & tree = trees[iname];
    tree = (TTree*)sign_file->Get(treename.Data());
    Common::CheckValidTree(tree,treename,sign_file->GetName());
  }  

  // output
  std::ofstream output(category+"_"+ctau+"cm."+Common::outTextExt,std::ios::trunc);

  // loop over nbins
  const auto nbins = hists.front()->GetXaxis()->GetNbins();
  for (auto ibin = 1; ibin <= nbins; ibin++)
  {
    std::cout << "working on bin: " << ibin << std::endl;
    for (auto iname = 0U; iname < nnames; iname++)
    {
      // get scale factor
      auto & tree = trees[iname];
      Float_t evtwgt; TBranch * b_evtwgt; tree->SetBranchAddress("evtwgt",&evtwgt,&b_evtwgt);
      tree->GetEntry(0);

      const auto & hist = hists[iname];
      const auto denom = hist->GetBinContent(1);
      const auto numer = hist->GetBinContent(ibin);
      const auto ratio = numer / denom;
      const auto error = 1.96*std::sqrt((ratio*(1-ratio))/(denom/evtwgt));

      if (iname == 0) 
      {
	output << Form("%s & %s",hist->GetXaxis()->GetBinLabel(ibin),Common::PrintValueAndError(100*ratio,100*error).Data());
      }
      else
      {
	output << Form(" & %s",Common::PrintValueAndError(100*ratio,100*error).Data());
      }
    }
    output << " \\\\" << std::endl;
  }

  // delete it all
  Common::DeleteVec(trees);
  Common::DeleteVec(hists);
  delete sign_file;
}
