#include "CombinePreparer.hh"

CombinePreparer::CombinePreparer(const TString & infilename, const TString & bininfoname,
				 const TString & ratioinfoname, const TString & binratioinfoname,
				 const TString & systfilename, const TString & wsname,
				 const TString & datacardname, const Bool_t blinddata,
				 const Bool_t savemetadata, const TString & wsfilename)

  : fInFileName(infilename), fBinInfoName(bininfoname),
    fRatioInfoName(ratioinfoname), fBinRatioInfoName(binratioinfoname),
    fSystFileName(systfilename), fWSName(wsname), 
    fDatacardName(datacardname), fBlindData(blinddata),
    fSaveMetaData(savemetadata), fWSFileName(wsfilename)
{
  std::cout << "Initializing CombinePreparer..." << std::endl;

  // Setup Config
  CombinePreparer::SetupCommon();
  CombinePreparer::SetupABCD();
  CombinePreparer::ReadSystematics();
  
  // Get input file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()));
  Common::CheckValidFile(fInFile,fInFileName);
  fInFile->cd();

  // Get input hists
  const auto datahistname = Common::HistNameMap["Data"]+"_Plotted";
  DataHist = (TH2F*)fInFile->Get(datahistname.Data());
  Common::CheckValidHist(DataHist,datahistname,fInFileName);

  // Setup binning scheme
  ABCD::SetupNBins(DataHist);

  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s",fWSFileName.Data()),"RECREATE");
}

CombinePreparer::~CombinePreparer()
{
  std::cout << "Tidying up in the destructor..." << std::endl;

  // delete it all
  if (fSaveMetaData) delete fConfigPave;
  delete fOutFile;
  delete DataHist;
  delete fInFile;
}

void CombinePreparer::PrepareCombine()
{
  std::cout << "Preparing inputs to combine" << std::endl;

  for (const auto & HistNamePair : Common::HistNameMap)
  {
    const auto & sample = HistNamePair.first;
    const auto & histname = HistNamePair.second;

    // loop over singals
    if (Common::GroupMap[sample] != SampleGroup::isSignal) continue;
    
    // get signal hist
    auto SignHist = (TH2F*)fInFile->Get(histname.Data());
    Common::CheckValidHist(SignHist,histname,fInFileName);
     
    // WS name
    const auto sample_wsname = fWSName+"_"+sample;

    // Make the WS
    CombinePreparer::MakeWS(sample,SignHist,sample_wsname);

    // Make the Datacard
    CombinePreparer::MakeDatacard(sample,SignHist,sample_wsname);

    // delete signal hist
    delete SignHist;
  }

  // save meta data
  if (fSaveMetaData) CombinePreparer::MakeConfigPave();
}

void CombinePreparer::MakeWS(const TString & sample, const TH2F * SignHist, const TString & sample_wsname)
{
  std::cout << "Making WS for: " << sample.Data() << std::endl;

  // make output
  fOutFile->cd();
  auto workspace = new RooWorkspace(sample_wsname.Data(),sample_wsname.Data());
  
  // bkg1
  const auto in_bkg1 = std::max(DataHist->GetBinContent(1,1)-SignHist->GetBinContent(1,1),0.0);
  RooRealVar bkg1(ABCD::bkg1name.Data(),ABCD::bkg1name.Data(),in_bkg1,0.0,10.0*in_bkg1); 
  workspace->import(bkg1);
  
  // loop over all possible ratios and save them
  for (const auto & RatioPair : ABCD::RatioMap)
  {
    const auto ratio = RatioPair.first;
    const auto bin   = RatioPair.second;

    // numerator bin (i)
    const auto binXY = ABCD::BinMap.at(bin);
    const auto ibinX = binXY.ibinX;
    const auto ibinY = binXY.ibinY;

    // denominator bin (j)
    const auto jbinX = ((bin%2==0)?ibinX-1:ibinX);
    const auto jbinY = ((bin%2==0)?ibinY:ibinY-1);

    const auto ibkgd = std::max(DataHist->GetBinContent(ibinX,ibinY)-SignHist->GetBinContent(ibinX,ibinY),0.0);
    const auto jbkgd = std::max(DataHist->GetBinContent(jbinX,jbinY)-SignHist->GetBinContent(jbinX,jbinY),0.0);

    // save ratio to workspace
    RooRealVar c(Form("%s%d",ABCD::ratiobase.Data(),ratio),Form("%s%d",ABCD::ratiobase.Data(),ratio),checkNaN(ibkgd/jbkgd),0.0,10.0);
    workspace->import(c);
  }

  // write ws
  fOutFile->cd();
  workspace->Write(workspace->GetName(),TObject::kWriteDelete);

  // delete workspace
  delete workspace;
}

