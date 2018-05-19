#include "Combine.hh"

namespace Combine
{
  // input maps
  std::map<TString,Int_t> EntryMap;
  std::map<TString,GMSBinfo> GMSBMap;
  std::map<TString,std::vector<TString> > GMSBSubGroupMap;

  void SetupEntryMap(const Bool_t doObserved)
  {
    // set up entries in tree, based on expectd, adjust accordingly
    Combine::EntryMap["r2sigdown"] = 0;
    Combine::EntryMap["r1sigdown"] = 1;
    Combine::EntryMap["rexp"]      = 2;
    Combine::EntryMap["r1sigup"]   = 3;
    Combine::EntryMap["r2sigup"]   = 4;
    
    // adjust for observed limit
    if (doObserved)
    {
      for (auto & EntryPair : Combine::EntryMap)
      {
	auto & entry = EntryPair.second;
	entry += 1;
      }
      Combine::EntryMap["robs"] = 0;
    }
  }
  
  void SetupGMSB(const TString & filename)
  {
    // read in parameters... 
    std::fstream inparams("signal_config/all_params.txt");
    Float_t mass, width, br;
    TString lambda, ctau;
    
    while (inparams >> lambda >> ctau >> mass >> width >> br)
    {
      const TString s_ctau = (ctau.EqualTo("0.1") ? "0p1" : ctau);
      const TString name = "GMSB_L"+lambda+"TeV_CTau"+s_ctau+"cm";
      
      const Int_t i_lambda = lambda.Atoi();
      const Float_t f_ctau = ctau.Atof();
      
      Combine::GMSBMap[name] = {lambda,i_lambda,s_ctau,f_ctau,mass,width,br};
    }
  
    // read in xsecs...
    std::fstream inxsecs("signal_config/all_xsecs.txt");
    Float_t xsec, exsec;
    
    while (inxsecs >> lambda >> ctau >> xsec >> exsec)
    {
      const TString s_ctau = (ctau.EqualTo("0.1") ? "0p1" : ctau);
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
      auto infile = TFile::Open(Form("%s",filename.Data()));
      auto isnull = Common::IsNullFile(infile);

      if (!isnull)
      {
	// get tree
	const TString treename = "limit";
	auto intree = (TTree*)infile->Get(Form("%s",treename.Data()));
	Common::CheckValidTree(intree,treename,filename);
	
	// get limit branch
	Double_t limit = 0; TBranch * b_limit = 0; TString s_limit = "limit";
	intree->SetBranchAddress(s_limit.Data(),&limit,&b_limit);

	// 5(6) Entries in tree, one for each quantile 
	if (fDoObserved)
        {
	  b_limit->GeEntry(Combine::EntryMap["robs"]);
	  info.robs = limit;
	}
	else info.robs = -1;
	
	// 2sigdown
	b_limit->GetEntry(Combine::EntryMap["r2sigdown"]);
	info.r2sigdown = limit;
	
	// 1sigdown
	b_limit->GetEntry(Combine::EntryMap["r1sigdown"]);
	info.r1sigdown = limit;
	
	// expected
	b_limit->GetEntry(Combine::EntryMap["rexp"]);
	info.rexp = limit;
	
	// 1sigup
	b_limit->GetEntry(Combine::EntryMap["r1sigup"]);
	info.r1sigup = limit;
	
	// 2sigup
	b_limit->GetEntry(Combine::EntryMap["r2sigup"]);
	info.r2sigup = limit;
	
	// delete once done
	delete intree;
	delete infile;
      }
      else
      {
	info.robs      = -1.f;
	info.r2sigdown = -1.f;
	info.r1sigdown = -1.f;
	info.rexp      = -1.f;
	info.r1sigup   = -1.f;
	info.r2sigup   = -1.f;
	
	std::cout << "skipping this file: " << filename.Data() << std::endl;
      }
    }
  }
  
  void RemoveGMSBSamples()
  {
    std::vector<TString> keysToRemove;
    for (const auto & GMSBPair : Combine::GMSBMap)
    {
      const auto & name = GMSBPair.first;
      const auto & info = GMSBPair.second;
      
      if (info.rexp < 0.f) keysToRemove.emplace_back(name);
    }
    
    for (const auto & keyToRemove : keysToRemove) Combine::GMSBMap.erase(keyToRemove);

    // ****************** HACK FOR NOW *************** //
    Combine::GMSBMap.erase("GMSB_L200TeV_CTau400cm");
  }
  
  void SetupGMSBSubGroups()
  {
    for (const auto & GMSBPair : Combine::GMSBMap)
    {
      const auto & name = GMSBPair.first;
      const auto & info = GMSBPair.second;
      
      Config::GMSBSubGroupMap["GMSB_CTau"+info.s_ctau+"cm"].emplace_back(name);
    }
  }
};
