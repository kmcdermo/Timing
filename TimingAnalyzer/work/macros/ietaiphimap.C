#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TColor.h"

#include <iostream>
#include <vector>
#include <map>
#include <fstream>

static const Float_t shift = 0.00169548; //ns
static const Float_t N     = 38.1; // ns
static const Float_t C     = 0.2439; // ns
static const Float_t sn    = 0.0513; // GeV
static const Float_t sol   = 29.9792458; // cm / ns
static const Float_t rhEcut = 1.0; // rh Energy cut
static const Float_t dRcut  = 0.3; // isolation value -- deltaR cut on rh

inline Float_t rad2  (Float_t x, Float_t y){return x*x + y*y;}
inline Float_t TOF   (Float_t x, Float_t y, Float_t z, Float_t vx, Float_t vy, Float_t vz, Float_t time)
{
  return time + (std::sqrt(z*z+rad2(x,y))-std::sqrt((z-vz)*(z-vz)+rad2((x-vx),(y-vy))))/sol;
}

struct epid
{
public:
  epid() {}
  Int_t i1_; // iphi (EB) or ix (EE)
  Int_t i2_; // ieta (EB) or iy (EE)
};

typedef std::map<Int_t,epid> epidMap;

void getMap(epidMap & epids)
{
  //  initialize map of detector ids of iphi/ieta
  std::ifstream inputids;
  inputids.open("config/detids.txt",std::ios::in);
  Int_t ID;
  Int_t i1, i2;
  TString name;
  while (inputids >> ID >> i1 >> i2 >> name){
    epids[ID]       = epid();
    epids[ID].i1_   = i1;
    epids[ID].i2_   = i2;
  }
  inputids.close();
}

