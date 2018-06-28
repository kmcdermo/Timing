#include "Common.hh"
#include "TLatex.h"
#include "TColor.h"

namespace Common
{
  TString PrimaryDataset;
  std::map<TString,TString> SampleMap;
  std::map<TString,SampleGroup> GroupMap;
  std::map<TString,TString> SignalGroupMap;
  std::map<TString,vector<TString> > SignalSubGroupMap;
  std::map<TString,TString> TreeNameMap;
  std::map<TString,TString> HistNameMap;
  std::map<TString,TString> SampleCutFlowHistNameMap;
  std::map<TString,TString> GroupCutFlowHistNameMap;
  std::map<TString,TString> SignalCutFlowHistNameMap;
  std::map<TString,ColorStruct> SignalSubGroupColorMap;
  std::map<TString,Color_t> ColorMap;
  std::map<TString,TString> LabelMap;
  std::map<TString,TString> CutMap;
  std::vector<std::pair<TString,TString> > CutFlowPairVec;

  void SetupPrimaryDataset(const TString & pdname)
  {
    Common::PrimaryDataset = pdname;
  }

  void SetupSamples()
  {    
    // DYLL
    Common::SampleMap["MC/DYJetsToLL/base"] = "DYLL";
    Common::SampleMap["MC/DYJetsToLL/ext"]  = "DYLL";

    // DiPhoBox
    Common::SampleMap["MC/DiPhotonJetsBox/M40_80"]  = "DiPho";
    Common::SampleMap["MC/DiPhotonJetsBox/M80_Inf"] = "DiPho";

    // Data
    Common::SampleMap[Form("Data/%s/B/v1",Common::PrimaryDataset.Data())] = "Data";
    Common::SampleMap[Form("Data/%s/C/v1",Common::PrimaryDataset.Data())] = "Data";
    Common::SampleMap[Form("Data/%s/D/v1",Common::PrimaryDataset.Data())] = "Data";
    Common::SampleMap[Form("Data/%s/E/v1",Common::PrimaryDataset.Data())] = "Data";
    Common::SampleMap[Form("Data/%s/F/v1",Common::PrimaryDataset.Data())] = "Data";
  }

  void SetupSignalSamples()
  {
    const std::vector<TString> lambdas;
    const std::vector<TString> ctaus;

    // loop over all possible GMSBs...
    for (const auto & lambda : lambdas)
    {
      for (const auto & ctau : ctaus)
      {
	Common::SampleMap["MC/GMSB/L"+lambda+"TeV_CTau"+ctau+"cm"] = "GMSB_L"+lambda+"_CTau"+ctau;
      }
    }

    // temp...
    //    Common::SampleMap["MC/GMSB/L200TeV_CTau400cm"] = "GMSB_L200_CTau400";

    // eventually do HVDS...
  }

  void SetupGroups()
  {
    for (const auto & SamplePair : Common::SampleMap)
    {
      const auto & sample = SamplePair.second;
      
      if      (sample == "QCD")   Common::GroupMap[sample] = isBkgd;
      else if (sample == "GJets") Common::GroupMap[sample] = isBkgd;
      else if (sample == "DYLL")  Common::GroupMap[sample] = isBkgd;
      else if (sample == "DiPho") Common::GroupMap[sample] = isBkgd;
      else if (sample == "TX")    Common::GroupMap[sample] = isBkgd;
      else if (sample == "WX")    Common::GroupMap[sample] = isBkgd;
      else if (sample == "ZX")    Common::GroupMap[sample] = isBkgd;
      else if (sample == "Data")  Common::GroupMap[sample] = isData;
      else if (sample.Contains("GMSB")) Common::GroupMap[sample] = isSignal;
      else if (sample.Contains("HVDS")) Common::GroupMap[sample] = isSignal;
      else
      {
	std::cerr << "Aye... the Common groups are messed up!!! Fix it!!! Exiting..." << std::endl;
	exit(1);
      }
    }
  }

  void SetupSignalGroups()
  {
    for (const auto & SamplePair : Common::SampleMap)
    {
      const auto & sample = SamplePair.second;

      if      (sample.Contains("GMSB")) Common::SignalGroupMap[sample] = "GMSB";
      else if (sample.Contains("HVDS")) Common::SignalGroupMap[sample] = "HVDS";
    }
  }

