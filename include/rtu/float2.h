#pragma once
#ifndef _RTU_FLOAT2_H_
#define _RTU_FLOAT2_H_

#include <rtu/common.h>
#include <rtu/math.h>

namespace rtu {

template<typename real>
class real2
{
public:
	// Public members
	union
	{
		struct { real x; real y; };
		struct { real s; real t; };
	};

	// Constants
	inline static real2<real> UNIT_X();  // ( 1, 0 )
	inline static real2<real> UNIT_Y();  // ( 0, 1 )

	// Construction
	inline real2(); // Uninitialized
	inline real2( real scalar1, real scalar2 );
	inline explicit real2( real const * const pair );

	// Getters
	inline real* ptr();
	inline real  operator[]( int32 i ) const;
	inline real& operator[]( int32 i );

	// Setters
	inline void set( real scalar1, real scalar2 );
	inline void set( real const * const pair );

	// Arithmetic operations
	inline real2<real> operator+( const real2<real>& other ) const;
	inline real2<real> operator-( const real2<real>& other ) const;
	inline real2<real> operator*( const real2<real>& other ) const; // Componentwise
	inline real2<real> operator*( real scalar ) const;
	inline real2<real> operator-() const;

	// Arithmetic updates
	inline real2<real>& operator+=( const real2<real>& other );
	inline real2<real>& operator-=( const real2<real>& other );
	inline real2<real>& operator*=( const real2<real>& other ); // Componentwise
	inline real2<real>& operator*=( real scalar );

	// Vector operations
	inline real length() const;
	inline real length2() const;

	// Used when previous length is needed
	inline real normalize();

	// Only normalizes if needed, saves unnecessary sqrt
	inline void tryNormalize();

	inline real dot( const real2<real>& other ) const;
};

// Constants
template<typename real>
real2<real> real2<real>::UNIT_X()
{
	return real2<real>( 1, 0, 0 );
}

template<typename real>
real2<real> real2<real>::UNIT_Y()
{
	return real2<real>( 0, 1, 0 );
}

// Construction
template<typename real>
real2<real>::real2()
{
	// Do not initialize for performance!
}

template<typename real>
real2<real>::real2( real scalar1, real scalar2 )
	: x( scalar1 ), y( scalar2 )
{
}

template<typename real>
real2<real>::real2( real const * const pair )
	: x( pair[0] ), y( pair[1] )
{
}

// Getters
template<typename real>
real* real2<real>::ptr()
{
	return &x;
}

template<typename real>
real real2<real>::operator[]( int32 i ) const
{
	return ptr()[i];
}

template<typename real>
real& real2<real>::operator[]( int32 i )
{
	return ptr()[i];
}

// Setters
template<typename real>
void real2<real>::set( real scalar1, real scalar2 )
{
	x = scalar1;
	y = scalar2;
}

template<typename real>
void real2<real>::set( real const * const pair )
{
	x = pair[0];
	y = pair[1];
}

// Arithmetic operations
template<typename real>
real2<real> real2<real>::operator+( const real2<real>& other ) const
{
	return real2<real>( x + other.x, y + other.y );
}

template<typename real>
real2<real> real2<real>::operator-( const real2<real>& other ) const
{
	return real2<real>( x - other.x, y - other.y );
}

template<typename real>
real2<real> real2<real>::operator*( const real2<real>& other ) const
{
	return real2<real>( x * other.x, y * other.y );
}

template<typename real>
real2<real> real2<real>::operator*( real scalar ) const
{
	return real2<real>( x * scalar, y * scalar );
}

template<typename real>
real2<real> real2<real>::operator-() const
{
	return real2<real>( -x, -y );
}

// Arithmetic updates
template<typename real>
real2<real>& real2<real>::operator+=( const real2<real>& other )
{
	x += other.x;
	y += other.y;
	return *this;
}

template<typename real>
real2<real>& real2<real>::operator-=( const real2<real>& other )
{
	x -= other.x;
	y -= other.y;
	return *this;
}

template<typename real>
real2<real>& real2<real>::operator*=( const real2<real>& other )
{
	x *= other.x;
	y *= other.y;
	return *this;
}

template<typename real>
real2<real>& real2<real>::operator*=( real scalar )
{
	x *= scalar;
	y *= scalar;
	return *this;
}

// Vector operations
template<typename real>
real real2<real>::length() const
{
	return sqrt( length2() );
}

template<typename real>
real real2<real>::length2() const
{
	return x*x + y*y;
}

template<typename real>
real real2<real>::normalize()
{
	real len = length();
	if( len > math<real>::ZERO_TOLERANCE() )
	{
		real invLen = static_cast<real>(1.0) / len;
		x *= invLen;
		y *= invLen;
		return len;
	}
	else
	{
		x = 0;
		y = 0;
		return 0;
	}
}

template<typename real>
void real2<real>::tryNormalize()
{
	real len2 = length2();
	if( !math<real>::isEqual( len2, 1 ) )
	{
		real invLen = static_cast<real>(1.0) / sqrt( len2 );
		x *= invLen;
		y *= invLen;
	}
}

template<typename real>
real real2<real>::dot( const real2<real>& other ) const
{
	return x*other.x + y*other.y;
}

typedef real2<float>  float2;
typedef real2<double> double2;

} // namespace rtu

#endif
