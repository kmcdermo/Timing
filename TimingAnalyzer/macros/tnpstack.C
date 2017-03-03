#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"

#include <fstream>

void tnpstack()
{
  gStyle->SetOptStat(0);

  TFile * file = TFile::Open("skimmedtnptree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  TH1F * histVID0 = new TH1F("histVID0","Probe Electron OOT Flag [(zmass>76&&zmass<106) && (hltdoubleel37_27||hltdoubleel33_33)]",2,0,2);
  TH1F * histVID1 = new TH1F("histVID1","Probe Electron OOT Flag [(zmass>76&&zmass<106) && (hltdoubleel37_27||hltdoubleel33_33)]",2,0,2);
  TH1F * histVID2 = new TH1F("histVID2","Probe Electron OOT Flag [(zmass>76&&zmass<106) && (hltdoubleel37_27||hltdoubleel33_33)]",2,0,2);
  TH1F * histVID3 = new TH1F("histVID3","Probe Electron OOT Flag [(zmass>76&&zmass<106) && (hltdoubleel37_27||hltdoubleel33_33)]",2,0,2);
  TH1F * histVID4 = new TH1F("histVID4","Probe Electron OOT Flag [(zmass>76&&zmass<106) && (hltdoubleel37_27||hltdoubleel33_33)]",2,0,2);

  histVID0->Sumw2();
  histVID1->Sumw2();
  histVID2->Sumw2();
  histVID3->Sumw2();
  histVID4->Sumw2();

  tree->Draw("el2seedOOT>>histVID0","(zmass>76&&zmass<106)&&(hltdoubleel37_27||hltdoubleel33_33)&&(el2vid>=0)");
  tree->Draw("el2seedOOT>>histVID1","(zmass>76&&zmass<106)&&(hltdoubleel37_27||hltdoubleel33_33)&&(el2vid>=1)");
  tree->Draw("el2seedOOT>>histVID2","(zmass>76&&zmass<106)&&(hltdoubleel37_27||hltdoubleel33_33)&&(el2vid>=2)");
  tree->Draw("el2seedOOT>>histVID3","(zmass>76&&zmass<106)&&(hltdoubleel37_27||hltdoubleel33_33)&&(el2vid>=3)");
  tree->Draw("el2seedOOT>>histVID4","(zmass>76&&zmass<106)&&(hltdoubleel37_27||hltdoubleel33_33)&&(el2vid>=4)");

  std::ofstream unscaled;
  unscaled.open("unscaled.txt",std::ios_base::trunc);
  
  unscaled << "VID OOT==False OOT==True" << std::endl;
  unscaled << "all "    << std::setprecision(10) << histVID0->GetBinContent(1) << " " << histVID0->GetBinContent(2) << std::endl;
  unscaled << "veto "   << std::setprecision(10) << histVID1->GetBinContent(1) << " " << histVID1->GetBinContent(2) << std::endl;
  unscaled << "loose "  << std::setprecision(10) << histVID2->GetBinContent(1) << " " << histVID2->GetBinContent(2) << std::endl;
  unscaled << "medium " << std::setprecision(10) << histVID3->GetBinContent(1) << " " << histVID3->GetBinContent(2) << std::endl;
  unscaled << "tight "  << std::setprecision(10) << histVID4->GetBinContent(1) << " " << histVID4->GetBinContent(2) << std::endl;
  
  unscaled.close();

  histVID0->Scale(1.0/histVID0->Integral());
  histVID1->Scale(1.0/histVID1->Integral());
  histVID2->Scale(1.0/histVID2->Integral());
  histVID3->Scale(1.0/histVID3->Integral());
  histVID4->Scale(1.0/histVID4->Integral());

  histVID0->SetLineColor(kBlack);
  histVID1->SetLineColor(kGreen+1);
  histVID2->SetLineColor(kRed);
  histVID3->SetLineColor(kBlue);
  histVID4->SetLineColor(kViolet-1);

//   histVID0->SetLineWidth(5);
//   histVID1->SetLineWidth(4);
//   histVID2->SetLineWidth(3);
//   histVID3->SetLineWidth(2);
//   histVID4->SetLineWidth(1);

  histVID0->SetMarkerColor(kBlack);
  histVID1->SetMarkerColor(kGreen+1);
  histVID2->SetMarkerColor(kRed);
  histVID3->SetMarkerColor(kBlue);
  histVID4->SetMarkerColor(kViolet-1);

  histVID0->SetMarkerStyle(kFullCircle);
  histVID1->SetMarkerStyle(kFullSquare);
  histVID2->SetMarkerStyle(kFullTriangleUp);
  histVID3->SetMarkerStyle(kFullTriangleDown);
  histVID4->SetMarkerStyle(kFullDiamond);

  histVID0->SetMaximum(2.0);
  histVID0->SetMinimum(1e-6);

  TCanvas * canv = new TCanvas(); canv->cd(); canv->SetLogy();
  histVID0->Draw("hist e p");
  histVID1->Draw("same hist e p");
  histVID2->Draw("same hist e p");
  histVID3->Draw("same hist e p");
  histVID4->Draw("same hist e p");

  TLegend * leg = new TLegend(0.6,0.6,0.9,0.9);
  leg->AddEntry(histVID0,"All Probes","pl");
  leg->AddEntry(histVID1,"Veto Probes","pl");
  leg->AddEntry(histVID2,"Loose Probes","pl");
  leg->AddEntry(histVID3,"Medium Probes","pl");
  leg->AddEntry(histVID4,"Tight Probes","pl");
  leg->Draw("same");

  canv->SaveAs("tnp_seedOOT_stack.pdf");

  std::ofstream output;
  output.open("output.txt",std::ios_base::trunc);
  
  output << "VID OOT==False OOT==True" << std::endl;
  output << "all "    << std::setprecision(10) << histVID0->GetBinContent(1) << " " << histVID0->GetBinContent(2) << std::endl;
  output << "veto "   << std::setprecision(10) << histVID1->GetBinContent(1) << " " << histVID1->GetBinContent(2) << std::endl;
  output << "loose "  << std::setprecision(10) << histVID2->GetBinContent(1) << " " << histVID2->GetBinContent(2) << std::endl;
  output << "medium " << std::setprecision(10) << histVID3->GetBinContent(1) << " " << histVID3->GetBinContent(2) << std::endl;
  output << "tight "  << std::setprecision(10) << histVID4->GetBinContent(1) << " " << histVID4->GetBinContent(2) << std::endl;
  
  output.close();
}
