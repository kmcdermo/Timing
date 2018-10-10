#include "CommonTimeFit.hh"

namespace Common
{
  void SetupTimeFitType(const std::string & str, TimeFitType & type)
  {
    if      (str.find("Gaus1")       != std::string::npos) type = TimeFitType::Gaus1;
    if      (str.find("Gaus1core")   != std::string::npos) type = TimeFitType::Gaus1core;
    else if (str.find("Gaus2fm")     != std::string::npos) type = TimeFitType::Gaus2fm;
    else if (str.find("Gaus2fmcore") != std::string::npos) type = TimeFitType::Gaus2fmcore;
    else if (str.find("Gaus3fm")     != std::string::npos) type = TimeFitType::Gaus3fm;
    else if (str.find("Gaus3fmcore") != std::string::npos) type = TimeFitType::Gaus3fmcore;
    else
    {
      std::cerr << "Specified a non-supported fit type: " << str.c_str() << " ... Exiting..." << std::endl;
      exit(1);
    }
  }
};

void TimeFitStruct::PrepFit()
{
  // Word on fit notation
  // "GausN" == N Gaussians fit
  // "fm" == "fixed mean", i.e. for N Gaussian fit, all Gaussians share the same mu
  // "core" == mid point of range of fit is mean of the histogram, range is n times the std. dev of hist

  // set tmp init vals
  Float_t norm  = hist->Integral(varBinsX?"width":"") / Common::SqrtPI;
  Float_t mu    = hist->GetMean();
  Float_t sigma = hist->GetStdDev(); 

  // range vars
  auto rangelow = 0.f;
  auto rangeup  = 0.f;

  // make tmp fit first if not gausNcore, set range
  if (type == TimeFitType::Gaus1 || type == TimeFitType::Gaus2fm || type == TimeFitType::Gaus3fm)
  {
    // set range for tmp and main fit
    rangelow = rangeLow;
    rangeup  = rangeUp;

    auto tmp_form = new TFormula("tmp_formula","[0]*exp(-0.5*((x-[1])/[2])**2)");
    auto tmp_fit  = new TF1("tmp_fit",tmp_form->GetName(),rangelow,rangeup);

    tmp_fit->SetParameter(0,norm);
    tmp_fit->SetParameter(1,mu);
    tmp_fit->SetParameter(2,sigma); tmp_fit->SetParLimits(2,0,10);

    // fit hist with tmp tf1
    hist->Fit(tmp_fit->GetName(),"RBQ0");

    norm  = tmp_fit->GetParameter(0); // constant
    mu    = tmp_fit->GetParameter(1); // mu
    sigma = tmp_fit->GetParameter(2); // sigma

    delete tmp_form;
    delete tmp_fit;
  }
  else // "core" fits
  {
    // set range for main fit
    rangelow = (mu-(rangeLow*sigma));
    rangeup  = (mu+(rangeUp *sigma));
  }
  
  // names for fits and formulas
  const TString histname = hist->GetName();
  const TString formname = histname+"_formula";
  const TString fitname  = histname+"_fit";

  if (type == TimeFitType::Gaus1 || type == TimeFitType::Gaus1core)
  {
    form = new TFormula(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)");
    fit  = new TF1(fitname.Data(),form->GetName(),rangelow,rangeup);

    fit->SetParName(0,"N");      fit->SetParameter(0,norm);
    fit->SetParName(1,"#mu");    fit->SetParameter(1,mu);
    fit->SetParName(2,"#sigma"); fit->SetParameter(2,sigma); fit->SetParLimits(2,0,10);
  }
  else if (type == TimeFitType::Gaus2fm || type == TimeFitType::Gaus2fmcore)
  {
    form = new TFormula(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[1])/[4])**2)");
    fit  = new TF1(fitname.Data(),form->GetName(),rangelow,rangeup);

    fit->SetParName(0,"N_{1}");      fit->SetParameter(0,norm);
    fit->SetParName(1,"#mu");        fit->SetParameter(1,mu);
    fit->SetParName(2,"#sigma_{1}"); fit->SetParameter(2,sigma);   fit->SetParLimits(2,0,10);
    fit->SetParName(3,"N_{2}");      fit->SetParameter(3,norm/10);
    fit->SetParName(4,"#sigma_{2}"); fit->SetParameter(4,sigma*4); fit->SetParLimits(4,0,10);
  }
  else if (type == TimeFitType::Gaus3fm || type == TimeFitType::Gaus3fmcore)
  {
    form = new TFormula(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[1])/[4])**2)+[5]*exp(-0.5*((x-[1])/[6])**2)");
    fit  = new TF1(fitname.Data(),form->GetName(),rangelow,rangeup);

    fit->SetParName(0,"N_{1}");      fit->SetParameter(0,norm*0.8);  fit->SetParLimits(0,norm*0.5,norm);
    fit->SetParName(1,"#mu");        fit->SetParameter(1,mu);
    fit->SetParName(2,"#sigma_{1}"); fit->SetParameter(2,sigma*0.7); fit->SetParLimits(2,sigma*0.5,sigma);
    fit->SetParName(3,"N_{2}");      fit->SetParameter(3,norm*0.3);  fit->SetParLimits(3,norm*0.1,norm*0.5);
    fit->SetParName(4,"#sigma_{2}"); fit->SetParameter(4,sigma*1.4); fit->SetParLimits(4,sigma,sigma*1.5);
    fit->SetParName(5,"N_{3}");      fit->SetParameter(5,norm*0.01); fit->SetParLimits(5,norm*0.005,norm*0.1);
    fit->SetParName(6,"#sigma_{3}"); fit->SetParameter(6,sigma*2.5); fit->SetParLimits(6,sigma*1.5,sigma*5.0);
  }
  else
  {
    std::cerr << "How did this happen?? Fit was not set for prepping fits! Exiting..." << std::endl;
    exit(1);
  }
}

