//#include "precompiled.h"
#include <rtu/math.h>

namespace rtu {

template<>  const float math<float>::PI			       = 3.1415926535897932384626433832795f;
template<>  const float math<float>::TWO_PI		       = 6.283185307179586476925286766559f;
template<>  const float math<float>::PI_2		       = 1.5707963267948966192313216916398f;
template<>  const float math<float>::PI_4	           = 0.78539816339744830961566084581988f;
template<>  const float math<float>::INV_PI		       = 0.31830988618379067153776752674503f;
template<>  const float math<float>::LN_2		       = 0.69314718055994530941723212145818f;
template<>  const float math<float>::LN_10		       = 2.3025850929940456840179914546844f;
template<>  const float math<float>::INV_LN_2	       = 1.4426950408889634073599246810023f;
template<>  const float math<float>::INV_LN_10	       = 0.43429448190325182765112891891667f;
template<>  const float math<float>::DEG_TO_RAD        = 0.017453292519943295769236907684886f;
template<>  const float math<float>::RAD_TO_DEG        = 57.295779513082320876798154814105f;
template<>  const float math<float>::ZERO_TOLERANCE    = 1e-06f;
template<>  const float math<float>::EPSILON		   = FLT_EPSILON;
template<>  const float math<float>::MAX_VALUE	       = FLT_MAX;
template<>  const float math<float>::MIN_VALUE	       = FLT_MIN;
template<>  const float math<float>::PLUS_INF	       = -logf(0.0f);
template<>  const float math<float>::MINUS_INF	       = logf(0.0f);

template<>  const double math<double>::PI			   = 3.1415926535897932384626433832795;
template<>  const double math<double>::TWO_PI		   = 6.283185307179586476925286766559;
template<>  const double math<double>::PI_2		       = 1.5707963267948966192313216916398;
template<>  const double math<double>::PI_4	           = 0.78539816339744830961566084581988;
template<>  const double math<double>::INV_PI		   = 0.31830988618379067153776752674503;
template<>  const double math<double>::LN_2		       = 0.69314718055994530941723212145818;
template<>  const double math<double>::LN_10		   = 2.3025850929940456840179914546844;
template<>  const double math<double>::INV_LN_2	       = 1.4426950408889634073599246810023;
template<>  const double math<double>::INV_LN_10	   = 0.43429448190325182765112891891667;
template<>  const double math<double>::DEG_TO_RAD      = 0.017453292519943295769236907684886;
template<>  const double math<double>::RAD_TO_DEG      = 57.295779513082320876798154814105;
template<>  const double math<double>::ZERO_TOLERANCE  = 1e-08;
template<>  const double math<double>::EPSILON		   = DBL_EPSILON;
template<>  const double math<double>::MAX_VALUE	   = DBL_MAX;
template<>  const double math<double>::MIN_VALUE	   = DBL_MIN;
template<>  const double math<double>::PLUS_INF	       = -log(0.0);
template<>  const double math<double>::MINUS_INF	   = logf(0.0);

} // namespace rtu
