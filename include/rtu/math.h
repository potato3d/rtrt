#pragma once
#ifndef _RTU_MATH_H_
#define _RTU_MATH_H_

#include "common.h"
#include <cmath>

#if defined(_MSC_VER)
	// float.h is required in Windows for _isnan. Maybe we should
	// detect NaN's using another approach (such as Lua's)?
	#include <float.h>
#endif

// undefine the min/max macros (usually defined by windows.h)
#ifdef min
	#undef min
#endif
#ifdef max
	#undef max
#endif
#ifndef NOMINMAX
	#define NOMINMAX
#endif

// undefine the 'PI' macro if it was defined by other libraries
#ifdef PI
#undef PI
#undef PI_2
#undef PI_4
#endif

namespace rtu {

template<typename real>
class math
{
public:
	// Constants
	static  const real PI;				// PI
	static  const real TWO_PI;			// 2*PI
	static  const real PI_2;			// PI/2
	static  const real PI_4;			// PI/4
	static  const real INV_PI;			// 1/PI
	static  const real LN_2;			// ln(2)
	static  const real LN_10;			// ln(10)
	static  const real INV_LN_2;		// 1/ln(2)
	static  const real INV_LN_10;		// 1/ln(10)
	static  const real DEG_TO_RAD;		// Convert degrees to radians
	static  const real RAD_TO_DEG;		// Convert radians to degrees
	static  const real ZERO_TOLERANCE;	// Approximation to zero
	static  const real EPSILON;			// Smallest difference between two numbers
	static  const real MAX_VALUE;		// Maximum valid value for a number
	static  const real MIN_VALUE;		// Minimum valid value for a number
	static  const real PLUS_INF;		// Positive infinity
	static  const real MINUS_INF;		// Negative infinity

	// Logarithms
	inline static real log2( real scalar );
	inline static real log10( real scalar );

	// Tests if the given floating-point value is not a number (NaN).
	inline static bool isNaN( real scalar );

	// Tests if the given floating-point value is infinite (INF) or not a number (NaN).
	inline static bool isInvalid( real scalar );

	// Compares two real values using appropriate ZERO_TOLERANCE
	inline static bool isEqual( real a, real b );

	// Converts an angle from degrees to radians
	inline static real toRadians( real degrees );

	// Converts an angle from radians to degrees
	inline static real toDegrees( real radians );

	// The absolute value of a number -- its value without regard to sign.
	inline static real abs( real a );

	// The minimum of two values.
	inline static real min( real a, real b );

	// The maximum of two values.
	inline static real max( real a, real b );

	// Return the fraction digits of the given floating point value. The result is always positive.
	inline static real frac( real a );

	// Round to the nearest integer in floating point representation. Ex: 1.2 becomes 1.0, 2.5 becomes 3.0, etc.
	inline static real round( real a );

	// Clamps a value to the interval [minimum, maximum].
	inline static real clampTo( real value, real minimum, real maximum );

	// Clamps a value if it's below a minimum.
	inline static real clampAbove( real value, real minimum );

	// Clamps a value if it's above a maximum.
	inline static real clampBelow( real value, real maximum );

	// Returns 1 if the value is positive or -1 if the value is negative.
	inline static real sign( real value );

	// Returns the sign bit of the value: 0 if positive or 1 if negative.
	inline static uint32 signBit( real value );

	// Returns the square of a number (the number multiplied by itself).
	inline static real square( real value );

	// Returns the square of a number multiplied by its sign.
	// I.e. this function keeps the square of negative numbers negative.
	inline static real signedSquare( real v );

