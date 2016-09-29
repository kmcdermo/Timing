#include "testTimeFitter.h"

void testTimeFitter()
{
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  TVirtualFitter::SetDefaultFitter("Minuit2");

  Config::outdir = "testFits";
  FileStat_t dummyFileStat; 
  if (gSystem->GetPathInfo(Config::outdir.Data(), dummyFileStat) == 1){
    TString mkDir = "mkdir -p ";
    mkDir += Config::outdir.Data();
    gSystem->Exec(mkDir.Data());
  }
  
  std::vector<TString>  formnames = {"gaus2fm","gaus3fm"};
  std::vector<Double_t> fitranges = {5.0};
  std::vector<TString>  nbinses   = {"40"};
  std::vector<TString>  histnames = {"h_tdweighttime","h_tdseedtimeTOF"};

  //  std::vector<TString>  histnames = {"h_tdweighttime","h_tdseedtimeTOF"};
  //  std::vector<TString>  nbinses   = {"40","100"};

//   std::vector<TString>  formnames = {"gaus1"};
//   std::vector<Double_t> fitranges = {0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,1.5,2.0,3.0,5.0};
  
  for (UInt_t inbins = 0; inbins < nbinses.size(); inbins++) 
  {
    Config::nbins = nbinses[inbins];
    TFile * file  = TFile::Open(Form("tdouttimes%s.root",Config::nbins.Data()));
    for (UInt_t ihist = 0; ihist < histnames.size(); ihist++) 
    { 
      Config::histname = histnames[ihist];
      TH1F * hist = (TH1F*)file->Get(Config::histname.Data());
      hist->Scale(1.0/hist->Integral());
      hist->SetMarkerStyle(1);
      hist->SetMarkerColor(kBlack);
      hist->SetLineColor(kBlack);
      hist->SetTitle(Form("%s (%s bins)",hist->GetTitle(),Config::nbins.Data()));
      hist->GetXaxis()->SetTitle("Time [ns]");
      hist->GetYaxis()->SetTitle("Events");
    
      for (UInt_t itype = 0; itype < formnames.size(); itype++) 
      {
	Config::formname = formnames[itype];
	for (UInt_t irange = 0; irange < fitranges.size(); irange++) 
	{
	  Config::fitrange = fitranges[irange];
	  fittingCore(hist);
	}
      }
      delete hist;
    }
    delete file;
  }
}

void fittingCore(TH1F *& hist)
{
  TCanvas * lincanv = new TCanvas();
  lincanv->SetLogy(0);
  lincanv->cd();

  Double_t mean, emean, sigma, esigma;
  TF1 * fit;
  arr3 temps;

  doFit(hist,fit,temps);
  getFitParams(fit,mean,emean,sigma,esigma);
  hist->SetMaximum(Config::nbins == 100 ? 0.12 : 0.3); 
  hist->SetMinimum(0.0);
  hist->Draw("ep");

  TF1 * sub1; TF1 * sub2; TF1 * sub3;
  drawFit(fit,lincanv,sub1,sub2,sub3);

  TPaveText * text = new TPaveText(0.6,0.78,0.9,0.88,"NDC");
  text->AddText(Form("#mu = %f #pm %f",mean,emean));
  text->AddText(Form("#sigma = %f #pm %f",sigma,esigma));
  text->AddText(Form("#chi^{2} / ndf = %6.2f / %i",fit->GetChisquare(),fit->GetNDF()));
  text->SetTextSize(0.03);
  text->SetFillColorAlpha(kWhite,0.f);
  text->Draw("same");
  
  lincanv->SaveAs(Form("%s/%s_%sbins_%3.1f_%s_lin.pdf",Config::outdir.Data(),Config::histname.Data(),Config::nbins.Data(),Config::fitrange,Config::formname.Data()));

  // log canvas saves individ. params
  TCanvas * logcanv = new TCanvas();
  logcanv->SetLogy(1);
  logcanv->cd();

  hist->SetMaximum(Config::nbins == 100 ? 0.2 : 0.45);
  hist->SetMinimum(0.000001);
  hist->Draw("ep");
  drawFit(fit,logcanv,sub1,sub2,sub3);

  TPaveText * stats;
  drawStats(fit,stats,logcanv);

  logcanv->SaveAs(Form("%s/%s_%sbins_%3.1f_%s_log.pdf",Config::outdir.Data(),Config::histname.Data(),Config::nbins.Data(),Config::fitrange,Config::formname.Data()));

  // delete everything
  deleteAll(fit,sub1,sub2,sub3,text,lincanv,stats,logcanv);

//   TFormula form(Config::formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)");
//   TF1 * fitfull = new TF1(Form("%s_fit_full",Config::formname.Data()),Config::formname.Data(),-3.0,3.0);
//   fitfull->SetParameters(fit->GetParameter(0),fit->GetParameter(1),fit->GetParameter(2));
//   fitfull->SetLineWidth(3);
//   fitfull->SetLineColor(kMagenta-3); //kViolet-6
//   fitfull->Draw("same");
}

