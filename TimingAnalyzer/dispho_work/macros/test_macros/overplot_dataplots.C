#include "Common.cpp+"

#include "TStyle.h"
#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h"
#include "TColor.h"

#include <vector>
#include <string>
#include <fstream>

struct InputStruct
{
  InputStruct() {}
  InputStruct(const TString & label, const TString & filename, const Color_t color, const Bool_t VarBins)
    : label(label) 
  {
    file = TFile::Open(filename.Data());
    Common::CheckValidFile(file,filename);

    const TString histname = "Data_Hist_Plotted";
    hist = (TH1F*)file->Get(histname.Data());
    Common::CheckValidHist(hist,histname,filename);

    hist->SetName(hist->GetName()+label);
    hist->SetLineColor(color);
    hist->SetMarkerColor(color);

    hist->Scale(1.f/hist->Integral((VarBins?"width":"")));
  }
  
  const TString label;
  TFile * file;
  TH1F  * hist;
};

Bool_t IsVarBins(const TString & plotconfig)
{
  Bool_t VarBins = false;

  std::string str;
  std::ifstream infile(plotconfig.Data(),std::ios::in);

  while (std::getline(infile,str))
  {
    if (str.find("x_bins=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_bins=");

      if      (str.find("VARIABLE") != std::string::npos) VarBins = true;
      else if (str.find("CONSTANT") != std::string::npos) VarBins = false;
      else
      {
	std::cerr << "bins are either CONSTANT or VARIABLE, not: " << str.c_str() << "! Exiting..." << std::endl;
	exit(1);
      }
      break;
    }
  }

  return VarBins;
}

void SetupInputs(const TString & textfile, std::vector<InputStruct> & inputs, const Bool_t VarBins)
{
  TString filename, label;
  std::ifstream infile(textfile.Data(),std::ios::in);

  Int_t counter = 0;
  while (infile >> filename >> label)
  {
    inputs.emplace_back(label,filename,Common::ColorVec[counter++],VarBins);
  }
}

void GetMinMax(Float_t & min, Float_t & max, std::vector<InputStruct> & inputs)
{
  for (const auto & input : inputs)
  {
    const auto & hist = input.hist;
    for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
    {
      const auto tmp = hist->GetBinContent(ibinX);
      if (tmp < min && tmp > 0.f) min = tmp;
      if (tmp > max)              max = tmp;
    }
  }
}

void MakeCanvas(std::vector<InputStruct> & inputs, TLegend *& leg, const TString & outfiletext, 
		const Float_t min, const Float_t max, const Bool_t isLogy)
{
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy(isLogy);

  for (auto iinput = 0U; iinput < inputs.size(); iinput++)
  {
    auto & hist = inputs[iinput].hist;
    hist->SetMinimum(isLogy ? min / 1.5 : min / 1.05);
    hist->SetMaximum(isLogy ? max * 1.5 : max * 1.05);

    hist->Draw(iinput>0?"ep same":"ep");
  }
  leg->Draw("same");

  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,outfiletext+(isLogy?"_log":"_lin"));

  // delete canv
  delete canv;
}

void overplot_dataplots(const TString & textfile, const TString & plotconfig, const TString & outfiletext)
{
  // set style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);
  Common::SetupEras();

  // read plot config
  const Bool_t VarBins = IsVarBins(plotconfig);
  
  // setup inputs
  std::vector<InputStruct> inputs;
  SetupInputs(textfile,inputs,VarBins);

  // set legend
  auto leg = new TLegend(0.7,0.75,0.825,0.92);
  leg->SetBorderSize(1);
  leg->SetLineColor(kBlack);
  for (const auto & input : inputs)
  {
    leg->AddEntry(input.hist,input.label.Data(),"epl");
  }

  // get min, max
  Float_t min = 1e9, max = -1e9;
  GetMinMax(min,max,inputs);

  // MakeCanvas
  MakeCanvas(inputs,leg,outfiletext,min,max,false);
  MakeCanvas(inputs,leg,outfiletext,min,max,true);

  // delete it all
  delete leg;
  for (auto & input : inputs) 
  {
    delete input.hist;
    delete input.file;
  }
  delete tdrStyle;
}