	// Computes the power of two immediately above a value. 
	// Ex1: 11 becomes 16. Ex2: 4 becomes 8.
	inline static uint32 nextPowerOf2( uint32 value );
};

// Logarithms
template<typename real>
inline real math<real>::log2( real scalar )
{
	return log( scalar ) * INV_LN_2();
}

template<typename real>
inline real math<real>::log10( real scalar )
{
	return log( scalar ) * INV_LN_10();
}

// Tests if the given floating-point value is not a number (NaN).
template<typename real>
inline bool math<real>::isNaN( real scalar )
{
#if defined(_MSC_VER)
	return _isnan( scalar ) != 0;
#else
	return isnan( scalar );
#endif
}

// Tests if the given floating-point value is infinite (INF) or not a number (NaN).
template<typename real>
inline bool math<real>::isInvalid( real scalar )
{
#if defined(_MSC_VER)
	return _finite( scalar ) == 0;
#else
	return finite( scalar ) == 0;
#endif
}

// Compares two real values using appropriate ZERO_TOLERANCE
template<typename real>
inline bool math<real>::isEqual( real a, real b )
{
	real delta = b - a;
	return ( delta < 0 ? delta >= -ZERO_TOLERANCE : delta <= ZERO_TOLERANCE );
}

// Converts an angle from degrees to radians
template<typename real>
inline real math<real>::toRadians( real degrees )
{
	return degrees * DEG_TO_RAD;
}

// Converts an angle from radians to degrees
template<typename real>
inline real math<real>::toDegrees( real radians )
{
	return radians * RAD_TO_DEG;
}

// The absolute value of a number -- its value without regard to sign.
template<typename real>
inline real math<real>::abs( real a )
{
	return ( a >= 0  ? a : -a );
}

// The minimum of two values.
template<typename real>
inline real math<real>::min( real a, real b )
{
	return ( a < b ? a : b );
}

// The maximum of two values.
template<typename real>
inline real math<real>::max( real a, real b )
{
	return ( a > b ? a : b );
}

// Return the fraction digits of the given floating point value. The result is always positive.
template<typename real>
inline real rtu::math<real>::frac( real a )
{
	return ( a >= 0 ) ? a - (int)a : frac( -a );
}

// Round to the nearest integer. Ex: 1.2 becomes 1, 2.5 becomes 3, etc.
template<typename real>
inline real rtu::math<real>::round( real a )
{
	return ( frac( a ) < static_cast<real>( 0.5 ) ) ? floor( a ) : ceil( a );
}

// Clamps a value to the interval [minimum, maximum].
template<typename real>
inline real math<real>::clampTo( real value, real minimum, real maximum )
{
	return ( value < minimum ? minimum : ( value > maximum ? maximum : value ) );
}

// Clamps a value if it's below a minimum.
template<typename real>
inline real math<real>::clampAbove( real value, real minimum )
{
	return ( value < minimum ? minimum : value );
}

// Clamps a value if it's above a maximum.
template<typename real>
inline real math<real>::clampBelow( real value, real maximum )
{
	return ( value > maximum ? maximum : value );
}

// Returns 1 if the value is positive or -1 if the value is negative.
template<typename real>
inline real math<real>::sign( real value )
{
	return ( value < 0 ? -1 : 1 );
}

// Returns the sign bit of the value: 0 if positive and 1 if negative.
template<typename real>
inline uint32 rtu::math<real>::signBit( real value )
{
	return ( value < 0 ) ? 1 : 0;
}

// Returns the square of a number (the number multiplied by itself).
template<typename real>
inline real square( real value )
{
	return ( value * value );
}

// Returns the square of a number multiplied by its sign.
// I.e. this function keeps the square of negative numbers negative.
template<typename real>
inline real math<real>::signedSquare( real v )
{
	return ( v < 0 ? -v * v : v * v );
}

// Computes the power of two immediately above a value.
template<typename real>
inline uint32 math<real>::nextPowerOf2( uint32 value )
{
	value |= ( value >> 1 );
	value |= ( value >> 2 );
	value |= ( value >> 4 );
	value |= ( value >> 8 );
	value |= ( value >> 16 );
	return ( value + 1 );
}

typedef math<float>  mathf;
typedef math<double> mathd;

} // namespace rtu

#endif
