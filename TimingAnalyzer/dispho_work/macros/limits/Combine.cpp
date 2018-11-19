#include "Combine.hh"

namespace Combine
{
  // input maps
  std::vector<TString> RValVec;
  std::map<TString,GMSBinfo> GMSBMap;
  std::map<TString,std::vector<TString> > GMSBSubGroupMap;

  void SetupRValVec(const Bool_t doObserved)
  {
    std::cout << "Setting up RValVec..." << std::endl;

    // set up entries in tree, based on expectd, adjust accordingly
    RValVec = {"r2sigdown","r1sigdown","rexp","r1sigup","r2sigup"};

    // adjust for observed limit
    if (doObserved)
    {
      RValVec.insert(RValVec.begin(),"robs");
    }
  }
  
  void SetupGMSB(const TString & indir, const TString & infilename)
  {
    std::cout << "Setting up GMSB..." << std::endl;

    // read in parameters... 
    std::fstream inparams("signal_config/gmsb_params.txt");
    Float_t gen_ctau, mass, width, br;
    TString lambda, ctau;
    
    while (inparams >> lambda >> ctau >> gen_ctau >> mass >> width >> br)
    {
      const TString s_ctau = Common::ReplaceDotWithP(ctau);
      const TString name = "GMSB_L"+lambda+"TeV_CTau"+s_ctau+"cm";
      
      const Int_t i_lambda = lambda.Atoi();
      const Float_t f_ctau = ctau.Atof();
     
      Combine::GMSBMap[name] = {lambda,i_lambda,s_ctau,f_ctau,gen_ctau,mass,width,br};
    }
  
    // read in xsecs...
    std::fstream inxsecs("signal_config/gmsb_xsecs.txt");
    Float_t xsec, exsec;
    
    while (inxsecs >> lambda >> ctau >> xsec >> exsec)
    {
      const TString s_ctau = Common::ReplaceDotWithP(ctau);
      const TString name = "GMSB_L"+lambda+"TeV_CTau"+s_ctau+"cm";
      
      Combine::GMSBMap[name].xsec  = xsec;
      Combine::GMSBMap[name].exsec = exsec;
    }
    
    // read in r-values...
    for (auto & GMSBPair : Combine::GMSBMap)
    {
      const auto & name = GMSBPair.first;
      auto       & info = GMSBPair.second;
      
      // get file
      const TString filename = Form("%s/%s%s.root",indir.Data(),infilename.Data(),name.Data());
      auto isnull = Common::IsNullFile(filename);

      if (!isnull)
      {
	// open file
	auto infile = TFile::Open(Form("%s",filename.Data()));

	// get tree
	const TString treename = "limit";
	auto intree = (TTree*)infile->Get(Form("%s",treename.Data()));
	Common::CheckValidTree(intree,treename,filename);
	
	// get limit branch
	Double_t limit = 0; TBranch * b_limit = 0; TString s_limit = "limit";
	intree->SetBranchAddress(s_limit.Data(),&limit,&b_limit);

	// 5(6) Entries in tree, one for each quantile 
	for (auto ientry = 0U; ientry < intree->GetEntries(); ientry++)
	{
	  b_limit->GetEntry(ientry);
	  info.rvalmap[Combine::RValVec[ientry]] = limit;
	}
	
	// delete once done
	delete intree;
	delete infile;
      }
      else
      {
	for (const auto & RVal : RValVec)
	{
	  info.rvalmap[RVal] = -1.f;
	}
	
	std::cout << "skipping this file: " << filename.Data() << std::endl;
      }
    }
  }
  
  void RemoveGMSBSamples()
  {
    std::cout << "Removing GMSB Samples from GMSBMap..." << std::endl;

    std::vector<TString> keysToRemove;
    for (const auto & GMSBPair : Combine::GMSBMap)
    {
      const auto & name = GMSBPair.first;
      const auto & info = GMSBPair.second;

      if (info.rvalmap.at("rexp") < 0.f) keysToRemove.emplace_back(name);
    }
    
    for (const auto & keyToRemove : keysToRemove) Combine::GMSBMap.erase(keyToRemove);
  }
  
  void SetupGMSBSubGroups()
  {
    std::cout << "Setting up GMSB SubGroups..." << std::endl;

    for (const auto & GMSBPair : Combine::GMSBMap)
    {
      const auto & name = GMSBPair.first;
      const auto & info = GMSBPair.second;
      
      Combine::GMSBSubGroupMap["GMSB_CTau"+info.s_ctau+"cm"].emplace_back(name);
    }
  }
};