  void SetupSignalSubGroups()
  {
    for (const auto & SampleGroupPair : Common::SignalGroupMap)
    {
      const auto & sample = SampleGroupPair.first;
      const auto & group  = SampleGroupPair.second;
    
      if (group == "GMSB")
      {
	const TString s_ctau = "_CTau";
	auto i_ctau = sample.Index(s_ctau);
	auto l_ctau = s_ctau.Length();
	
	const TString ctau(sample(i_ctau+l_ctau,sample.Length()-i_ctau-l_ctau));
	Common::SignalSubGroupMap["GMSB_CTau"+ctau+"cm"].emplace_back(sample);
      }
    }
  }
  
  void SetupTreeNames()
  {
    for (const auto & GroupPair : Common::GroupMap)
    {
      const auto & sample = GroupPair.first;
      Common::TreeNameMap[sample] = sample+"_Tree";
    }
  }

  void SetupHistNames()
  {
    for (const auto & GroupPair : Common::GroupMap)
    {
      const auto & sample = GroupPair.first;
      Common::HistNameMap[sample] = sample+"_Hist";
    }
  }

  void SetupSampleCutFlowHistNames()
  {
    for (const auto & SamplePair : Common::SampleMap)
    {
      const auto & input = SamplePair.first;
      const auto samplename = Common::ReplaceSlashWithUnderscore(input);

      Common::SampleCutFlowHistNameMap[input] = samplename+"_"+Common::h_cutflowname;
    }
  }

  void SetupGroupCutFlowHistNames()
  {
    for (const auto & GroupPair : Common::GroupMap)
    {
      const auto & sample = GroupPair.first;
      const auto group    = GroupPair.second;
      if (group == isSignal) continue;

      Common::GroupCutFlowHistNameMap[sample] = sample+"_"+Common::h_cutflowname;
    }
  }

  void SetupSignalCutFlowHistNames()
  {
    for (const auto & GroupPair : Common::GroupMap)
    {
      const auto & sample = GroupPair.first;
      const auto group    = GroupPair.second;
      if (group != isSignal) continue;

      Common::SignalCutFlowHistNameMap[sample] = sample+"_"+Common::h_cutflowname;
    }
  }

  void SetupSignalSubGroupColors()
  {
    // GMSB: add more as we go
    Common::SignalSubGroupColorMap["GMSB_CTau0p1cm"] = {kAzure,"Up"};
    Common::SignalSubGroupColorMap["GMSB_CTau10cm"]  = {kSpring,"Up"};
    Common::SignalSubGroupColorMap["GMSB_CTau400cm"] = {kPink,"Down"};
    Common::SignalSubGroupColorMap["GMSB_CTau600cm"] = {kViolet,"Down"};
    Common::SignalSubGroupColorMap["GMSB_CTau1200cm"] = {kOrange,"Down"};

    // deal with HVDS later...
  }

  void SetupColors()
  {
    Common::ColorMap["QCD"]   = kGreen;
    Common::ColorMap["GJets"] = kRed;
    Common::ColorMap["DYLL"]  = kMagenta;
    Common::ColorMap["DiPho"] = kCyan;
    Common::ColorMap["TX"]    = kOrange;
    Common::ColorMap["WX"]    = kOrange-6;
    Common::ColorMap["ZX"]    = kOrange+2;
    Common::ColorMap["Data"]  = kBlack;
    
    for (const auto & SignalSubGroupPair : Common::SignalSubGroupMap)
    {
      const auto & group   = SignalSubGroupPair.first;
      const auto & samples = SignalSubGroupPair.second;
    
      Int_t counter = 0;
      for (const auto & sample : samples)
      {
	Common::ColorMap[sample] = Common::SignalSubGroupColorMap[group].color+counter;
	
	if      (Common::SignalSubGroupColorMap[group].increment == "Up"  ) counter++;
	else if (Common::SignalSubGroupColorMap[group].increment == "Down") counter--;
	else    
	{
	  std::cerr << "Yikes, you messed up the signal sub group color config pretty bad, try again..." << std::endl;
	  exit(1);
	}
      }
    }
  }

