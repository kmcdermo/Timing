#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "THStack.h"
#include "TCanvas.h"
#include "TLegend.h"

#include "common/common.C"

#include <vector>
#include <map>
#include <utility>

void multicanv2D_njet()
{
  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  std::vector<TString> histnames = {"nJetsPt5_vs_jetHT","nJetsPt10_vs_jetHT","nJetsPt15_vs_jetHT","nJetsPt20_vs_jetHT","nJetsPt30_vs_jetHT",
				    "nJetsPt40_vs_jetHT","nJetsPt50_vs_jetHT","nJetsPt75_vs_jetHT","nJetsPt100_vs_jetHT"};

  std::vector<TString> dirs = {"output/MC/signal/GMSB/photondump/ctau100","output/MC/signal/GMSB/photondump/ctau2000","output/MC/signal/GMSB/photondump/ctau6000",
			       "output/MC/signal/HVDS/photondump/ctau100","output/MC/signal/HVDS/photondump/ctau1000"};

  for (UInt_t ihistname = 0; ihistname < histnames.size(); ihistname++)
  {
    // declare canvas
    TCanvas * canv = new TCanvas("canv","canv",2000,1300); 
    canv->Divide(3,2,0.0001,0.001);  
	
    for (UInt_t idir = 0; idir < dirs.size(); idir++)
    {
      TFile * file = TFile::Open(Form("%s/cuts_phanypt10.0_rhE1.0_ecalaccept/plots.root",dirs[idir].Data()));
	
      // cd to the right pad
      canv->cd(idir+1);

      // get and draw the histogram in question
      TH1F * hist = (TH1F*)file->Get(histnames[ihistname].Data());
      hist->Scale(1.f/hist->Integral());
      hist->Draw("colz");
    } // end loop over dirs

    // save and delete the canvas
    canv->SaveAs(Form("output/MC/signal/%s.png",histnames[ihistname].Data()));
    delete canv;

  } // end loop over histogram names
}
