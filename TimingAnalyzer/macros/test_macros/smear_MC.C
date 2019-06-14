#include "Common.cpp+"

// config
constexpr auto ibinLow = 3;
constexpr auto ibinUp  = 16;
constexpr auto nFit    = 20;
static const auto hist_file_name = Common::eosPreFix+"/"+Common::eosDir+"/"+Common::calibDir+"/"+"phoE_0_Zee_EB_Full_SmearInput_230419_Gaus2fm.root";
static const TString dir = "skims/v4p1/final/categories_v2p1";
static const TString category = "inclusive_2pho";
static const auto bkgd_file_name   = dir+"/"+category+".root";
static const auto signal_file_name = dir+"/"+"signals_"+category+".root";
static const TString time_name = "phoweightedtimeLT120SMEAR_Gaus2fm_0";

// prototypes
TH1F * GetHist(const TString & histname, TFile * hist_file);

// main macro
void smear_MC()
{
  // config
  auto rand = new TRandomMixMax();
  gRandom = rand;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupSignalGroups();
  Common::SetupTreeNames();

  // input file
  auto hist_file = TFile::Open(hist_file_name.Data());
  Common::CheckValidFile(hist_file,hist_file_name);

  // input hists
  auto mc_sigma    = GetHist("MC_sigma"   ,hist_file);
  auto data_alpha  = GetHist("Data_alpha" ,hist_file);
  auto data_sigma1 = GetHist("Data_sigma1",hist_file);
  auto data_sigma2 = GetHist("Data_sigma2",hist_file);

  // input fits
  std::map<Int_t,TF1*> fitmap;
  for (auto ibin = ibinLow; ibin <= ibinUp; ibin++)
  {
    // input hists
    auto data_hist = (TH1D*)hist_file->Get(Form("Data_Hist_ibin%i",ibin));

    // smear values
    const auto alpha  = data_alpha->GetBinContent(ibin);
    const auto sigma1 = std::sqrt(std::pow(data_sigma1->GetBinContent(ibin),2)-std::pow(mc_sigma->GetBinContent(ibin),2));
    const auto sigma2 = std::sqrt(std::pow(data_sigma2->GetBinContent(ibin),2)-std::pow(mc_sigma->GetBinContent(ibin),2));

    // mapped fits
    fitmap[ibin] = new TF1(Form("formula_%i",ibin),Form("%f*exp(-pow((x/%f),2)/2)+(1.0-%f)*exp(-pow((x/%f),2)/2)",alpha,sigma1,alpha,sigma2),
			   data_hist->GetXaxis()->GetBinLowEdge(1),data_hist->GetXaxis()->GetBinUpEdge(data_hist->GetXaxis()->GetNbins()));
    
    // delete it all
    delete data_hist;
  }
  // delete data inputs
  delete data_sigma2;
  delete data_sigma1;
  delete data_alpha;

  // input files
  auto bkgd_file = TFile::Open(bkgd_file_name.Data(),"UPDATE");
  Common::CheckValidFile(bkgd_file,bkgd_file_name);

  auto signal_file = TFile::Open(signal_file_name.Data(),"UPDATE");
  Common::CheckValidFile(signal_file,signal_file_name);

  // loop over trees
  for (const auto & TreeNamePair : Common::TreeNameMap)
  {
    // Init
    const auto & sample   = TreeNamePair.first;
    const auto & treename = TreeNamePair.second;
 
    // Skip data
    if (Common::GroupMap[sample] == SampleGroup::isData) continue;

    // Get infile
    auto & io_file = ((Common::GroupMap[sample] != SampleGroup::isSignal) ? bkgd_file : signal_file);
    io_file->cd();

    // Get TTree
    auto io_tree = (TTree*)io_file->Get(Form("%s",treename.Data()));
    const auto isnull = Common::IsNullTree(io_tree);

    if (!isnull)
    {
      std::cout << "Smearing sample: " << sample.Data() << std::endl;

      // branches
      auto phoE_0 = 0.f; TBranch * b_phoE_0 = 0; io_tree->SetBranchAddress("phoE_0",&phoE_0,&b_phoE_0);
      auto photimeSMEAR_0 = 0.f; TBranch * b_photimeSMEAR_0 = io_tree->Branch(time_name.Data(),&photimeSMEAR_0,time_name+"/F");
  
      // loop entries
      const auto nentries = io_tree->GetEntries();
      for (auto ientry = 0U; ientry < nentries; ientry++)
      {
	if (ientry % Common::nEvCheck == 0 || ientry == 0) std::cout << "  Entry: " << ientry << " out of: " << nentries << std::endl;
	
	// get energy
	b_phoE_0->GetEntry(ientry);
	
	// get bin
	auto ibin = mc_sigma->FindBin(phoE_0);
	if      (ibin < 1) ibin = 1;
	else if (ibin > mc_sigma->GetXaxis()->GetNbins()) ibin = mc_sigma->GetXaxis()->GetNbins();
	
	photimeSMEAR_0 = fitmap.at(ibin)->GetRandom();
	b_photimeSMEAR_0->Fill();
      }

      // write and elete
      Common::Write(io_file,io_tree);
      delete io_tree;
    }
    else
    {
      std::cout << "Skipping sample: " << sample.Data() << std::endl;
    }
  } // end loop over trees

  // delete files
  delete signal_file;
  delete bkgd_file;

  // delete the rest
  delete mc_sigma;
  delete hist_file;
  delete rand;
}

TH1F * GetHist(const TString & histname, TFile * hist_file)
{
  auto hist = (TH1F*)hist_file->Get(histname.Data());
  Common::CheckValidHist(hist,histname,hist_file->GetName());
  return hist;
}
