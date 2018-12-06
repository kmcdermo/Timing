#include "Common.cpp+"

static const TString filename  = "skims/v1/orig_2phosCR/qcd.root";
static const TString selection = "";
static const TString label     = "QCDCR_Data_MET_v_Time_ZoomOut";

// static const TString filename  = "skims/v1/zee/skim.root";
// static const TString selection = "phoisEB_0&&phopt_0>40&&phoisEB_1&&phopt_1>40&&hltDiEle33MW";
// static const TString label     = "DYCR_Data_MET_v_Time";

static const TString outdir = "/eos/user/k/kmcdermo/www/dispho/plots/madv2_v1/test/adish_abcd";

void fast2D_with_correlation()
{
  // set style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);
  Common::SetupEras();

  // get inputs
  auto file = TFile::Open(filename.Data());
  Common::CheckValidFile(file,filename);
  
  const TString treename = "Data_Tree";
  auto tree = (TTree*)file->Get(treename.Data());
  Common::CheckValidTree(tree,treename,filename);

  // setup hist
  auto hist = new TH2F("hist","MET vs Time;Photon Seed Time [ns];MET [GeV]",100,-25,25,40,0,1000); 
  hist->Sumw2();

  // fill hist
  std::cout << "Filling hist..." << std::endl;
  tree->Draw("t1pfMETpt:phoseedtime_0>>hist",selection.Data(),"goff");

  // draw it
  std::cout << "Drawing and saving..." << std::endl;
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogz(1);

  hist->Draw("colz");

  // print correlation
  auto text = new TPaveText(0.68,0.7,0.81,0.93,"NDC");
  text->SetTextAlign(11);
  text->SetFillColorAlpha(text->GetFillColor(),0);
  text->AddText(Form("CF: %6.3f",hist->GetCorrelationFactor()));
  text->Draw("same");

  // save it
  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,label);

  // move it to www
  gSystem->Exec("mv "+label+".png "+outdir);
  gSystem->Exec("mv "+label+".pdf "+outdir);

  // delete it all
  std::cout << "Deleting..." << std::endl;
  delete text;
  delete canv;
  delete hist;
  delete tree;
  delete file;
  delete tdrStyle;
}
