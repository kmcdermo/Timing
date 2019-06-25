#include "Common.cpp+"

const auto sf_lin = 1.05;
const auto sf_log = 2.0;

void SetupInputs(TFile * file, TH1F *& data, TH1F *& sign, const TString & filename_base, const TString & signal, const Color_t color, const Style_t style, const TString & label);
void DrawCanvas(const Bool_t isLogY, const TString & label, const TString & finalname, const Double_t min, const Double_t max, 
		TCanvas *& canv, TLegend *& leg, TH1F *& data_L, TH1F *& data_R, TH1F *& sign_L, TH1F *& sign_R);
void MakeTextFile(const TString & label, const TString & finalname, const TH1F * data_L, const TH1F * data_R, const TH1F * sign_R);
void FillTextFile(std::ofstream & output, const TString & label, const TH1F * hist);
void MakeLegend(TLegend *& leg, const TString & label_L, const TString & label_R, const TString & signal_label, const TString & extra_L,
		const TH1F * data_L, const TH1F * data_R, const TH1F * sign_L, const TH1F * sign_R);
void GetMinMax(const TH1F * data_L, const TH1F * data_R, const TH1F * sign_L, const TH1F * sign_R, Double_t & min, Double_t & max);
void GetMinMax(const TH1F * hist, Double_t & min, Double_t & max);
void SetMinMax(TH1F *& hist, const Double_t min, const Double_t max, const Double_t sf);

void drawDataVData(const TString & leftname, const TString & rightname, 
		   const TString & cut_label, const TString & split, const TString cut_unit,
		   const TString & signal, const TString & finalname)
{
  // make config
  auto tdrStyle = new TStyle();
  Common::SetTDRStyle(tdrStyle);
  Common::SetupEras();

  // make output
  auto outfile = TFile::Open(finalname+".root","RECREATE");

  // Get inputs
  TFile * file_L = 0; TH1F * data_L = 0, * sign_L = 0;
  SetupInputs(file_L,data_L,sign_L,leftname,signal,kRed+1,20,"L");

  TFile * file_R = 0; TH1F * data_R = 0, * sign_R = 0;
  SetupInputs(file_R,data_R,sign_R,rightname,signal,kBlue,21,"R");

  // make ratio
  auto err_L = 0.0, err_R = 0.0;
  const auto int_L = data_L->IntegralAndError(1,data_L->GetXaxis()->GetNbins(),err_L,"width");
  const auto int_R = data_R->IntegralAndError(1,data_R->GetXaxis()->GetNbins(),err_R,"width");
  const auto ratio = int_R / int_L;
  const auto error = ratio * std::sqrt(std::pow(err_L/int_L,2) + std::pow(err_R/int_R,2));

  // make pave
  auto pave = new TPaveText();
  pave->SetName("PaveText");
  pave->AddText(Form("Ratio R/L : %f +/- %f",ratio,error));

  // get signal params
  const TString s_ctau = "_CTau";
  auto i_ctau = signal.Index(s_ctau);
  auto l_ctau = s_ctau.Length();
  const TString ctau(signal(i_ctau+l_ctau,signal.Length()-i_ctau-l_ctau));

  const TString s_lambda = "_Lambda";
  auto i_lambda = signal.Index(s_lambda);
  auto l_lambda = s_lambda.Length();
  const TString lambda(signal(i_lambda+l_lambda,i_ctau-i_lambda-l_lambda));
  
  // make labels
  const auto label_L = "["+cut_label+" < "+split+" "+cut_unit+"]";
  const auto label_R = "["+cut_label+" #geq "+split+" "+cut_unit+"]";
  const auto signal_label = "GMSB #Lambda: "+ctau+" TeV, c#tau: "+ctau+" cm";

  // write to outfile
  Common::Write(outfile,data_L);
  Common::Write(outfile,data_R);
  Common::Write(outfile,sign_L);
  Common::Write(outfile,sign_R);
  Common::Write(outfile,pave);

  // make canvas
  auto canv = new TCanvas("Canvas","");
  canv->cd();
  
  // draw absolute normalization
  TString label = "absnorm";
  
  TLegend * leg = 0;
  MakeLegend(leg,label_L,label_R,signal_label,"",data_L,data_R,sign_L,sign_R);
    
  auto min = 1e9, max = -1e9;
  GetMinMax(data_L,data_R,sign_L,sign_R,min,max);

  DrawCanvas(true ,label,finalname,min,max,canv,leg,data_L,data_R,sign_L,sign_R);
  DrawCanvas(false,label,finalname,min,max,canv,leg,data_L,data_R,sign_L,sign_R);
  Common::Write(outfile,leg ,Form("%s_%s",leg ->GetName(),label.Data()));
  Common::Write(outfile,canv,Form("%s_%s",canv->GetName(),label.Data()));

  MakeTextFile(label,finalname,data_L,data_R,sign_R);

  // draw scaled data
  data_L->Scale(ratio);

  label = "scaled";

  delete leg;
  MakeLegend(leg,label_L,label_R,signal_label,Form(" (Scaled x %5.3f)",ratio),data_L,data_R,sign_L,sign_R);

  min = 1e9, max = -1e9;
  GetMinMax(data_L,data_R,sign_L,sign_R,min,max);

  DrawCanvas(true ,label,finalname,min,max,canv,leg,data_L,data_R,sign_L,sign_R);
  DrawCanvas(false,label,finalname,min,max,canv,leg,data_L,data_R,sign_L,sign_R);
  Common::Write(outfile,leg ,Form("%s_%s",leg ->GetName(),label.Data()));
  Common::Write(outfile,canv,Form("%s_%s",canv->GetName(),label.Data()));

  MakeTextFile(label,finalname,data_L,data_R,sign_R);

  // delete it all
  delete leg;
  delete canv;
  delete pave;
  delete sign_R;
  delete data_R;
  delete file_R;
  delete sign_L;
  delete data_L;
  delete file_L;
  delete outfile;
  delete tdrStyle;
}