void doFit(TH1F *& hist, TF1 *& fit, arr3 & temps)
{
  TF1 * tempfit = new TF1("temp","gaus(0)",-Config::fitrange,Config::fitrange);
  tempfit->SetParLimits(2,0,10);
  hist->Fit("temp","RB0Q"); // for real testing, do not suppress output --> drop q
  const Double_t norm  = tempfit->GetParameter(0); // constant
  const Double_t mean  = tempfit->GetParameter(1); // mean
  const Double_t sigma = tempfit->GetParameter(2); // sigma
  temps = {norm,mean,sigma};
  delete tempfit;

  if (Config::formname.EqualTo("gaus1",TString::kExact)) {
    TFormula form(Config::formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)");
    fit  = new TF1(Form("%s_fit",Config::formname.Data()),Config::formname.Data(),-Config::fitrange,Config::fitrange);
    fit->SetParName(0,"norm");  fit->SetParameter(0,norm);
    fit->SetParName(1,"mean");  fit->SetParameter(1,mean);
    fit->SetParName(2,"sigma"); fit->SetParameter(2,sigma);
  }
  else if (Config::formname.EqualTo("gaus2",TString::kExact)) {
    TFormula form(Config::formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[4])/[5])**2)");
    fit  = new TF1(Form("%s_fit",Config::formname.Data()),Config::formname.Data(),-Config::fitrange,Config::fitrange);
    fit->SetParName(0,"norm1");  fit->SetParameter(0,norm);
    fit->SetParName(1,"mean1");  fit->SetParameter(1,mean);
    fit->SetParName(2,"sigma1"); fit->SetParameter(2,sigma);     fit->SetParLimits(2,0,10);
    fit->SetParName(3,"norm2");  fit->SetParameter(3,norm*0.1);
    fit->SetParName(4,"mean2");  fit->SetParameter(4,0);
    fit->SetParName(5,"sigma2"); fit->SetParameter(5,sigma*4.0); fit->SetParLimits(5,0,10);
  }
  else if (Config::formname.EqualTo("gaus2fm",TString::kExact)) {
    TFormula form(Config::formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[1])/[4])**2)");
    fit  = new TF1(Form("%s_fit",Config::formname.Data()),Config::formname.Data(),-Config::fitrange,Config::fitrange);
    fit->SetParName(0,"norm1");  fit->SetParameter(0,norm);  
    fit->SetParName(1,"mean");   fit->SetParameter(1,mean); 
    fit->SetParName(2,"sigma1"); fit->SetParameter(2,sigma);     fit->SetParLimits(2,0,10); 
    fit->SetParName(3,"norm2");  fit->SetParameter(3,norm*0.1); 
    fit->SetParName(4,"sigma2"); fit->SetParameter(4,sigma*4.0); fit->SetParLimits(4,0,10);
  }
  else if (Config::formname.EqualTo("gaus3fm",TString::kExact)) {
    TFormula form(Config::formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[1])/[4])**2)+[5]*exp(-0.5*((x-[1])/[6])**2)");
    fit  = new TF1(Form("%s_fit",Config::formname.Data()),Config::formname.Data(),-Config::fitrange,Config::fitrange);
    fit->SetParName(0,"norm1");  fit->SetParameter(0,norm*0.8);  fit->SetParLimits(0,norm*0.5,norm);
    fit->SetParName(1,"mean");   fit->SetParameter(1,mean);
    fit->SetParName(2,"sigma1"); fit->SetParameter(2,sigma*0.7); fit->SetParLimits(2,sigma*0.5,sigma);
    fit->SetParName(3,"norm2");  fit->SetParameter(3,norm*0.3);  fit->SetParLimits(3,norm*0.1,norm*0.5);
    fit->SetParName(4,"sigma2"); fit->SetParameter(4,sigma*1.4); fit->SetParLimits(4,sigma,sigma*1.5);
    fit->SetParName(5,"norm3");  fit->SetParameter(5,norm*0.01); fit->SetParLimits(5,norm*0.005,norm*0.1);
    fit->SetParName(6,"sigma3"); fit->SetParameter(6,sigma*2.5); fit->SetParLimits(6,sigma*1.5,sigma*5.0);
  }
  else if (Config::formname.EqualTo("crystalball",TString::kExact)) {
    //  use directly the functionin ROOT::MATH note that the parameters definition is different is (alpha, n sigma, mu)
    fit = new TF1(Form("%s_fit",Config::formname.Data()),Config::formname.Data(),-Config::fitrange,Config::fitrange);
    fit->SetParameters(norm, mean, sigma, 1.0, 0.5);     // N, mean, sigma, alpha, n
  }

  Int_t status = hist->Fit(fit->GetName(),"RB0Q");
}