void CombinePreparer::MakeDatacard(const TString & sample, const TH2F * SignHist, const TString & sample_wsname)
{
  std::cout << "Making datacard for: " << sample.Data() << std::endl;

  // make output datacard
  std::ofstream datacard(fDatacardName+"_"+sample+"."+ABCD::inTextExt,std::ios::trunc);
  
  // generic filler
  const TString filler = "----------------------------------------------------------------------------------------------------------------------------------";

  // do each section!
  CombinePreparer::FillTopSection(datacard);
  datacard << filler.Data() << std::endl;

  CombinePreparer::FillObservationSection(datacard);
  datacard << filler.Data() << std::endl;

  CombinePreparer::FillProcessSection(datacard,SignHist);
  datacard << filler.Data() << std::endl;

  CombinePreparer::FillSystematicsSection(datacard);
  datacard << filler.Data() << std::endl;

  CombinePreparer::FillRateParamSection(datacard,sample_wsname);
}

void CombinePreparer::FillTopSection(std::ofstream & datacard)
{
  // top section
  datacard << Form("imax %d number of bins",ABCD::nbinsXY) << std::endl;
  datacard << "jmax 1 number of processes minus 1" << std::endl;
  datacard << Form("kmax %lu number of nuisance parameters",fSystematics.size()) << std::endl;
}
  
void CombinePreparer::FillObservationSection(std::ofstream & datacard)
{
  // observation section : fill with data!
  datacard << "bin ";
  for (const auto & BinPair : ABCD::BinMap)
  {
    const auto bin = BinPair.first;
    datacard << Form("%s%d ",ABCD::binbase.Data(),bin);
  }
  datacard << std::endl;

  datacard << "observation ";
  for (const auto & BinPair : ABCD::BinMap)
  {
    const auto bin   = BinPair.first;
    const auto binXY = BinPair.second;
    const auto ibinX = binXY.ibinX;
    const auto ibinY = binXY.ibinY;

    if ((fBlindData && (ibinX != ABCD::nbinsX || ibinY != ABCD::nbinsY)) || (!fBlindData))
    {
      datacard << Form("%d ",Int_t(DataHist->GetBinContent(ibinX,ibinY)));
    }
    else
    {
      const auto pred = checkNaN(DataHist->GetBinContent(ibinX,1)*DataHist->GetBinContent(1,ibinY)/DataHist->GetBinContent(1,1));
      datacard << Form("%f ",pred);
    }
  }
  datacard << std::endl;
}

void CombinePreparer::FillProcessSection(std::ofstream & datacard, const TH2F * SignHist)
{
  // process section : fill with signal!
  datacard << "bin ";
  for (const auto & BinPair : ABCD::BinMap)
  {
    const auto bin = BinPair.first;
    datacard << Form("%s%d %s%d ",ABCD::binbase.Data(),bin,ABCD::binbase.Data(),bin);
  }
  datacard << std::endl;

  datacard << "process ";
  for (auto ibin = 1; ibin <= ABCD::nbinsXY; ibin++)
  {
    datacard << "sig bkg ";
  }
  datacard << std::endl;

  datacard << "process ";
  for (auto ibin = 1; ibin <= ABCD::nbinsXY; ibin++)
  {
    datacard << "0 1 ";
  }
  datacard << std::endl;

  datacard << "rate ";
  for (const auto & BinPair : ABCD::BinMap)
  {
    const auto bin   = BinPair.first;
    const auto binXY = BinPair.second;
    const auto ibinX = binXY.ibinX;
    const auto ibinY = binXY.ibinY;

    datacard << Form("%f 1 ",SignHist->GetBinContent(ibinX,ibinY));
  }
  datacard << std::endl;
}

