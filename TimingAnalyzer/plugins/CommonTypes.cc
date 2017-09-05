#include "CommonTypes.h"

namespace oot
{
  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, 
		   const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
		   std::vector<oot::Photon> & photons, const float phpTmin)
  {
    if (photonsH.isValid()) // standard handle check
    {
      for (const auto& photon : *photonsH)
      {
	if (photon.pt() >= phpTmin) photons.emplace_back(photon,false);
      }
    }
    
    if (ootPhotonsH.isValid()) // standard handle check
    {
      for (const auto& photon : *ootPhotonsH)
      {
	  if (photon.pt() >= phpTmin) photons.emplace_back(photon,true);
      }
    }
    
    std::sort(photons.begin(),photons.end(),oot::sortByPt);
  }  
  
  void PrepJets(const edm::Handle<std::vector<pat::Jet> > & jetsH, 
		std::vector<pat::Jet> & jets, const float jetpTmin)
  {
    if (jetsH.isValid()) // standard handle check
    {
      for (const auto& jet : *jetsH)
      {
	if (jet.pt() > jetpTmin) jets.push_back(jet);
      }
      
      std::sort(jets.begin(),jets.end(),oot::sortByPt);
    }
  }  

  float GetChargedHadronEA(const float eta)
  {
    if      (eta <  1.0)                  return 0.0360;
    else if (eta >= 1.0   && eta < 1.479) return 0.0377;
    else if (eta >= 1.479 && eta < 2.0  ) return 0.0306;
    else if (eta >= 2.0   && eta < 2.2  ) return 0.0283;
    else if (eta >= 2.2   && eta < 2.3  ) return 0.0254;
    else if (eta >= 2.3   && eta < 2.4  ) return 0.0217;
    else if (eta >= 2.4)                  return 0.0167;
    else                                  return 0.;
  }
  
  float GetNeutralHadronEA(const float eta) 
  {
    if      (eta <  1.0)                  return 0.0597;
    else if (eta >= 1.0   && eta < 1.479) return 0.0807;
    else if (eta >= 1.479 && eta < 2.0  ) return 0.0629;
    else if (eta >= 2.0   && eta < 2.2  ) return 0.0197;
    else if (eta >= 2.2   && eta < 2.3  ) return 0.0184;
    else if (eta >= 2.3   && eta < 2.4  ) return 0.0284;
    else if (eta >= 2.4)                  return 0.0591;
    else                                  return 0.;
  }
  
  float GetGammaEA(const float eta) 
  {
    if      (eta <  1.0)                  return 0.1210;
    else if (eta >= 1.0   && eta < 1.479) return 0.1107;
    else if (eta >= 1.479 && eta < 2.0  ) return 0.0699;
    else if (eta >= 2.0   && eta < 2.2  ) return 0.1056;
    else if (eta >= 2.2   && eta < 2.3  ) return 0.1457;
    else if (eta >= 2.3   && eta < 2.4  ) return 0.1719;
    else if (eta >= 2.4)                  return 0.1998;
    else                                  return 0.;
  }
};
