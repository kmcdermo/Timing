// basic C++ types
#include <vector>
#include <utility>
#include <unordered_map>
#include <cmath>
#include <algorithm>

// DataFormats
#include "DataFormats/DetId/interface/DetId.h"

typedef std::vector<std::pair<DetId,float> > DetIdPairVec;
typedef std::unordered_map<uint32_t,int> uiiumap;

constexpr float PI    =     3.14159265358979323846;
constexpr float TWOPI = 2.0*3.14159265358979323846;

inline float rad2  (const float x, const float y){return x*x + y*y;}
inline float phi   (const float x, const float y){return std::atan2(y,x);}
inline float theta (const float r, const float z){return std::atan2(r,z);}
inline float eta   (const float x, const float y, const float z)
{
  return -1.0f*std::log(std::tan(theta(std::sqrt(rad2(x,y)),z)/2.f));
}
inline float deltaR(const float phi1, const float eta1, const float phi2, const float eta2)
{
  if   (std::abs(phi2-phi1)<PI) 
  {
    return std::sqrt(rad2(eta2-eta1,phi2-phi1));
  }
  else 
  {
    if (phi2-phi1>0.f) 
    {
      return std::sqrt(rad2(eta2-eta1,  TWOPI-(phi2-phi1) ));
    }
    else // technically, the last sign flip is unnecessary here
    {
      return std::sqrt(rad2(eta2-eta1,-(TWOPI+(phi2-phi1))));
    }
  }
}
