#pragma once
#ifndef _RTU_FLOAT4_H_
#define _RTU_FLOAT4_H_

#include <rtu/common.h>
#include <rtu/math.h>

namespace rtu {

template<typename real>
class real4
{
public:
	// Public members
	union
	{
		struct { real x; real y; real z; real w; };
		struct { real r; real g; real b; real a; };
	};

	// Constants
	inline static real4<real> RED();   // ( 1, 0, 0, 1 )
	inline static real4<real> GREEN(); // ( 0, 1, 0, 1 )
	inline static real4<real> BLUE();  // ( 0, 0, 1, 1 )

	// Construction
	inline real4(); // Uninitialized
	inline real4( real scalar1, real scalar2, real scalar3, real scalar4 );
	inline explicit real4( real const * const quad );

	// Getters
	inline real* ptr();
	inline real  operator[]( int32 i ) const;
	inline real& operator[]( int32 i );

	// Setters
	inline void set( real scalar1, real scalar2, real scalar3, real scalar4 );
	inline void set( real const * const quad );

	// Arithmetic operations
	inline real4<real> operator+( const real4<real>& other ) const;
	inline real4<real> operator-( const real4<real>& other ) const;
	inline real4<real> operator*( const real4<real>& other ) const; // Componentwise
	inline real4<real> operator*( real scalar ) const;
	inline real4<real> operator-() const;

	// Arithmetic updates
	inline real4<real>& operator+=( const real4<real>& other );
	inline real4<real>& operator-=( const real4<real>& other );
	inline real4<real>& operator*=( const real4<real>& other ); // Componentwise
	inline real4<real>& operator*=( real scalar );

	// Vector operations
	inline real length() const;
	inline real length2() const;

	// Used when previous length is needed
	inline real normalize();

	// Only normalizes if needed, saves unnecessary sqrt
	inline void tryNormalize();

	inline real dot( const real4<real>& other ) const;
};

// Constants
template<typename real>
real4<real> real4<real>::RED()
{
	return real4<real>( 1, 0, 0, 1 );
}

template<typename real>
real4<real> real4<real>::GREEN()
{
	return real4<real>( 0, 1, 0, 1 );
}

template<typename real>
real4<real> real4<real>::BLUE()
{
	return real4<real>( 0, 0, 1, 1 );
}

// Construction
template<typename real>
real4<real>::real4()
{
	// Do not initialize for performance!
}

template<typename real>
real4<real>::real4( real scalar1, real scalar2, real scalar3, real scalar4 )
	: x( scalar1 ), y( scalar2 ), z( scalar3 ), w( scalar4 )
{
}

template<typename real>
real4<real>::real4( real const * const quad )
	: x( quad[0] ), y( quad[1] ), z( quad[2] ), w( quad[3] )
{
}

// Getters
template<typename real>
real* real4<real>::ptr()
{
	return &x;
}

template<typename real>
real real4<real>::operator[]( int32 i ) const
{
	return ptr()[i];
}

template<typename real>
real& real4<real>::operator[]( int32 i )
{
	return ptr()[i];
}

// Setters
template<typename real>
void real4<real>::set( real scalar1, real scalar2, real scalar3, real scalar4 )
{
	x = scalar1;
	y = scalar2;
	z = scalar3;
	w = scalar4;
}

template<typename real>
void real4<real>::set( real const * const quad )
{
	x = quad[0];
	y = quad[1];
	z = quad[2];
	w = quad[3];
}

// Arithmetic operations
template<typename real>
real4<real> real4<real>::operator+( const real4<real>& other ) const
{
	return real4<real>( x + other.x, y + other.y, z + other.z, w + other.w );
}

template<typename real>
real4<real> real4<real>::operator-( const real4<real>& other ) const
{
	return real4<real>( x - other.x, y - other.y, z - other.z, w - other.w );
}

template<typename real>
real4<real> real4<real>::operator*( const real4<real>& other ) const
{
	return real4<real>( x * other.x, y * other.y, z * other.z, w * other.w );
}

template<typename real>
real4<real> real4<real>::operator*( real scalar ) const
{
	return real4<real>( x * scalar, y * scalar, z * scalar, w * scalar );
}

template<typename real>
real4<real> real4<real>::operator-() const
{
	return real4<real>( -x, -y, -z, -w );
}

// Arithmetic updates
template<typename real>
real4<real>& real4<real>::operator+=( const real4<real>& other )
{
	x += other.x;
	y += other.y;
	z += other.z;
	w += other.w;
	return *this;
}

template<typename real>
real4<real>& real4<real>::operator-=( const real4<real>& other )
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	w -= other.w;
	return *this;
}

template<typename real>
real4<real>& real4<real>::operator*=( const real4<real>& other )
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
	w *= other.w;
	return *this;
}

template<typename real>
real4<real>& real4<real>::operator*=( real scalar )
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
	return *this;
}

// Vector operations
template<typename real>
real real4<real>::length() const
{
	return sqrt( length2() );
}

template<typename real>
real real4<real>::length2() const
{
	return x*x + y*y + z*z + w*w;
}

template<typename real>
real real4<real>::normalize()
{
	real len = length();
	if( len > math<real>::ZERO_TOLERANCE() )
	{
		real invLen = static_cast<real>(1.0) / len;
		x *= invLen;
		y *= invLen;
		z *= invLen;
		w *= invLen;
		return len;
	}
	else
	{
		x = 0;
		y = 0;
		z = 0;
		w = 0;
		return 0;
	}
}

template<typename real>
void real4<real>::tryNormalize()
{
	real len2 = length2();
	if( !math<real>::isEqual( len2, 1 ) )
	{
		real invLen = static_cast<real>(1.0) / sqrt( len2 );
		x *= invLen;
		y *= invLen;
		z *= invLen;
		w *= invLen;
	}
}

template<typename real>
real real4<real>::dot( const real4<real>& other ) const
{
	return x*other.x + y*other.y + z*other.z + w*other.w;
}

typedef real4<float>  float4;
typedef real4<double> double4;

}  // namespace rtu

#endif