void getFitParams(TF1 *& fit, Double_t & mean, Double_t & emean, Double_t & sigma, Double_t & esigma) 
{
  if (Config::formname.EqualTo("gaus1",TString::kExact) || Config::formname.EqualTo("crystalball",TString::kExact)) {
    mean   = fit->GetParameter(1);
    sigma  = fit->GetParameter(2);

    emean  = fit->GetParError(1);
    esigma = fit->GetParError(2);
  }
  else  if (Config::formname.EqualTo("gaus2",TString::kExact)) {
    const Double_t const1 = fit->GetParameter(0); 
    const Double_t const2 = fit->GetParameter(3);
    const Double_t denom =  const1 + const2;

    mean   = (const1*fit->GetParameter(1) + const2*fit->GetParameter(4))/denom;
    sigma  = (const1*fit->GetParameter(2) + const2*fit->GetParameter(5))/denom;

    emean  = rad2(const1*fit->GetParError(1),const2*fit->GetParError(4));
    esigma = rad2(const1*fit->GetParError(2),const2*fit->GetParError(5));

    emean  = std::sqrt(emean) /denom;
    esigma = std::sqrt(esigma)/denom;
  }
  else if (Config::formname.EqualTo("gaus2fm",TString::kExact)) {
    const Double_t const1 = fit->GetParameter(0); 
    const Double_t const2 = fit->GetParameter(3);
    const Double_t denom =  const1 + const2;

    mean   = fit->GetParameter(1);
    sigma  = (const1*fit->GetParameter(2) + const2*fit->GetParameter(4))/denom;

    emean  = fit->GetParError(1);
    esigma = rad2(const1*fit->GetParError(2),const2*fit->GetParError(4));

    esigma = std::sqrt(esigma)/denom;
  }
  else if (Config::formname.EqualTo("gaus3fm",TString::kExact)) {
    const Double_t const1 = fit->GetParameter(0); 
    const Double_t const2 = fit->GetParameter(3);
    const Double_t const3 = fit->GetParameter(5);
    const Double_t denom  =  const1 + const2 + const3;

    mean   = fit->GetParameter(1);
    sigma  = (const1*fit->GetParameter(2) + const2*fit->GetParameter(4) + const3*fit->GetParameter(6))/denom;

    emean  = fit->GetParError(1);
    esigma = rad2(const1*fit->GetParError(2),const2*fit->GetParError(4))+std::pow(const3*fit->GetParError(6),2);

    esigma = std::sqrt(esigma)/denom;
  }
}

