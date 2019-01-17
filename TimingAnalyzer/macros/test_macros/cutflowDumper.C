#include "Common.cpp+"

void cutflowDumper(const TString & infilename, const TString & outfiletext)
{
  // setup config
  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupGroupCutFlowHistNames();

  // get input file
  auto infile = TFile::Open(infilename.Data());
  Common::CheckValidFile(infile,infilename);

  // setup input hist map
  std::map<TString,TH1F*> HistMap;
  for (const auto & GroupPair : Common::GroupMap)
  {
    const auto & sample     = GroupPair.first;
    const auto & inhistname = Common::GroupCutFlowHistNameMap[sample];
    HistMap[sample] = (TH1F*)infile->Get(Form("%s",inhistname.Data()));
    Common::CheckValidHist(HistMap[sample],inhistname,infilename);
  }

  // get bin labels (use data)
  std::vector<TString> BinLabelVec;
  const auto & DataHist = HistMap["Data"];
  const auto nbinsX = DataHist->GetXaxis()->GetNbins();
  for (auto ibinX = 1; ibinX <= nbinsX; ibinX++) 
  {
    BinLabelVec.emplace_back(DataHist->GetXaxis()->GetBinLabel(ibinX));
  }

  // setup order of dump by data, then counts
  std::vector<TString> SampleVec;
  for (const auto & GroupPair : Common::GroupMap)
  {
    SampleVec.emplace_back(GroupPair.first);
  }
  std::sort(SampleVec.begin(),SampleVec.end(),
	    [&](const auto & sample1, const auto & sample2)
	    {
	      if      (sample1 == "Data") return sample1 > sample2;
	      else if (sample2 == "Data") return sample2 > sample1;
	      else 
	      {
		return HistMap[sample1]->GetBinContent(nbinsX) > HistMap[sample2]->GetBinContent(nbinsX);
	      }
	    }); 
  

  // init output textfile
  ofstream output(outfiletext+".csv",std::ios::trunc);
  
  // first dump names of Hists
  for (const auto & Sample : SampleVec)
  {
    output << "," << Sample.Data();
  }
  output << std::endl;

  // loop over bins, dump it
  for (auto ibinX = 1; ibinX <= nbinsX; ibinX++)
  {
    output << BinLabelVec[ibinX-1].Data();
    
    for (const auto & Sample : SampleVec)
    {
      const auto & hist = HistMap[Sample];
      output <<  ","  << std::setprecision(3) << hist->GetBinContent(ibinX) 
	     << "+/-" << std::setprecision(3) << hist->GetBinError  (ibinX);
    }
   
    output << std::endl;
  }
}
