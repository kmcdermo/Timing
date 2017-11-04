#ifndef _analysisutils_
#define _analysisutils_

inline Float_t rad2  (const Float_t x, const Float_t y){return x*x + y*y;}
inline Float_t theta (const Float_t r, const Float_t z){return std::atan2(r,z);}
inline Float_t eta   (const Float_t x, const Float_t y, const Float_t z)
{
  return -1.0f*std::log(std::tan(theta(std::sqrt(rad2(x,y)),z)/2.f));
}
inline Float_t rad2  (const Float_t x, const Float_t y, const Float_t z)
{
  return x*x + y*y + z*z;
}
inline Float_t phi   (const Float_t x, const Float_t y){return std::atan2(y,x);}
inline Float_t mphi  (Float_t phi)
{    
  while (phi >= Config::PI) phi -= Config::TWOPI;
  while (phi < -Config::PI) phi += Config::TWOPI;
  return phi;
}
inline Float_t deltaR(const Float_t eta1, const Float_t phi1, const Float_t eta2, const Float_t phi2)
{
  return std::sqrt(rad2(eta2-eta1,mphi(phi1-phi2)));
}
inline Float_t TOF   (const Float_t x,  const Float_t y,  const Float_t z, 
		      const Float_t vx, const Float_t vy, const Float_t vz, const Float_t time)
{
  return time + (std::sqrt(rad2(x,y,z))-std::sqrt(rad2((x-vx),(y-vy),(z-vz))))/Config::sol;
}

// PF isolation EAs

inline float GetChargedHadronEA(const float eta)
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
  
inline float GetNeutralHadronEA(const float eta) 
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
  
inline float GetGammaEA(const float eta) 
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

// PF Iso based pt corrections

inline float GetChargedHadronPt(const bool isEB, const float pt)
{
  if   (isEB) return 0.f*pt;
  else        return 0.f*pt;
}

inline float GetNeutralHadronPt(const bool isEB, const float pt)
{
  if   (isEB) return 0.0148*pt + 0.000017*pt*pt;
  else        return 0.0163*pt + 0.000014*pt*pt;
}

inline float GetGammaPt(const bool isEB, const float pt)
{
  if   (isEB) return 0.0047*pt;
  else        return 0.0034*pt;
}

// Detector based EAs

inline float GetEcalPFClEA(const bool isEB)
{
  if   (isEB) return 0.167;
  else        return 0.;
}
  
inline float GetHcalPFClEA(const bool isEB) 
{
  if   (isEB) return 0.108;
  else        return 0.;
}
  
inline float GetTrackEA(const bool isEB) 
{
  if   (isEB) return 0.113;
  else        return 0.;
}

// Det Iso based pt corrections

inline float GetEcalPFClPt(const bool isEB, const float pt)
{
  if   (isEB) return 0.0028*pt;
  else        return 0.f*pt;
}

inline float GetHcalPFClPt(const bool isEB, const float pt)
{
  if   (isEB) return 0.0087*pt;
  else        return 0.f*pt;
}

inline float GetTrackPt(const bool isEB, const float pt)
{
  if   (isEB) return 0.0056*pt;
  else        return 0.f*pt;
}

#endif