void dumpFit(TF1 *& fit, arr3 & temps)
{
  if (Config::formname.EqualTo("gaus3fm",TString::kExact)) {
    std::cout << std::endl
	      << " n1/n0: " << fit->GetParameter(0) / temps[0] << std::endl
	      << " n2/n0: " << fit->GetParameter(3) / temps[0] << std::endl
	      << " n3/n0: " << fit->GetParameter(5) / temps[0] << std::endl
	      << " s1/s0: " << fit->GetParameter(2) / temps[2] << std::endl
	      << " s2/s0: " << fit->GetParameter(4) / temps[2] << std::endl
	      << " s3/s0: " << fit->GetParameter(6) / temps[2] << std::endl
	      << " m-m0 : " << fit->GetParameter(1) - temps[1] << std::endl
	      << std::endl;
  }
}

void drawFit(TF1 *& fit, TCanvas *& canv, TF1 *& sub1, TF1 *& sub2, TF1 *& sub3) 
{
  canv->cd();

  fit->SetLineWidth(3);
  fit->SetLineColor(kMagenta-3); //kViolet-6
  fit->Draw("same");

  if (Config::formname.EqualTo("gaus1",TString::kExact) || Config::formname.EqualTo("crystalball",TString::kExact)) {
    return;
  }
  else if (Config::formname.EqualTo("gaus2",TString::kExact)) {
    sub1 = new TF1("sub1","gaus(0)",-Config::fitrange,Config::fitrange);
    sub1->SetParameters(fit->GetParameter(0),fit->GetParameter(1),fit->GetParameter(2));
  
    sub2 = new TF1("sub2","gaus(0)",-Config::fitrange,Config::fitrange);
    sub2->SetParameters(fit->GetParameter(3),fit->GetParameter(4),fit->GetParameter(5));
  } 
  else if (Config::formname.EqualTo("gaus2fm",TString::kExact)) {
    sub1 = new TF1("sub1","gaus(0)",-Config::fitrange,Config::fitrange);
    sub1->SetParameters(fit->GetParameter(0),fit->GetParameter(1),fit->GetParameter(2));
  
    sub2 = new TF1("sub2","gaus(0)",-Config::fitrange,Config::fitrange);
    sub2->SetParameters(fit->GetParameter(3),fit->GetParameter(1),fit->GetParameter(4));
  } 
  else if (Config::formname.EqualTo("gaus3fm",TString::kExact)) {
    sub1 = new TF1("sub1","gaus(0)",-Config::fitrange,Config::fitrange);
    sub1->SetParameters(fit->GetParameter(0),fit->GetParameter(1),fit->GetParameter(2));
  
    sub2 = new TF1("sub2","gaus(0)",-Config::fitrange,Config::fitrange);
    sub2->SetParameters(fit->GetParameter(3),fit->GetParameter(1),fit->GetParameter(4));

    sub3 = new TF1("sub3","gaus(0)",-Config::fitrange,Config::fitrange);
    sub3->SetParameters(fit->GetParameter(5),fit->GetParameter(1),fit->GetParameter(6));

    sub3->SetLineColor(kGreen-3); // kViolet-3
    sub3->SetLineWidth(2);
    sub3->SetLineStyle(7);
    sub3->Draw("same");
  } 
  
  sub1->SetLineColor(kRed) ;  // kgreen-3
  sub1->SetLineWidth(2);
  sub1->SetLineStyle(7);
  sub1->Draw("same");

  sub2->SetLineColor(kBlue); // kViolet-3
  sub2->SetLineWidth(2);
  sub2->SetLineStyle(7);
  sub2->Draw("same");
}