void ietaiphimap()
{
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

  // initialize ecal map
  epidMap epids;
  getMap(epids);

  // initialize tree and file + options
  TFile * file = TFile::Open("input/DATA/doubleeg/skimmedtree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  TString el = "el1";

  // initialize branches !!!!!!!!!!! ALWAYS SET VECTOR OF BRANCHES TO ZERO OR SEG FAULTS BECAUSE ROOT IS PIECE OF %^&*!
  std::vector<Float_t> * rhtimes = 0; tree->SetBranchAddress(Form("%srhtimes",el.Data()), &rhtimes);
  std::vector<Float_t> * rhEs = 0;    tree->SetBranchAddress(Form("%srhEs",el.Data()), &rhEs);
  std::vector<Int_t>   * rhids = 0;   tree->SetBranchAddress(Form("%srhids",el.Data()), &rhids);
  Int_t nrh;        tree->SetBranchAddress(Form("%snrh",el.Data()),&nrh);
  Float_t scE;      tree->SetBranchAddress(Form("%sscE",el.Data()), &scE);
  Float_t seedtime; tree->SetBranchAddress(Form("%sseedtime",el.Data()), &seedtime);
  Float_t seedE;    tree->SetBranchAddress(Form("%sseedE",el.Data()), &seedE);
  Float_t elE;      tree->SetBranchAddress(Form("%sE",el.Data()), &elE);
  Float_t elphi;    tree->SetBranchAddress(Form("%sphi",el.Data()), &elphi);
  Float_t eleta;    tree->SetBranchAddress(Form("%seta",el.Data()), &eleta);

  std::vector<Float_t> * rhXs = 0; tree->SetBranchAddress(Form("%srhXs",el.Data()), &rhXs);
  std::vector<Float_t> * rhYs = 0; tree->SetBranchAddress(Form("%srhYs",el.Data()), &rhYs);
  std::vector<Float_t> * rhZs = 0; tree->SetBranchAddress(Form("%srhZs",el.Data()), &rhZs);

  Float_t seedX; tree->SetBranchAddress(Form("%sseedX",el.Data()), &seedX);
  Float_t seedY; tree->SetBranchAddress(Form("%sseedY",el.Data()), &seedY);
  Float_t seedZ; tree->SetBranchAddress(Form("%sseedZ",el.Data()), &seedZ);

  Float_t vtxX; tree->SetBranchAddress("vtxX", &vtxX);
  Float_t vtxY; tree->SetBranchAddress("vtxY", &vtxY);
  Float_t vtxZ; tree->SetBranchAddress("vtxZ", &vtxZ);


  // output histograms

  // 601; //49, 601, 833, 1213, 1675, 1809, 1857, 1979, 2308 // entries for seedE != maxE in el1
  UInt_t entry = 2549; // 2579, 3480, 3492, 3955 --> 20 pruned rec hit events in el1
  tree->GetEntry(entry);

  std::cout << elE << " " << elphi << " " << eleta << std::endl;

  Int_t minphi = 1000, maxphi = -1000;
  Int_t mineta = 1000, maxeta = -1000;
  Float_t maxE = -1.f;

  std::ifstream inputids;
  inputids.open("config/el1_rhids_20.txt",std::ios::in);
  Int_t  id = -1;
  std::vector<Int_t> ids;
  while(inputids >> id){
    ids.push_back(id);
  }
  inputids.close();

  for (Int_t rh = 0; rh < nrh; rh++)
  {
    for (UInt_t rhid = 0; rhid < ids.size(); rhid++)
    {
      if (ids[rhid] != (*rhids)[rh]) continue;
      const Float_t tmpE = (*rhEs)[rh];    
      const Float_t tmptime = (*rhtimes)[rh];    
      
      const Int_t detid = (*rhids)[rh]; 
      const Int_t tmpphi = epids[detid].i1_; 
      const Int_t tmpeta = epids[detid].i2_;
      
      if (tmpphi > maxphi) maxphi = tmpphi;
      if (tmpphi < minphi) minphi = tmpphi;
      if (tmpeta > maxeta) maxeta = tmpeta;
      if (tmpeta < mineta) mineta = tmpeta;
            
      if (tmpE > maxE) maxE = tmpE;
    }
  }

  std::cout << "maxE: " << maxE << " seedE: " << seedE << std::endl;
  std::cout << "minphi: " << minphi << " maxphi: " << maxphi << std::endl;
  std::cout << "mineta: " << mineta << " maxeta: " << maxeta << std::endl;

  TH2F * h_map = new TH2F("h_map","RecHit Time in SC (ns)",maxeta-mineta+1,mineta,maxeta+1,maxphi-minphi+1,minphi,maxphi+1);
  h_map->SetXTitle("i_{#eta}"); h_map->SetYTitle("i_{#phi}"); //h_map->SetMinimum(0.0);
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

  std::ofstream output;
  output.open("scdump.txt",std::ios_base::app);
  output << "Event: " << entry << std::endl;
  output << "----------------" << std::endl;

  Float_t wgtT = 0.0f;
  Float_t sumS = 0.0f;
  Float_t mintime = 1000.f, maxtime = -1000.f;

  std::map<Int_t,Int_t> rhIds;
  for (Int_t rh = 0; rh < nrh; rh++)
  {
    for (UInt_t rhid = 0; rhid < ids.size(); rhid++)
    {
      if (ids[rhid] != (*rhids)[rh]) continue;

      if (rhIds.count((*rhids)[rh]) > 0) continue;
      rhIds[(*rhids)[rh]]++;
      
      const Float_t tmpE = (*rhEs)[rh];    
      const Float_t tmpT = TOF((*rhXs)[rh],(*rhYs)[rh],(*rhZs)[rh],vtxX,vtxY,vtxZ,(*rhtimes)[rh])-shift;
  
      const Int_t detid = (*rhids)[rh]; 
      const Int_t tmpphi = epids[detid].i1_; 
      const Int_t tmpeta = epids[detid].i2_;
      
      h_map->Fill(tmpeta,tmpphi,tmpT);
      
      const Float_t tmpS = (N / ( tmpE / sn)) + (std::sqrt(2.0f)*C);
      sumS += 1.0f / (tmpS*tmpS);
      wgtT += tmpT / (tmpS*tmpS);

      output << "Detid " << detid << " ieta " << tmpeta << " iphi " << tmpphi << " energy " << tmpE << " time " << (*rhtimes)[rh] << std::endl;
      
      if (tmpT > maxtime) maxtime = tmpT;
      if (tmpT < mintime) mintime = tmpT;
    }
  }

  std::cout << "mintime: " << mintime << " maxtime: " << maxtime << std::endl;

  seedtime = TOF(seedX,seedY,seedZ,vtxX,vtxY,vtxZ,seedtime)-shift;

  std::cout << seedtime << " " <<  wgtT / sumS << std::endl;

  for (Int_t xbin = 1; xbin <= h_map->GetXaxis()->GetNbins(); xbin++)
  {
    for (Int_t ybin = 1; ybin <= h_map->GetYaxis()->GetNbins(); ybin++)
    {
      Float_t content = h_map->GetBinContent(xbin,ybin);
      if (content == 0.f) h_map->SetBinContent(xbin,ybin,-1000.f);
    }
  }

  output << "----------------" << std::endl << std::endl;
  output.close();

  h_map->SetMinimum(mintime);
  h_map->SetMaximum(maxtime);
  
  h_map->Draw("colz");
  //h_map->Draw("box");
}
