#include "CombinePreparer.hh"

CombinePreparer::CombinePreparer(const TString & infilename, const TString & bininfoname,
				 const TString & ratioinfoname, const TString & binratioinfoname,
				 const TString & systfilename, const TString & datacardname, 
				 const Bool_t blinddata)
  : fInFileName(infilename), fBinInfoName(bininfoname),
    fRatioInfoName(ratioinfoname), fBinRatioInfoName(binratioinfoname),
    fSystFileName(systfilename), fDatacardName(datacardname),
    fBlindData(blinddata)
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
  
  BkgdHist = (TH2F*)fInFile->Get(Common::BkgdHistName.Data());
  Common::CheckValidHist(BkgdHist,Common::BkgdHistName,fInFileName);

  // Get template file
  const auto inTemplateFileName = Common::eosPreFix+"/"+Common::eosDir+"/"+Common::calibDir+"/"+Common::templateFileName;
  fTemplateFile = TFile::Open(inTemplateFileName.Data());
  Common::CheckValidFile(fTemplateFile,inTemplateFileName);
  fTemplateFile->cd();

  // Get template hists
  TemplateHistX = (TH1D*)fTemplateFile->Get(Common::templateXHistName.Data());
  Common::CheckValidHist(TemplateHistX,Common::templateXHistName,inTemplateFileName);

  TemplateHistY = (TH1D*)fTemplateFile->Get(Common::templateYHistName.Data());
  Common::CheckValidHist(TemplateHistY,Common::templateYHistName,inTemplateFileName);

  // Setup binning scheme
  ABCD::SetupNBins(DataHist);
}

CombinePreparer::~CombinePreparer()
{
  std::cout << "Tidying up in the destructor..." << std::endl;

  delete TemplateHistY;
  delete TemplateHistX;
  delete fTemplateFile;

  delete BkgdHist;
  delete DataHist;
  delete fInFile;
}

void CombinePreparer::PrepareCombine()
{
  std::cout << "Preparing inputs to combine" << std::endl;

  // make norm + ratios once
  CombinePreparer::MakeParameters();

  // loop over all signals and make datacard for each signal
  for (const auto & HistNamePair : Common::HistNameMap)
  {
    const auto & sample = HistNamePair.first;
    const auto & histname = HistNamePair.second;

    // loop over singals
    if (Common::GroupMap[sample] != SampleGroup::isSignal) continue;
    
    // get signal hist
    auto SignHist = (TH2F*)fInFile->Get(histname.Data());
    Common::CheckValidHist(SignHist,histname,fInFileName);

    // Make the Datacard
    CombinePreparer::MakeDatacard(sample,SignHist);

    // delete signal hist
    delete SignHist;
  }
}

void CombinePreparer::MakeParameters()
{
  std::cout << "Making parameters for background..." << std::endl;

  // bkg1 --> estimate from BbkgdHist * k-Factor
  fParameterMap[ABCD::bkg1name] = std::max(BkgdHist->GetBinContent(1,1) * (DataHist->Integral() / BkgdHist->Integral()),0.0);
  
  // loop over all possible ratios and put in parameter map
  for (const auto & RatioPair : ABCD::RatioMap)
  {
    const auto ratio = RatioPair.first;
    const auto bin   = RatioPair.second;

    // X or Y axis ratio?
    const auto isX = (ratio%2==0);

    // bin split in ABCD plot
    const auto binXY = ABCD::BinMap.at(bin);
    const auto ibin  = (isX ? binXY.ibinX : binXY.ibinY);

    // which template to use
    const auto & TemplateHist = (isX ? TemplateHistX : TemplateHistY);

    // which axis to use
    const auto & DataAxis = (isX ? DataHist->GetXaxis() : DataHist->GetYaxis());

    // template bin boundaries
    const auto ibinLow = TemplateHist->FindBin(DataAxis->GetBinLowEdge(ibin-1));
    const auto ibinMid = TemplateHist->FindBin(DataAxis->GetBinLowEdge(ibin));
    const auto ibinUp  = TemplateHist->FindBin(DataAxis->GetBinUpEdge (ibin));

    // get ratio
    const TString rationame = Form("%s%d",ABCD::ratiobase.Data(),ratio);
    fParameterMap[rationame] = std::max(TemplateHist->Integral(ibinMid,ibinUp) / TemplateHist->Integral(ibinLow,ibinMid-1),0.0);
  }
}

void CombinePreparer::MakeDatacard(const TString & sample, const TH2F * SignHist)
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

  CombinePreparer::FillRateParamSection(datacard);
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
      // predict using surrounding bins the top right corner
      const auto pred = checkNaN(DataHist->GetBinContent(ibinX-1,ibinY)*DataHist->GetBinContent(ibinX,ibinY-1)/DataHist->GetBinContent(ibinX-1,ibinY-1));
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

void CombinePreparer::FillRateParamSection(std::ofstream & datacard)
{
  // bkg1 first for bin1
  datacard << Form("%s rateParam %s1 bkg %f",ABCD::bkg1name.Data(),ABCD::binbase.Data(),fParameterMap[ABCD::bkg1name]) << std::endl;

  // then remainder of bins
  for (const auto & BinRatioVecPair : ABCD::BinRatioVecMap)
  {
    const auto bin = BinRatioVecPair.first;
    const auto RatioVec = BinRatioVecPair.second;

    // add bkg1 first
    datacard << Form("%s rateParam %s%d bkg %f",ABCD::bkg1name.Data(),ABCD::binbase.Data(),bin,fParameterMap[ABCD::bkg1name]) << std::endl;

    for (const auto ratio : RatioVec)
    {
      const TString rationame = Form("%s%d",ABCD::ratiobase.Data(),ratio);
      datacard << Form("%s rateParam %s%d bkg %f",rationame.Data(),ABCD::binbase.Data(),bin,fParameterMap[rationame]) << std::endl;
    }
  }
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

  std::ifstream inconfig(fSystFileName.Data(),std::ios::in);
  TString name, type;
  Double_t val;
  Bool_t isSig, isBkg;

  while (inconfig >> name >> type >> val >> isSig >> isBkg)
  {
    fSystematics.emplace_back(name,type,val,isSig,isBkg);
  };
}