void TimeFitStruct::DoFit()
{
  hist->Fit(fit->GetName(),"RBQ0");
}

void TimeFitStruct::GetFitResult()
{
  // get common result
  result.mu       = fit->GetParameter(1);
  result.emu      = fit->GetParError (1);
  result.chi2ndf  = fit->GetChisquare();
  result.chi2prob = fit->GetProb();

  if (type == TimeFitType::Gaus1 || type == TimeFitType::Gaus1core)
  {
    result.sigma  = fit->GetParameter(2);
    result.esigma = fit->GetParError (2);
  }
  else if (type == TimeFitType::Gaus2fm || type == TimeFitType::Gaus2fmcore)
  {
    const Float_t const1 = fit->GetParameter(0); 
    const Float_t const2 = fit->GetParameter(3);
    const Float_t denom  = const1 + const2;

    result.sigma  = (const1*fit->GetParameter(2)+const2*fit->GetParameter(4))/denom;
    result.esigma = std::hypot(const1*fit->GetParError(2),const2*fit->GetParError(4))/denom;
  }
  else if (type == TimeFitType::Gaus3fm || type == TimeFitType::Gaus3fmcore)
  {
    const Double_t const1 = fit->GetParameter(0); 
    const Double_t const2 = fit->GetParameter(3);
    const Double_t const3 = fit->GetParameter(5);
    const Double_t denom  = const1 + const2 + const3;
    
    result.sigma  = (const1*fit->GetParameter(2) + const2*fit->GetParameter(4) + const3*fit->GetParameter(6))/denom;
    result.esigma = std::hypot(std::hypot(const1*fit->GetParError(2),const2*fit->GetParError(4)),const3*fit->GetParError(6))/denom; // need c++17...
  }
  else
  {
    std::cerr << "How did this happen?? Fit was not set for getting result! Exiting..." << std::endl;
    exit(1);
  }
}

void TimeFitStruct::DeleteInternal()
{
  // always delete projected hist
  delete hist;
  
  // skip further deletes if no entries
  if (!isEmpty())
  {
    delete form;
    delete fit;
  }
}