  void SetupLabels()
  {
    Common::LabelMap["QCD"]   = "QCD"; //"#QCD (H_{T} Binned)";
    Common::LabelMap["GJets"] = "#gamma+Jets"; //"#gamma + Jets (H_{T} Binned)";
    Common::LabelMap["DYLL"]  = "DY#rightarrowLL+Jets";
    Common::LabelMap["DiPho"] = "2#gamma";
    Common::LabelMap["TX"]    = "T+X";
    Common::LabelMap["WX"]    = "W+X";
    Common::LabelMap["ZX"]    = "Z+X";
    Common::LabelMap["Data"]  = "Data";

    // GMSB Labels
    for (const auto & GroupPair : Common::GroupMap)
    {
      const auto & sample = GroupPair.first;
      const auto group    = GroupPair.second;
      
      if (group != isSignal) continue;
      if (Common::SignalGroupMap[sample] == "GMSB")
      {
	const TString s_lambda = "_L";
	auto i_lambda = sample.Index(s_lambda);
	auto l_lambda = s_lambda.Length();
	
	const TString s_ctau = "_CTau";
	auto i_ctau = sample.Index(s_ctau);
	auto l_ctau = s_ctau.Length();
	
	const TString lambda(sample(i_lambda+l_lambda,i_ctau-i_lambda-l_lambda));
	TString ctau(sample(i_ctau+l_ctau,sample.Length()-i_ctau-l_ctau));
	ctau.ReplaceAll("p",".");
      
	Common::LabelMap[sample] = "GMSB #Lambda:"+lambda+"TeV c#tau:"+ctau+"cm";
      }
    }

    // HVDS Labels when we get some samples...
  }

  void KeepOnlySamples(const std::vector<TString> & samplevec)
  {
    // erase by find
    for (auto iter = Common::SampleMap.cbegin(); iter != Common::SampleMap.cend();)
    {
      const auto & sample = iter->first;
      if (std::find(samplevec.begin(),samplevec.end(),sample) == samplevec.end())
      {
	Common::SampleMap.erase(iter++);
      }
      else
      {
	iter++;
      }
    }
  }

  void KeepOnlySignals()
  {
    // erase by key first
    for (const auto & GroupPair : Common::GroupMap)
    {
      const auto & sample = GroupPair.first;
      const auto & group  = GroupPair.second;

      if (group == isSignal) continue;

      Common::TreeNameMap.erase(sample);
      Common::HistNameMap.erase(sample);
      Common::ColorMap.erase(sample);
      Common::LabelMap.erase(sample);
    }

    // erase groups now
    for (auto iter = Common::GroupMap.cbegin(); iter != Common::GroupMap.cend();)
    {
      if (iter->second != isSignal) 
      {
	Common::GroupMap.erase(iter++);
      }
      else
      {
	iter++;
      }
    }
  }

  void SetupCuts(const TString & cutconfig)
  {
    std::cout << "Reading cut config..." << std::endl;

    std::ifstream infile(Form("%s",cutconfig.Data()),std::ios::in);
    std::string str;
    while (std::getline(infile,str))
    {
      if (str == "") continue;
      else if (str.find("common_cut=") != std::string::npos)
      {
	const TString cut = Common::RemoveDelim(str,"common_cut=");

	if (cut != "")
	{
	  for (const auto & GroupPair : GroupMap)
	  {
	    Common::CutMap[GroupPair.first] += Form("%s",cut.Data());
	  }
	}
      }
      else if (str.find("bkgd_cut=") != std::string::npos)
      {
	const TString cut = Common::RemoveDelim(str,"bkgd_cut=");

	if (cut != "")
	{
	  for (const auto & GroupPair : GroupMap)
	  {
	    if (GroupPair.second == isBkgd)
	    {
	      Common::CutMap[GroupPair.first] += Form("&&%s",cut.Data());
	    }
	  }
	}
      }
      else if (str.find("sign_cut=") != std::string::npos)
      {
	const TString cut = Common::RemoveDelim(str,"sign_cut=");

	if (cut != "")
	{
	  for (const auto & GroupPair : GroupMap)
	  {
	    if (GroupPair.second == isSignal)
	    {
	      Common::CutMap[GroupPair.first] += Form("&&%s",cut.Data());
	    }
	  }
	}
      }
      else if (str.find("data_cut=") != std::string::npos)
      {
	const TString cut = Common::RemoveDelim(str,"data_cut=");

	if (cut != "")
	{
	  for (const auto & GroupPair : GroupMap)
	  {
	    if (GroupPair.second == isData)
	    {
	      Common::CutMap[GroupPair.first] += Form("&&%s",cut.Data());
	    }
	  }
	}
      }
      else 
      {
	std::cerr << "Aye... your cut config is messed up, try again!" << std::endl;
	exit(1);
      }
    }
  }

  void SetupCutFlow(const TString & cutflowconfig)
  {
    std::cout << "Reading cut flow config..." << std::endl;

    std::ifstream infile(Form("%s",cutflowconfig.Data()),std::ios::in);
    TString label,cut;
    while (infile >> label >> cut)
    {
      Common::CutFlowPairVec.emplace_back(std::pair<TString,TString>{label,cut});
    }
  }