void SetupInputs(TFile * file, TH1F *& data, TH1F *& sign, const TString & filename_base, const TString & signal, const Color_t color, const Style_t style, const TString & label)
{
  // get file
  const auto filename = filename_base+".root";
  file = TFile::Open(filename.Data());
  Common::CheckValidFile(file,filename);

  // get hists
  const TString dataname = "Data_Hist";
  data = (TH1F*)file->Get(dataname.Data());
  Common::CheckValidHist(data,dataname,filename);

  const auto signname = signal+"_Hist";
  sign = (TH1F*)file->Get(signname.Data());
  Common::CheckValidHist(sign,signname,filename);

  // set names
  data->SetName(dataname+"_"+label);
  sign->SetName(signname+"_"+label);

  // set styles
  data->SetLineColor  (color);
  data->SetMarkerColor(color);
  data->SetMarkerStyle(style);

  sign->SetLineColor  (kBlack);
  sign->SetMarkerColor(kBlack);
  sign->SetFillColor  (kBlack);
  sign->SetFillStyle  (0);
  sign->SetLineStyle(7);
  sign->SetLineWidth(2);
}

void DrawCanvas(const Bool_t isLogY, const TString & label, const TString & finalname, const Double_t min, const Double_t max, 
		TCanvas *& canv, TLegend *& leg, TH1F *& data_L, TH1F *& data_R, TH1F *& sign_L, TH1F *& sign_R)
{
  // set logy
  canv->SetLogy(isLogY);
  const TString canv_scale = (isLogY ? "log" : "lin");

  // set hist min/max
  const auto sf = (isLogY ? sf_log : sf_lin);
  SetMinMax(data_L,min,max,sf);
  SetMinMax(data_R,min,max,sf);
  //  SetMinMax(sign_L,min,max,sf);
  SetMinMax(sign_R,min,max,sf);
  
  // draw it all
  canv->cd();
  data_L->Draw("ep");
  data_R->Draw("ep same");
  //  sign_L->Draw("hist same");
  sign_R->Draw("hist same");
  leg->Draw("same");

  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,finalname+"_"+label+"_"+canv_scale);
  canv->SaveAs(finalname+"_"+label+"_"+canv_scale+".C");
}

void MakeTextFile(const TString & label, const TString & finalname, const TH1F * data_L, const TH1F * data_R, const TH1F * sign_R)
{
  std::ofstream output(finalname+"_"+label+"."+Common::outTextExt,std::ios::trunc);
  
  FillTextFile(output,"Data_Hist_L",data_L);
  FillTextFile(output,"Data_Hist_R",data_R);
  FillTextFile(output,"GMSB_Hist"  ,sign_R);
}

void FillTextFile(std::ofstream & output, const TString & label, const TH1F * hist)
{
  for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
  {
    output << label.Data() << " Content " << ibinX << " " << hist->GetBinContent(ibinX) << std::endl;
    output << label.Data() << " Error "   << ibinX << " " << hist->GetBinError  (ibinX) << std::endl;
  }
  output << label.Data() << " Entries " << hist->GetEntries() << std::endl;
}

void MakeLegend(TLegend *& leg, const TString & label_L, const TString & label_R, const TString & signal_label, const TString & extra_L,
		const TH1F * data_L, const TH1F * data_R, const TH1F * sign_L, const TH1F * sign_R)
{
  //  leg = new TLegend(0.30,0.7,0.82,0.92);
  leg = new TLegend(0.30,0.755,0.82,0.92);
  leg->SetName("Legend");
  leg->SetBorderSize(0);
  //  leg->SetLineColor(kBlack);
  leg->SetTextFont(42);
  leg->AddEntry(data_L,"Data "+label_L+extra_L,"epl");
  leg->AddEntry(data_R,"Data "+label_R,"epl");
  //  leg->AddEntry(sign_L,signal_label+" "+label_L,"l");
  leg->AddEntry(sign_R,signal_label+" "+label_R,"l");
}

void GetMinMax(const TH1F * data_L, const TH1F * data_R, const TH1F * sign_L, const TH1F * sign_R, Double_t & min, Double_t & max)
{
  // get min, max
  GetMinMax(data_L,min,max);
  GetMinMax(data_R,min,max);
  //  GetMinMax(sign_L,min,max);
  GetMinMax(sign_R,min,max);
}

void GetMinMax(const TH1F * hist, Double_t & min, Double_t & max)
{
  for (auto ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
  {
    const auto content = hist->GetBinContent(ibin);
    
    if (content < min && content > 0.0) min = content;
    if (content > max && content > 0.0) max = content;
  }
}

void SetMinMax(TH1F *& hist, const Double_t min, const Double_t max, const Double_t sf)
{
  hist->GetYaxis()->SetRangeUser(min/sf,max*sf);
}
