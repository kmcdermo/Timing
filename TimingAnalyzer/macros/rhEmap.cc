#include "rhEmap.hh"

#include "TStyle.h"
#include "TColor.h"
#include "TROOT.h"

#include "TH2F.h"
#include "TCanvas.h"

#include <fstream>
#include <iostream>

rhEmap::rhEmap()
{
  fInFile = TFile::Open("input/MC/signal/withReReco/ctau6000.root");
  fInTree = (TTree*)fInFile->Get("tree/tree");

  rhEmap::InitTree();
  rhEmap::MakeIetaIphiMap();

  // make the vid maps!
  fPhVIDMap["loose"]  = 1;
  fPhVIDMap["medium"] = 2;
  fPhVIDMap["tight"]  = 3;
  fPhVID = "medium";

  /// initialize 2d colz
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  // for a nice color palette
  const Int_t NRGBs = 5;
  const Int_t NCont = 255;
  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  tdrStyle->SetNumberContours(NCont);
  tdrStyle->SetOptStat(0);
  tdrStyle->cd();
  gROOT->ForceStyle();
}

rhEmap::~rhEmap()
{
  delete fInTree;
  delete fInFile;
}

void rhEmap::InitTree()
{
  nphotons  = 0;
  phnrh     = 0;
  phE       = 0;
  phpt      = 0;
  pheta     = 0;
  phsmaj    = 0;
  phsmin    = 0;
  phVID     = 0;
  phseedpos = 0;
  phrhID    = 0;
  phrhE     = 0;
  phrhX     = 0;
  phrhY     = 0;
  phrhZ     = 0;
  phrhtime  = 0;

  fInTree->SetBranchAddress("nphotons" , &nphotons , &b_nphotons);
  fInTree->SetBranchAddress("phnrh"    , &phnrh    , &b_phnrh);
  fInTree->SetBranchAddress("phE"      , &phE      , &b_phE);
  fInTree->SetBranchAddress("phpt"     , &phpt     , &b_phpt);
  fInTree->SetBranchAddress("pheta"    , &pheta    , &b_pheta);
  fInTree->SetBranchAddress("phsmaj"   , &phsmaj   , &b_phsmaj);
  fInTree->SetBranchAddress("phsmin"   , &phsmin   , &b_phsmin);
  fInTree->SetBranchAddress("phVID"    , &phVID    , &b_phVID);
  fInTree->SetBranchAddress("phseedpos", &phseedpos, &b_phseedpos);
  fInTree->SetBranchAddress("phrhID"   , &phrhID   , &b_phrhID);
  fInTree->SetBranchAddress("phrhE"    , &phrhE    , &b_phrhE);
  fInTree->SetBranchAddress("phrhX"    , &phrhX    , &b_phrhX);
  fInTree->SetBranchAddress("phrhY"    , &phrhY    , &b_phrhY);
  fInTree->SetBranchAddress("phrhZ"    , &phrhZ    , &b_phrhZ);
  fInTree->SetBranchAddress("phrhtime" , &phrhtime , &b_phrhtime);
}

void rhEmap::MakeIetaIphiMap()
{
  //  initialize map of detector ids of iphi/ieta
  std::ifstream inputids;
  inputids.open("../work/config/detids.txt",std::ios::in);
  Int_t ID;
  Int_t i1, i2;
  TString name;
  while (inputids >> ID >> i1 >> i2 >> name)
  {
    epids[ID]     = epid();
    epids[ID].i1_ = i1;
    epids[ID].i2_ = i2;
  }
  inputids.close();
}

void rhEmap::CheckForGoodPhotons()
{
  std::ofstream goodphos;
  goodphos.open("prompt.txt",std::ios_base::trunc);
  for (UInt_t ientry = 0; ientry < fInTree->GetEntries(); ientry++)
  {  
    fInTree->GetEntry(ientry);
    for (Int_t iph = 0; iph < nphotons; iph++)
    {
      if ((*phpt)[iph] < 100.f) continue;
      if (fPhVIDMap[fPhVID] < (*phVID)[iph]) continue;
      if (((std::abs((*pheta)[iph]) > 2.5) || (std::abs((*pheta)[iph]) > 1.4442 && std::abs((*pheta)[iph]) < 1.566))) continue;
      if ((*phseedpos)[iph] == -9999) continue;

      const Float_t time = (*phrhtime)[iph][(*phseedpos)[iph]];
      const Float_t ph_E = (*phE)[iph];
      const Float_t smaj = 1.f/std::sqrt((*phsmin)[iph]);
      const Float_t smin = 1.f/std::sqrt((*phsmaj)[iph]);
      
      if (std::abs(time) < 0.001) 
      { 
	std::cout << "Entry: " << ientry << " iph: " << iph << " time: " << time << " phE: " << ph_E << " smaj: " << smaj << " smin: " << smin << std::endl;
	goodphos << ientry << " " << iph << " " << time << " " << ph_E << " " << smaj << " " << smin << std::endl;
      }
    }
  }
  goodphos.close();
}

void rhEmap::DumpGoodPhotonRHIDs()
{
  fInTree->GetEntry(17664); 
  Int_t iph = 0;
  for (Int_t irh = 0; irh < (*phnrh)[iph]; irh++)
  {
    if ((*phrhE)[iph][irh] < 1.f) continue;
    Int_t rhID = (*phrhID)[iph][irh];
    std::cout << rhID << std::endl;
  }
}

