#include "Common.cpp+"

void drawHist(TH1D * hist, const TString & label);
void drawHist(TH1D * hist, const TString & label, const Bool_t isLogy);

void makeTemplates(const TString & outfiletext)
{
  // setup
  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupHistNames();

  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);

  // inputs
  const TString filename = outfiletext+".root";
  auto file = TFile::Open(filename,"UPDATE");
  Common::CheckValidFile(file,filename);
  file->cd();

  const TString histname = Common::HistNameMap["Data"]+"_Plotted";
  auto hist = (TH2F*)file->Get(histname.Data());
  Common::CheckValidHist(hist,histname,filename);

  // time projection template
  auto hist_projX = hist->ProjectionX(Form("%s_Time_for_MET_0_to_200",hist->GetName()),
					   hist->GetYaxis()->FindBin(0.f),
					   hist->GetYaxis()->FindBin(200.f)-1);

  hist_projX->GetXaxis()->SetTitle(Form("%s {0 #leq p_{T}^{miss} < 200 [GeV]}",hist_projX->GetXaxis()->GetTitle()));
  hist_projX->GetYaxis()->SetTitle("Events");
  hist_projX->SetTitle(hist_projX->GetXaxis()->GetTitle());
  hist_projX->Scale(1.f/hist_projX->Integral());

  file->cd();
  hist_projX->Write(hist_projX->GetName(),TObject::kWriteDelete);

  drawHist(hist_projX,outfiletext+"_projX");

  // met projection template
  auto hist_projY = hist->ProjectionY(Form("%s_MET_for_Time_n2_to_2",hist->GetName()),
					   hist->GetXaxis()->FindBin(-2.f),
					   hist->GetXaxis()->FindBin(2.f)-1);

  hist_projY->GetXaxis()->SetTitle(Form("%s {-2 #leq Photon Weighted Cluster Time < 2 [ns]}",hist_projY->GetXaxis()->GetTitle()));
  hist_projY->GetYaxis()->SetTitle("Events");
  hist_projY->SetTitle(hist_projY->GetXaxis()->GetTitle());
  hist_projY->Scale(1.f/hist_projY->Integral());

  file->cd();
  hist_projY->Write(hist_projY->GetName(),TObject::kWriteDelete);

  drawHist(hist_projY,outfiletext+"_projY");

  // delete it all
  delete hist_projY;
  delete hist_projX;
  delete hist;
  delete file;
  delete tdrStyle;
}

void drawHist(TH1D * hist, const TString & label)
{
  drawHist(hist,label,false);
  drawHist(hist,label,true);
}

void drawHist(TH1D * hist, const TString & label, const Bool_t isLogy)
{
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy(isLogy);

  hist->Draw("ep");
  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,Form("%s_%s",label.Data(),(isLogy?"log":"lin")));

  delete canv;
}