  void SetupBins(std::string & str, std::vector<Double_t> & bins, Bool_t & var_bins)
  {
    if      (str.find("CONSTANT") != std::string::npos)
    {
      var_bins = false;
      str = Common::RemoveDelim(str,"CONSTANT");
      Int_t nbins = 0; Double_t low = 0.f, high = 0.f;
      std::stringstream ss(str);
      ss >> nbins >> low >> high;
      Double_t bin_width = (high-low)/nbins;
      for (Int_t ibin = 0; ibin <= nbins; ibin++)
      {
	bins.push_back(low+ibin*bin_width);
      }
    } 
    else if (str.find("VARIABLE") != std::string::npos)
    {
      var_bins = true;
      str = Common::RemoveDelim(str,"VARIABLE");
      Float_t bin_edge;
      std::stringstream ss(str);
      while (ss >> bin_edge) bins.push_back(bin_edge);
    }
    else 
    {
      std::cerr << "Aye... bins are either VARIABLE or CONSTANT! Exiting..." << std::endl;
      exit(1);
    }
  }
    
  void SetupBinLabels(std::string & str, std::vector<TString> & binlabels)
  {
    std::stringstream ss(str);
    std::string binlabel;
    while (ss >> binlabel) binlabels.push_back(binlabel);
  }

  void SetupBlinding(const std::string & str, std::vector<BlindStruct> & blinding)
  {
    std::stringstream ss(str);
    std::string cutblock;
    while (ss >> cutblock)
    {
      cutblock = Common::RemoveDelim(cutblock,"(");
      cutblock = Common::RemoveDelim(cutblock,")");

      std::stringstream subss(cutblock);
      std::string cutvalue;
      std::vector<Float_t> tmpvec;
      while (std::getline(subss,cutvalue,','))
      {
	if      (cutvalue.find("-Inf") != std::string::npos) tmpvec.push_back(std::numeric_limits<float>::lowest());
	else if (cutvalue.find("+Inf") != std::string::npos) tmpvec.push_back(std::numeric_limits<float>::max());
	else    tmpvec.push_back(std::atof(cutvalue.c_str()));
      }

      // emplace parameters back as xlow,xup,ylow,yup
      if      (tmpvec.size() == 2) blinding.emplace_back(tmpvec[0],tmpvec[1]);
      else if (tmpvec.size() == 4) blinding.emplace_back(tmpvec[0],tmpvec[1],tmpvec[2],tmpvec[3]);
      else   
      {
	std::cerr << "Cutblock: " << cutblock.c_str() << " is formatted incorrectly! Exiting..." << std::endl;
	exit(1);
      }
    }
  } 

  void SetupScale(const std::string & str, Bool_t & scale)
  {
    if      (str.find("LOG") != std::string::npos)
    {
      scale = true;
    }
    else if (str.find("LIN") != std::string::npos)
    {
      scale = false;
    }
    else 
    {
      std::cerr << "Aye, scale is either LOG or LIN! Exiting..." << std::endl;
      exit(1);
    }
  }
  
  void SetupBool(const std::string & str, Bool_t & setting)
  {
    if      (str.find("1") != std::string::npos)
    {
      setting = true;
    }
    else if (str.find("0") != std::string::npos)
    {
      setting = false;
    }
    else 
    {
      std::cerr << "Aye, bool must be either 1 or 0! Exiting..." << std::endl;
      exit(1);
    }
  }

  void SetupWhichSamples(const TString & sampleconfig, std::vector<TString> & samplevec)
  {
    std::cout << "Reading sample config..." << std::endl;

    std::ifstream infile(Form("%s",sampleconfig.Data()),std::ios::in);
    std::string sample;
    while (infile >> sample)
    {
      samplevec.emplace_back(sample);
    }
  }
  
  void SetupWhichSignals(const std::string & str, std::vector<TString> & signalvec)
  {
    std::stringstream ss(str);
    std::string signal;
    while (ss >> signal)
    {
      if (signal == "*")
      {
	// loop over signals
	for (const auto & GroupPair : Common::GroupMap)
	{
	  const auto & sample = GroupPair.first;
	  const auto & group  = GroupPair.second;
	  
	  // skip non-signal samples
	  if (group != isSignal) continue;

	  signalvec.emplace_back(sample);
	}
      }
      else
      {
	if (str.find("!") != std::string::npos)
	{
	  signal = Common::RemoveDelim(signal,"!");
	  signalvec.erase(std::remove(signalvec.begin(), signalvec.end(), signal), signalvec.end()); 
	}
	else
	{
	  signalvec.emplace_back(signal);
	}
      }
    }
  }
  
