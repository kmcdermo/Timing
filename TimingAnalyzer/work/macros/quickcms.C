#include "quickcms.h"

void quickcms()
{
  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  TFile * file = TFile::Open("input/DATA/2016/doubleeg/skimmedtree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  TH1F * hist = new TH1F("effseedE_EB_sigman","",1000,0,1e4);
  hist->GetXaxis()->SetTitle("Effective E / #sigma_{n}");
  hist->GetYaxis()->SetTitle("Events");

  // Ecal ids, pedestals, and ADC conversion
  EcalIDMap      fEcalIDMap;
  IOVPairVec     fPedNoiseRuns;
  IDNoiseMapVec  fPedNoises;
  IOVPairVec     fADC2GeVRuns;
  ADC2GeVPairVec fADC2GeVs;
  
  if (Config::useSigma_n)
  {
    GetDetIDs(fEcalIDMap);
    GetPedestalNoise(fPedNoiseRuns,fPedNoises);
    GetADC2GeVConvs(fADC2GeVRuns,fADC2GeVs);
  }

  // Branches
  Int_t  run = 0;          TBranch * b_run;       tree->SetBranchAddress("run"      , &run      , &b_run);

  Float_t el1seedX  = 0.f; TBranch * b_el1seedX;  tree->SetBranchAddress("el1seedX" , &el1seedX , &b_el1seedX);
  Float_t el1seedY  = 0.f; TBranch * b_el1seedY;  tree->SetBranchAddress("el1seedY" , &el1seedY , &b_el1seedY);
  Float_t el1seedZ  = 0.f; TBranch * b_el1seedZ;  tree->SetBranchAddress("el1seedZ" , &el1seedZ , &b_el1seedZ);
  Float_t el1seedE  = 0.f; TBranch * b_el1seedE;  tree->SetBranchAddress("el1seedE" , &el1seedE , &b_el1seedE);
  Int_t   el1seedid = 0;   TBranch * b_el1seedid; tree->SetBranchAddress("el1seedid", &el1seedid, &b_el1seedid);

  Float_t el2seedX  = 0.f; TBranch * b_el2seedX;  tree->SetBranchAddress("el2seedX" , &el2seedX , &b_el2seedX);
  Float_t el2seedY  = 0.f; TBranch * b_el2seedY;  tree->SetBranchAddress("el2seedY" , &el2seedY , &b_el2seedY);
  Float_t el2seedZ  = 0.f; TBranch * b_el2seedZ;  tree->SetBranchAddress("el2seedZ" , &el2seedZ , &b_el2seedZ);
  Float_t el2seedE  = 0.f; TBranch * b_el2seedE;  tree->SetBranchAddress("el2seedE" , &el2seedE , &b_el2seedE);
  Int_t   el2seedid = 0;   TBranch * b_el2seedid; tree->SetBranchAddress("el2seedid", &el2seedid, &b_el2seedid);

  Int_t currentRun  = -1;
  Int_t PedNoiseIOV = -1;
  Int_t ADC2GeVIOV  = -1;
  for (UInt_t entry = 0; entry < tree->GetEntries(); entry++)
  {
    b_run->GetEvent(entry); b_el1seedid->GetEvent(entry); b_el2seedid->GetEvent(entry);
    b_el1seedX->GetEvent(entry); b_el1seedY->GetEvent(entry); b_el1seedZ->GetEvent(entry); b_el1seedE->GetEvent(entry); 
    b_el2seedX->GetEvent(entry); b_el2seedY->GetEvent(entry); b_el2seedZ->GetEvent(entry); b_el2seedE->GetEvent(entry);

    if (Config::useSigma_n) 
    {
      if (run != currentRun)
      {
	currentRun = run;
	for (UInt_t iov = 0; iov < fPedNoiseRuns.size(); iov++)
	{
	  if ((currentRun >= fPedNoiseRuns[iov].beg_) && (currentRun <= fPedNoiseRuns[iov].end_)) { PedNoiseIOV = iov; }
	}
	for (UInt_t iov = 0; iov < fADC2GeVRuns.size(); iov++)
	{
	  if ((currentRun >= fADC2GeVRuns[iov].beg_) && (currentRun <= fADC2GeVRuns[iov].end_)) { ADC2GeVIOV = iov; }
	}
      }
    }

    const Float_t el1seedeta = std::abs(eta(el1seedX,el1seedY,el1seedZ));
    const Float_t el2seedeta = std::abs(eta(el2seedX,el2seedY,el2seedZ));

    if (el1seedeta < Config::etaEB && el2seedeta < Config::etaEB) 
    {
      el1seedE /= (fPedNoises[PedNoiseIOV][el1seedid] * fADC2GeVs[ADC2GeVIOV].EB_);
      el2seedE /= (fPedNoises[PedNoiseIOV][el2seedid] * fADC2GeVs[ADC2GeVIOV].EB_);

      hist->Fill(effA(el1seedE,el2seedE));
    }
//     if ((el1seedeta > Config::etaEElow && el1seedeta < Config::etaEEhigh) && (el2seedeta > Config::etaEElow && el2seedeta < Config::etaEEhigh))
//     {
//       el1seedE /= (fPedNoises[PedNoiseIOV][el1seedid] * fADC2GeVs[ADC2GeVIOV].EE_);
//       el2seedE /= (fPedNoises[PedNoiseIOV][el2seedid] * fADC2GeVs[ADC2GeVIOV].EE_);

//       hist->Fill(effA(el1seedE,el2seedE));
//     }
  }

  TCanvas * canv = new TCanvas();
  canv->cd();
  canv->SetLogx(1);
  canv->SetLogy(1);

  hist->Draw();
  CMSLumi(canv,"Preliminary");

  canv->SaveAs(Form("%s.png",hist->GetName()));
}

void GetDetIDs(EcalIDMap & fEcalIDMap)
{
  // read in ix/iy, ieta/iphi for ecal detids
  std::ifstream inputids;
  inputids.open("config/detids.txt",std::ios::in);
  Int_t ID;
  Int_t i1, i2;
  TString name;
  while (inputids >> ID >> i1 >> i2 >> name)
  {
    fEcalIDMap[ID] = EcalID(i1,i2,name);
  }
  inputids.close();
}

void GetPedestalNoise(IOVPairVec & fPedNoiseRuns, IDNoiseMapVec & fPedNoises)
{
  // input runs
  std::ifstream pedruns;
  pedruns.open("config/pedestals/pedruns.txt",std::ios::in);
  Int_t t_ped_r1, t_ped_r2; // t is for temp
  while (pedruns >> t_ped_r1 >> t_ped_r2)
  {
    fPedNoiseRuns.push_back(IOVPair(t_ped_r1,t_ped_r2));
  }
  pedruns.close();
  
  // input ped to gev conversion factors
  fPedNoises.resize(fPedNoiseRuns.size()); // one map per IOV
  for (UInt_t iov = 0; iov < fPedNoiseRuns.size(); iov++)
  {
    std::ifstream inputpeds;
    inputpeds.open(Form("config/pedestals/pednoise_%i-%i.txt",fPedNoiseRuns[iov].beg_,fPedNoiseRuns[iov].end_),std::ios::in);
    IDNoiseMap t_noise_map;
    Int_t ID;
    Float_t noise;
    while (inputpeds >> ID >> noise) 
    {
      t_noise_map[ID] = noise;
    }
    fPedNoises[iov] = t_noise_map;
    inputpeds.close();
  }
}

void GetADC2GeVConvs(IOVPairVec & fADC2GeVRuns, ADC2GeVPairVec& fADC2GeVs)
{
  // input runs
  std::ifstream adcruns;
  adcruns.open("config/pedestals/adcruns.txt",std::ios::in);
  Int_t t_adc_r1, t_adc_r2; // t is for temp
  while (adcruns >> t_adc_r1 >> t_adc_r2)
  {
    fADC2GeVRuns.push_back(IOVPair(t_adc_r1,t_adc_r2));
  }
  adcruns.close();
  
  // input adc to gev conversion factors
  std::ifstream inputadcs; // only one file!
  inputadcs.open(Form("config/pedestals/adc2gev_%i-%i.txt",fADC2GeVRuns[0].beg_,fADC2GeVRuns[fADC2GeVRuns.size()-1].end_),std::ios::in);
  Float_t t_adc2gev_eb, t_adc2gev_ee;
  while (inputadcs >> t_adc_r1 >> t_adc_r2 >> t_adc2gev_eb >> t_adc2gev_ee) // one line per file, so can push directly back
  {
    fADC2GeVs.push_back(ADC2GeVPair(t_adc2gev_eb,t_adc2gev_ee)); 
  }
  inputadcs.close();
}