void drawStats(TF1 *& fit, TPaveText *& stats, TCanvas *& canv)
{
  canv->cd();
  if (Config::formname.EqualTo("gaus1",TString::kExact)) 
  {
    stats = new TPaveText(0.6,0.78,0.9,0.88,"NDC");
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(0),fit->GetParameter(0),fit->GetParError(0)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(1),fit->GetParameter(1),fit->GetParError(1)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(2),fit->GetParameter(2),fit->GetParError(2)));
    stats->SetTextSize(0.03);
    stats->SetFillColorAlpha(kWhite,0.f);
    stats->Draw("same");
  }
  else if (Config::formname.EqualTo("gaus2",TString::kExact)) {
    stats = new TPaveText(0.6,0.68,0.9,0.88,"NDC");
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(0),fit->GetParameter(0),fit->GetParError(0)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(1),fit->GetParameter(1),fit->GetParError(1)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(2),fit->GetParameter(2),fit->GetParError(2)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(3),fit->GetParameter(3),fit->GetParError(3)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(4),fit->GetParameter(4),fit->GetParError(4)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(5),fit->GetParameter(5),fit->GetParError(5)));
    stats->SetTextSize(0.03);
    stats->SetFillColorAlpha(kWhite,0.f);
    stats->Draw("same");
  } 
  else if (Config::formname.EqualTo("gaus2fm",TString::kExact)) {
    stats = new TPaveText(0.6,0.68,0.9,0.88,"NDC");
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(0),fit->GetParameter(0),fit->GetParError(0)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(1),fit->GetParameter(1),fit->GetParError(1)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(2),fit->GetParameter(2),fit->GetParError(2)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(3),fit->GetParameter(3),fit->GetParError(3)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(4),fit->GetParameter(4),fit->GetParError(4)));
    stats->SetTextSize(0.03);
    stats->SetFillColorAlpha(kWhite,0.f);
    stats->Draw("same");
  } 
  else if (Config::formname.EqualTo("gaus3fm",TString::kExact)) {
    stats = new TPaveText(0.6,0.68,0.9,0.88,"NDC");
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(0),fit->GetParameter(0),fit->GetParError(0)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(1),fit->GetParameter(1),fit->GetParError(1)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(2),fit->GetParameter(2),fit->GetParError(2)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(3),fit->GetParameter(3),fit->GetParError(3)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(4),fit->GetParameter(4),fit->GetParError(4)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(5),fit->GetParameter(5),fit->GetParError(5)));
    stats->AddText(Form("%s: %f #pm %f",fit->GetParName(6),fit->GetParameter(6),fit->GetParError(6)));
    stats->SetTextSize(0.03);
    stats->SetFillColorAlpha(kWhite,0.f);
    stats->Draw("same");
  } 
}

void deleteAll(TF1 *& fit, TF1 *& sub1, TF1 *& sub2, TF1 *& sub3, TPaveText *& text, TCanvas *& lincanv, TPaveText *& stats, TCanvas *& logcanv) 
{
  delete fit;
  if (Config::formname.EqualTo("gaus2",TString::kExact) || Config::formname.EqualTo("gaus2fm",TString::kExact) || Config::formname.EqualTo("gaus3fm",TString::kExact))
  {
    delete sub1;
    delete sub2;
    if (Config::formname.EqualTo("gaus3fm",TString::kExact)) 
    {
      delete sub3;
    }
  }
  delete text;
  delete lincanv;
  delete stats;
  delete logcanv;
}