  TH1F * SetupOutCutFlowHist(const TH1F * inhist, const TString & outname, std::map<TString,Int_t> & binlabels)
  {
    // get cut flow labels
    const auto inNbinsX = inhist->GetNbinsX();
    for (auto ibin = 1; ibin <= inNbinsX; ibin++) binlabels[inhist->GetXaxis()->GetBinLabel(ibin)] = ibin;
    
    Int_t inNbinsX_new = inNbinsX;
    for (const auto & SignalCutFlowPair : Common::CutFlowPairVec) binlabels[SignalCutFlowPair.first] = ++inNbinsX_new;
  
    // make new cut flow
    auto outhist = new TH1F(outname.Data(),inhist->GetTitle(),binlabels.size(),0,binlabels.size());
    outhist->Sumw2();
    
    for (const auto & binlabel : binlabels)
    {
      const auto & cut = binlabel.first;
      const auto ibin = binlabel.second;
      
      outhist->GetXaxis()->SetBinLabel(ibin,cut.Data());
      
      if (ibin > inNbinsX) continue;
      
      outhist->SetBinContent(ibin,inhist->GetBinContent(ibin));
      outhist->SetBinError(ibin,inhist->GetBinError(ibin));
    }
    
    outhist->GetYaxis()->SetTitle(inhist->GetYaxis()->GetTitle());
    
    return outhist;
  }

  void CheckValidFile(const TFile * file, const TString & fname)
  {
    if (file == (TFile*) NULL) // check if valid file
    {
      std::cerr << "Input file is bad pointer: " << fname.Data()
		<< " ...exiting..." << std::endl;
      exit(1);
    }
    else 
    {
      std::cout << "Successfully opened file: " << fname.Data() << std::endl;
    }
  }

  void CheckValidTree(const TTree * tree, const TString & tname, const TString & fname)
  {
    if (tree == (TTree*) NULL) // check if valid tree
    {
      std::cerr << "Input TTree is bad pointer: " << tname.Data() << " in input file: " << fname.Data()
		<< " ...exiting..." << std::endl;
      exit(1);
    }
    else 
    {
      std::cout << "Successfully opened tree: " << tname.Data() << " in input file: " << fname.Data() << std::endl;
    }
  }
  
  void CheckValidTH1F(const TH1F * plot, const TString & pname, const TString & fname)
  {
    if (plot == (TH1F*) NULL) // check if valid plot
    {
      std::cerr << "Input TH1F is bad pointer: " << pname.Data() << " in input file: " << fname.Data() 
		<< " ...exiting..." << std::endl;
      exit(1);
    }
  }

  void CheckValidTH1D(const TH1D * plot, const TString & pname, const TString & fname)
  {
    if (plot == (TH1D*) NULL) // check if valid plot
    {
      std::cerr << "Input TH1F is bad pointer: " << pname.Data() << " in input file: " << fname.Data() 
		<< " ...exiting..." << std::endl;
      exit(1);
    }
  }

  void CheckValidTH2F(const TH2F * plot, const TString & pname, const TString & fname)
  {
    if (plot == (TH2F*) NULL) // check if valid plot
    {
      std::cerr << "Input TH2F is bad pointer: " << pname.Data() << " in input file: " << fname.Data() 
		<< " ...exiting..." << std::endl;
      exit(1);
    }
  }

  Bool_t IsNullFile(const TString & filename)
  {
    struct stat buffer;
    return (stat (filename.Data(), &buffer) != 0);
  }

  Bool_t IsNullTree(const TTree * tree)
  {
    return (tree == (TTree*) NULL);
  }
  
  void AddTextFromInputConfig(TPaveText *& outpave, const TString & label, const TString & configname)
  {
    outpave->AddText(Form("%s: %s",label.Data(),configname.Data()));

    std::ifstream configfile(Form("%s",configname.Data()),std::ios::in);
    std::string str; // tmp string
    while (std::getline(configfile,str))
    {
      outpave->AddText(str.c_str());
    }
  }

  void AddTextFromInputPave(TPaveText *& outpave, TFile *& infile)
  {
    const auto inpave = (TPaveText*)infile->Get(Form("%s",Common::pavename.Data()));
    const auto lines  = inpave->GetListOfLines();

    TIter next(lines);
    while (auto text = (TText*)next())
    { 
      outpave->AddText(text->GetTitle());
    }
    
    delete inpave;
  }