void CombinePreparer::FillSystematicsSection(std::ofstream & datacard)
{
  // systematics section
  for (const auto & systematic : fSystematics)
  {
    const auto val   = systematic.val;
    const auto isSig = systematic.isSig;
    const auto isBkg = systematic.isBkg;
  
    datacard << Form("%s %s ",systematic.name.Data(),systematic.type.Data());
    for (auto ibin = 1; ibin <= ABCD::nbinsXY; ibin++)
    {
      datacard << Form("%s %s ",(isSig?Form("%f",val):"-"),(isBkg?Form("%f",val):"-"));
    }
    datacard << std::endl;
  }
}

void CombinePreparer::FillRateParamSection(std::ofstream & datacard, const TString & sample_wsname)
{
  // bkg1 first
  datacard << Form("%s rateParam %s1 bkg %s:%s",ABCD::bkg1name.Data(),ABCD::binbase.Data(),fWSFileName.Data(),sample_wsname.Data()) << std::endl;

  // then remainder of bins
  for (const auto & BinRatioVecPair : ABCD::BinRatioVecMap)
  {
    const auto bin = BinRatioVecPair.first;
    const auto RatioVec = BinRatioVecPair.second;

    datacard << Form("%s rateParam %s%d bkg %s:%s",ABCD::bkg1name.Data(),ABCD::binbase.Data(),bin,fWSFileName.Data(),sample_wsname.Data()) << std::endl;

    for (const auto ratio : RatioVec)
    {
      datacard << Form("%s%d rateParam %s%d bkg %s:%s",ABCD::ratiobase.Data(),ratio,ABCD::binbase.Data(),bin,fWSFileName.Data(),sample_wsname.Data()) << std::endl;
    }
  }
}

void CombinePreparer::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // give grand title
  fConfigPave->AddText("***** CombinePreparer Config *****");

  // dump ABCD info
  Common::AddTextFromInputConfig(fConfigPave,"BinInfo Config",fBinInfoName);
  Common::AddTextFromInputConfig(fConfigPave,"RatioInfo Config",fRatioInfoName);
  Common::AddTextFromInputConfig(fConfigPave,"BinRatioInfo Config",fBinRatioInfoName);

  // padding
  Common::AddPaddingToPave(fConfigPave,3);

  // dump systematics
  Common::AddTextFromInputConfig(fConfigPave,"Systematics Config",fSystFileName);

  // padding
  Common::AddPaddingToPave(fConfigPave,3);

  // dump combine info
  fConfigPave->AddText(Form("WS name: %s",fWSName.Data()));
  fConfigPave->AddText(Form("Datacard name: %s",fDatacardName.Data()));
  fConfigPave->AddText(Form("WS name: %s",fWSName.Data()));
  fConfigPave->AddText(Form("Blind data: %s",Common::PrintBool(fBlindData).Data()));

  // padding
  Common::AddPaddingToPave(fConfigPave,3);

  // save name of infile, redundant
  fConfigPave->AddText(Form("InFile name: %s",fInFileName.Data()));
  
  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void CombinePreparer::SetupCommon()
{
  std::cout << "Setup Common..." << std::endl;

  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupHistNames();
}

void CombinePreparer::SetupABCD()
{
  std::cout << "Setup ABCD..." << std::endl;

  ABCD::SetupBinMap(fBinInfoName);
  ABCD::SetupRatioMap(fRatioInfoName);
  ABCD::SetupBinRatioVecMap(fBinRatioInfoName);
}

void CombinePreparer::ReadSystematics()
{
  std::cout << "Reading Systematics..." << std::endl;  

  ifstream inconfig(fSystFileName.Data(),std::ios::in);
  TString name, type;
  Double_t val;
  Bool_t isSig, isBkg;

  while (inconfig >> name >> type >> val >> isSig >> isBkg)
  {
    fSystematics.emplace_back(name,type,val,isSig,isBkg);
  };
}