void rhEmap::DoPlotIndices()
{
  Int_t minphi = 1000, maxphi = -1000;
  Int_t mineta = 1000, maxeta = -1000;
  Float_t maxE = -1.f;

  std::ifstream inputids;
  inputids.open("config/detids-prompt2.txt",std::ios::in);
  Int_t id = -1;
  std::vector<Int_t> ids;
  while(inputids >> id){
    ids.push_back(id);
  }
  inputids.close();

  fInTree->GetEntry(17664); // 6494 --> prompt1, 17664 --> prompt2, 31665 --> delayed1
  Int_t iph = 0;

  for (Int_t irh = 0; irh < (*phnrh)[iph]; irh++)
  {
    for (UInt_t rhid = 0; rhid < ids.size(); rhid++)
    {
      if (ids[rhid] != (*phrhID)[iph][irh]) continue;
      const Float_t tmpE    = (*phrhE)[iph][irh];    
      const Float_t tmptime = (*phrhtime)[iph][irh];    
      
      const Int_t detid  = ids[rhid]; 
      const Int_t tmpphi = epids[detid].i1_; 
      const Int_t tmpeta = epids[detid].i2_;
      
      if (tmpphi > maxphi) maxphi = tmpphi;
      if (tmpphi < minphi) minphi = tmpphi;
      if (tmpeta > maxeta) maxeta = tmpeta;
      if (tmpeta < mineta) mineta = tmpeta;
            
      if (tmpE > maxE) maxE = tmpE;
    }
  }

  std::cout << "maxE: " << maxE << " seedE: " << (*phrhE)[iph][(*phseedpos)[iph]] << std::endl;
  std::cout << "minphi: " << minphi << " maxphi: " << maxphi << std::endl;
  std::cout << "mineta: " << mineta << " maxeta: " << maxeta << std::endl;

  TH2F * h_map = new TH2F("h_map","RecHit Energy in SC (GeV)",maxeta-mineta+1,mineta,maxeta+1,maxphi-minphi+1,minphi,maxphi+1);
  h_map->SetXTitle("i_{#eta}"); h_map->SetYTitle("i_{#phi}"); h_map->SetMinimum(0.0);
  for (Int_t xbin = 1; xbin <= h_map->GetXaxis()->GetNbins(); xbin++)
  {
    Int_t label = h_map->GetXaxis()->GetBinLowEdge(xbin);
    h_map->GetXaxis()->SetBinLabel(xbin,Form("%i",label));
  }
  for (Int_t ybin = 1; ybin <= h_map->GetYaxis()->GetNbins(); ybin++)
  {
    Int_t label = h_map->GetYaxis()->GetBinLowEdge(ybin);
    h_map->GetYaxis()->SetBinLabel(ybin,Form("%i",label));
  }

  for (Int_t irh = 0; irh < (*phnrh)[iph]; irh++)
  {
    for (UInt_t rhid = 0; rhid < ids.size(); rhid++)
    {
      if (ids[rhid] != (*phrhID)[iph][irh]) continue;
      
      const Float_t tmpE = (*phrhE)[iph][irh];    
  
      const Int_t detid  = ids[rhid];
      const Int_t tmpphi = epids[detid].i1_; 
      const Int_t tmpeta = epids[detid].i2_;

      //      std::cout << tmpeta << " " << tmpphi << " " << tmpE << std::endl;
      
      h_map->Fill(tmpeta,tmpphi,tmpE);
    }
  }

  TCanvas * canv = new TCanvas();
  canv->cd();
  h_map->Draw("box"); //h_map->Draw("colz");
  canv->SaveAs(Form("%s-prompt2.png",h_map->GetName()));
  //  delete canv;
  //  delete h_map;
}

void rhEmap::DoPlotNatural()
{
  Float_t midphi = 0.f, mideta = 0.f;
  Float_t maxE   = -1.f;

  fInTree->GetEntry(31665); // 6494 --> prompt1, 17664 --> prompt2, 31665 --> delayed1
  Int_t iph = 0;

  for (Int_t irh = 0; irh < (*phnrh)[iph]; irh++)
  {
    const Float_t tmpE = (*phrhE)[iph][irh];    
    if (tmpE > maxE)
    { 
      maxE   = tmpE;
      midphi = phi((*phrhX)[iph][irh],(*phrhY)[iph][irh]);
      mideta = eta((*phrhX)[iph][irh],(*phrhY)[iph][irh],(*phrhZ)[iph][irh]);
    }
  }

  std::cout << "maxE: "   << maxE   << " seedE: "  << (*phrhE)[iph][(*phseedpos)[iph]] << std::endl;
  std::cout << "midphi: " << midphi << " mideta: " << mideta << std::endl;
  
  TH2F * h_map = new TH2F("rhE_natural_delay1","RecHit Energy in SC (GeV)",20,mideta-0.1,mideta+0.1,20,midphi-0.1,midphi+0.1);
  h_map->SetXTitle("#eta"); h_map->SetYTitle("#phi"); h_map->SetMinimum(0.0);

  for (Int_t irh = 0; irh < (*phnrh)[iph]; irh++)
  {
    const Float_t tmpE   = (*phrhE)[iph][irh];    
    //    if (tmpE < 1.f) continue;
    const Float_t tmpphi = phi((*phrhX)[iph][irh],(*phrhY)[iph][irh]);
    const Float_t tmpeta = eta((*phrhX)[iph][irh],(*phrhY)[iph][irh],(*phrhZ)[iph][irh]);
  
    h_map->Fill(tmpeta,tmpphi,tmpE);
  }

  TCanvas * canv = new TCanvas("canv","canv",600,600);
  canv->cd();
  canv->SetGridx(1);
  canv->SetGridy(1);
  h_map->Draw("box"); //h_map->Draw("colz");
  canv->SaveAs(Form("%s.png",h_map->GetName()));
  //  delete canv;
  //  delete h_map;
}