  void Scale(TH2F *& hist, const Bool_t isUp, const Bool_t varBinsX, const Bool_t varBinsY)
  {
    std::cout << "Scaling " << (isUp?"up":"down") << " hist: " << hist->GetName() << std::endl;

    for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
    {
      const auto binwidthX = hist->GetXaxis()->GetBinWidth(ibinX);
      for (auto ibinY = 1; ibinY <= hist->GetYaxis()->GetNbins(); ibinY++)
      {
	const auto binwidthY = hist->GetYaxis()->GetBinWidth(ibinY);
	
	// get multiplier/divisor
	auto multiplier = 1.f;      
	if (varBinsX) multiplier *= binwidthX;
	if (varBinsY) multiplier *= binwidthY;
	
	// get content/error
	auto content = hist->GetBinContent(ibinX,ibinY);
	auto error   = hist->GetBinError  (ibinX,ibinY);
	
	// scale it
	if (isUp)
	{
	  content *= multiplier;
	  error   *= multiplier;
	}
	else
        {
	  content /= multiplier;
	  error   /= multiplier;
	}
	
	// set new contents
	hist->SetBinContent(ibinX,ibinY,content);
	hist->SetBinError  (ibinX,ibinY,error);
      }
    }
  }

  void Scale(TGraphAsymmErrors *& graph, const std::vector<Double_t> & bins, const Bool_t isUp)
  {
    std::cout << "Scaling " << (isUp?"up":"down") << " graph: " << graph->GetName() << std::endl;
    
    for (auto i = 0U; i < bins.size()-1; i++)
    {
      // get width
      const auto multiplier = bins[i+1]-bins[i];
      
      // get contents + errors
      Double_t xval,yval;
      graph->GetPoint(i,xval,yval);
      auto yerrl = graph->GetErrorYlow (i);
      auto yerrh = graph->GetErrorYhigh(i);
      
      // scale up or down
      if (isUp)
      {
	yval  *= multiplier;
	yerrl *= multiplier;
	yerrh *= multiplier;
      }
      else
      {
	yval  /= multiplier;
	yerrl /= multiplier;
	yerrh /= multiplier;
      }
      
      // set point with new values
      graph->SetPoint(i,xval,yval);
      graph->SetPointEYlow (i,yerrl);
      graph->SetPointEYhigh(i,yerrh);
    }
  }

  void Scale(TH1F *& hist, const Bool_t isUp)
  {
    std::cout << "Scaling " << (isUp?"up":"down") << " hist: " << hist->GetName() << std::endl;
    
    for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
    {
      // get width
      const auto multiplier = hist->GetXaxis()->GetBinWidth(ibinX);
      
      // get content and errors
      auto content = hist->GetBinContent(ibinX);
      auto error   = hist->GetBinError  (ibinX);
      
      // scale up or down
      if (isUp)
      {
	content *= multiplier;
	error   *= multiplier;
      }
      else
      {
	content /= multiplier;
	error   /= multiplier;
      }
      
      // set values
      hist->SetBinContent(ibinX,content);
      hist->SetBinError  (ibinX,error);
    }
  }

  void CMSLumi(TCanvas * canv, const Int_t iPosX) 
  {
    const TString  cmsText     = "CMS";
    const Double_t cmsTextFont = 61;  // default is helvetic-bold
    
    // extraText is either "Simulation" or "Preliminary"
    const Bool_t   writeExtraText  = !(Common::extraText.EqualTo("",TString::kExact));
    const Double_t extraTextFont   = 52;  // default is helvetica-italics

    const TString lumiText = Form("%5.2f fb^{-1} (13 TeV)", Common::lumi); // must change this spec once we are in fb range!
  
    // text sizes and text offsets with respect to the top frame
    // in unit of the top margin size
    const Double_t lumiTextSize     = 0.6;
    const Double_t lumiTextOffset   = 0.2;
    const Double_t cmsTextSize      = 0.75;
    const Double_t cmsTextOffset    = 0.1;  // only used in outOfFrame version

    const Double_t relPosX    = 0.045;
    const Double_t relPosY    = 0.035;
    const Double_t relExtraDY = 1.2;
 
    // ratio of "CMS" and extra text size
    const Double_t extraOverCmsTextSize  = 0.76;
 
    const Bool_t outOfFrame = (iPosX/10 == 0);

    Int_t alignY_=3;
    Int_t alignX_=2;
    if (iPosX    == 0) {alignY_ = 1;}
    if (iPosX/10 <= 1) {alignX_ = 1;}
    if (iPosX/10 == 2) {alignX_ = 2;}
    if (iPosX/10 == 3) {alignX_ = 3;}
    const Int_t align_ = 10*alignX_ + alignY_;

    const Double_t H = canv->GetWh();
    const Double_t W = canv->GetWw();
    const Double_t l = canv->GetLeftMargin();
    const Double_t t = canv->GetTopMargin();
    const Double_t r = canv->GetRightMargin();
    const Double_t b = canv->GetBottomMargin();
    const Double_t e = 0.025;

    TLatex latex;
    latex.SetNDC();
    latex.SetTextAngle(0);
    latex.SetTextColor(kBlack);    

    const Double_t extraTextSize = extraOverCmsTextSize*cmsTextSize;

    latex.SetTextFont(42);
    latex.SetTextAlign(31); 
    latex.SetTextSize(lumiTextSize*t);    
    latex.DrawLatex(1-r,1-t+lumiTextOffset*t,lumiText);

    if (outOfFrame) 
    {
      latex.SetTextFont(cmsTextFont);
      latex.SetTextAlign(11); 
      latex.SetTextSize(cmsTextSize*t);    
      latex.DrawLatex(l,1-t+lumiTextOffset*t,cmsText);
    }
  
    Double_t posX_ = 0;
    if (iPosX%10 <= 1)  
    {
      posX_ =  l + relPosX*(1-l-r);
    }
    else if (iPosX%10 == 2) 
    {
      posX_ =  l + 0.5*(1-l-r);
    }
    else if (iPosX%10 == 3) 
    {
      posX_ =  1-r - relPosX*(1-l-r);
    }
    
    Double_t posY_ = 1-t - relPosY*(1-t-b);

    if (!outOfFrame) 
    {
      latex.SetTextFont(cmsTextFont);
      latex.SetTextSize(cmsTextSize*t);
      latex.SetTextAlign(align_);
      latex.DrawLatex(posX_, posY_, cmsText);
      
      if (writeExtraText) 
      {
	latex.SetTextFont(extraTextFont);
	latex.SetTextAlign(align_);
	latex.SetTextSize(extraTextSize*t);
	latex.DrawLatex(posX_, posY_- relExtraDY*cmsTextSize*t, Common::extraText);
      }
    }
  
    else if (outOfFrame && writeExtraText)
    {
      if (iPosX == 0) 
      {
	posX_ = l +  relPosX*(1-l-r)+0.05;
	posY_ = 1-t+lumiTextOffset*t;
      }
      latex.SetTextFont(extraTextFont);
      latex.SetTextSize(extraTextSize*t);
      latex.SetTextAlign(align_);
      latex.DrawLatex(posX_, posY_, Common::extraText);
    }
  }

  void SetTDRStyle(TStyle * tdrStyle)
  {  
    // For the canvas:
    tdrStyle->SetCanvasBorderMode(0);
    tdrStyle->SetCanvasColor(kWhite);
    tdrStyle->SetCanvasDefH(600); //Height of canvas
    tdrStyle->SetCanvasDefW(700); //Width of canvas
    tdrStyle->SetCanvasDefX(0);   //Position on screen
    tdrStyle->SetCanvasDefY(0);

    // For the Pad:
    tdrStyle->SetPadBorderMode(0);
    // tdrStyle->SetPadBorderSize(Width_t size = 1);
    tdrStyle->SetPadColor(kWhite);
    tdrStyle->SetPadGridX(false);
    tdrStyle->SetPadGridY(false);
    tdrStyle->SetGridColor(0);
    tdrStyle->SetGridStyle(3);
    tdrStyle->SetGridWidth(1);

    // For the frame:
    tdrStyle->SetFrameBorderMode(0);
    tdrStyle->SetFrameBorderSize(1);
    tdrStyle->SetFrameFillColor(0);
    tdrStyle->SetFrameFillStyle(0);
    tdrStyle->SetFrameLineColor(1);
    tdrStyle->SetFrameLineStyle(1);
    tdrStyle->SetFrameLineWidth(1);

    // For the histo:
    tdrStyle->SetHistLineColor(1);
    tdrStyle->SetHistLineStyle(0);
    tdrStyle->SetHistLineWidth(1);
    tdrStyle->SetEndErrorSize(2);
    tdrStyle->SetErrorX(0.5);
    tdrStyle->SetMarkerStyle(20);
    tdrStyle->SetMarkerSize(0.6);

    //For the fit/function:
    tdrStyle->SetOptFit(0);
    tdrStyle->SetFitFormat("5.4g");
    tdrStyle->SetFuncColor(2);
    tdrStyle->SetFuncStyle(1);
    tdrStyle->SetFuncWidth(1);

    //For the date:
    tdrStyle->SetOptDate(0);
    // tdrStyle->SetDateX(Float_t x = 0.01);
    // tdrStyle->SetDateY(Float_t y = 0.01);

    // For the statistics box:
    tdrStyle->SetOptFile(0);
    tdrStyle->SetOptStat(0); // To display the mean and RMS:   SetOptStat("mr");
    tdrStyle->SetStatColor(kWhite);
    tdrStyle->SetStatFont(42);
    tdrStyle->SetStatFontSize(0.025);
    tdrStyle->SetStatTextColor(1);
    tdrStyle->SetStatFormat("6.4g");
    tdrStyle->SetStatBorderSize(1);
    tdrStyle->SetStatH(0.1);
    tdrStyle->SetStatW(0.15);
  
    // Margins:
    tdrStyle->SetPadTopMargin(0.05);
    tdrStyle->SetPadBottomMargin(0.14);
    tdrStyle->SetPadLeftMargin(0.18);
    tdrStyle->SetPadRightMargin(0.15); 

    // For the Global title:

    tdrStyle->SetOptTitle(0);
    tdrStyle->SetTitleFont(42);
    tdrStyle->SetTitleColor(1);
    tdrStyle->SetTitleTextColor(1);
    tdrStyle->SetTitleFillColor(10);
    tdrStyle->SetTitleFontSize(0.05);
    // tdrStyle->SetTitleH(0); // Set the height of the title box
    // tdrStyle->SetTitleW(0); // Set the width of the title box
    // tdrStyle->SetTitleX(0); // Set the position of the title box
    // tdrStyle->SetTitleY(0.985); // Set the position of the title box
    // tdrStyle->SetTitleStyle(Style_t style = 1001);
    // tdrStyle->SetTitleBorderSize(2);

    // For the axis titles:
    tdrStyle->SetTitleColor(1, "XYZ");
    tdrStyle->SetTitleFont(42, "XYZ");
    tdrStyle->SetTitleSize(Common::TitleSize, "XYZ");
    tdrStyle->SetTitleXOffset(Common::TitleXOffset);
    tdrStyle->SetTitleYOffset(Common::TitleYOffset);

    // For the axis labels:

    tdrStyle->SetLabelColor(1, "XYZ");
    tdrStyle->SetLabelFont(42, "XYZ");
    tdrStyle->SetLabelOffset(Common::LabelOffset, "XYZ");
    tdrStyle->SetLabelSize(Common::LabelSize, "XYZ");

    // For the axis:

    tdrStyle->SetAxisColor(1, "XYZ");
    tdrStyle->SetStripDecimals(kTRUE);
    tdrStyle->SetTickLength(Common::TickLength, "XYZ");
    tdrStyle->SetNdivisions(Common::Ndivisions, "X");
    tdrStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
    tdrStyle->SetPadTickY(1);

    // Change for log plots:
    tdrStyle->SetOptLogx(0);
    tdrStyle->SetOptLogy(0);
    tdrStyle->SetOptLogz(0);

    // Postscript options:
    // tdrStyle->SetPaperSize(15.,15.);
    // tdrStyle->SetLineScalePS(Float_t scale = 3);
    // tdrStyle->SetLineStyleString(Int_t i, const char* text);
    // tdrStyle->SetHeaderPS(const char* header);
    // tdrStyle->SetTitlePS(const char* pstitle);

    // tdrStyle->SetBarOffset(Float_t baroff = 0.5);
    // tdrStyle->SetBarWidth(Float_t barwidth = 0.5);
    // tdrStyle->SetPaintTextFormat(const char* format = "g");
    // tdrStyle->SetPalette(Int_t ncolors = 0, Int_t* colors = 0);
    // tdrStyle->SetTimeOffset(Double_t toffset);
    // tdrStyle->SetHistMinimumZero(kTRUE);
  
    // for a nice color palette
    const Int_t NRGBs = 5;
    const Int_t NCont = 255;
  
    Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
    Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
    Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    tdrStyle->SetNumberContours(NCont);

    tdrStyle->cd();
    gROOT->ForceStyle();
  }
};
