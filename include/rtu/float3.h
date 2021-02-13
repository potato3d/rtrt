#pragma once
#ifndef _RTU_FLOAT3_H_
#define _RTU_FLOAT3_H_

#include <rtu/common.h>
#include <rtu/math.h>

namespace rtu {

template<typename real>
class real3
{
public:
	// Public members
	union
	{
		struct { real x; real y; real z; };
		struct { real r; real g; real b; };
	};

	// Constants
	inline static real3<real> UNIT_X();  // ( 1, 0, 0 )
	inline static real3<real> UNIT_Y();  // ( 0, 1, 0 )
	inline static real3<real> UNIT_Z();  // ( 0, 0, 1 )

	// Construction
	inline real3(); // Uninitialized
	inline real3( real scalar1, real scalar2, real scalar3 );
	inline explicit real3( real const * const tuple );

	// Getters
	inline real* ptr();
	inline const real* ptr() const;
	inline real  operator[]( int32 i ) const;
	inline real& operator[]( int32 i );

	// Setters
	inline void set( real scalar1, real scalar2, real scalar3 );
	inline void set( real const * const tuple );

	// Arithmetic operations
	inline real3<real> operator+( const real3<real>& other ) const;
	inline real3<real> operator-( const real3<real>& other ) const;
	inline real3<real> operator*( const real3<real>& other ) const; // Componentwise
	inline real3<real> operator*( real scalar ) const;
	inline real3<real> operator-() const;

	// Arithmetic updates
	inline real3<real>& operator+=( const real3<real>& other );
	inline real3<real>& operator-=( const real3<real>& other );
	inline real3<real>& operator*=( const real3<real>& other ); // Componentwise
	inline real3<real>& operator*=( real scalar );

	// Vector operations
	inline real length() const;
	inline real length2() const;
	inline real normalize();

	inline real dot( const real3<real>& other ) const;
	inline real3<real> cross( const real3<real>& other ) const;

	// Normalize this first!
	inline void orthonormalBasis( real3<real>& u, real3<real>& v );
};

// Constants
template<typename real>
real3<real> real3<real>::UNIT_X()
{
	return real3<real>( 1, 0, 0 );
}

template<typename real>
real3<real> real3<real>::UNIT_Y()
{
	return real3<real>( 0, 1, 0 );
}

template<typename real>
real3<real> real3<real>::UNIT_Z()
{
	return real3<real>( 0, 0, 1 );
}

// Construction
template<typename real>
real3<real>::real3()
{
	// Do not initialize for performance!
}

template<typename real>
real3<real>::real3( real scalar1, real scalar2, real scalar3 )
	: x( scalar1 ), y( scalar2 ), z( scalar3 )
{
}

template<typename real>
real3<real>::real3( real const * const tuple )
	: x( tuple[0] ), y( tuple[1] ), z( tuple[2] )
{
}

// Getters
template<typename real>
real* real3<real>::ptr()
{
	return &x;
}

template<typename real>
const real* real3<real>::ptr() const
{
	return &x;
}

template<typename real>
real real3<real>::operator[]( int32 i ) const
{
	return ptr()[i];
}

template<typename real>
real& real3<real>::operator[]( int32 i )
{
	return ptr()[i];
}

// Setters
template<typename real>
void real3<real>::set( real scalar1, real scalar2, real scalar3 )
{
	x = scalar1;
	y = scalar2;
	z = scalar3;
}

template<typename real>
void real3<real>::set( real const * const tuple )
{
	x = tuple[0];
	y = tuple[1];
	z = tuple[2];
}

// Arithmetic operations
template<typename real>
real3<real> real3<real>::operator+( const real3<real>& other ) const
{
	return real3<real>( x + other.x, y + other.y, z + other.z );
}

template<typename real>
real3<real> real3<real>::operator-( const real3<real>& other ) const
{
	return real3<real>( x - other.x, y - other.y, z - other.z );
}

template<typename real>
real3<real> real3<real>::operator*( const real3<real>& other ) const
{
	return real3<real>( x * other.x, y * other.y, z * other.z );
}

template<typename real>
real3<real> real3<real>::operator*( real scalar ) const
{
	return real3<real>( x * scalar, y * scalar, z * scalar );
}

template<typename real>
real3<real> real3<real>::operator-() const
{
	return real3<real>( -x, -y, -z );
}

// Arithmetic updates
template<typename real>
real3<real>& real3<real>::operator+=( const real3<real>& other )
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

template<typename real>
real3<real>& real3<real>::operator-=( const real3<real>& other )
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

template<typename real>
real3<real>& real3<real>::operator*=( const real3<real>& other )
{
	x *= other.x;
	y *= other.y;
	z *= other.z;
	return *this;
}

template<typename real>
real3<real>& real3<real>::operator*=( real scalar )
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

// Vector operations
template<typename real>
real real3<real>::length() const
{
	return sqrt( length2() );
}

template<typename real>
real real3<real>::length2() const
{
	return x*x + y*y + z*z;
}

template<typename real>
real real3<real>::normalize()
{
	real len = length();
	if( len > math<real>::ZERO_TOLERANCE )
	{
		real invLen = static_cast<real>(1.0) / len;
		x *= invLen;
		y *= invLen;
		z *= invLen;
		return len;
	}
	else
	{
		x = 0;
		y = 0;
		z = 0;
		return 0;
	}
}

template<typename real>
real real3<real>::dot( const real3<real>& other ) const
{
	return x*other.x + y*other.y + z*other.z;
}

template<typename real>
real3<real> real3<real>::cross( const real3<real>& other ) const
{
	return real3<real>( y*other.z - z*other.y,
					    z*other.x - x*other.z,
					    x*other.y - y*other.x );
}

// Normalize this first!
template<typename real>
void rtu::real3<real>::orthonormalBasis( real3<real>& u, real3<real>& v )
{
	if( mathf::abs( x ) >= mathf::abs( y ) )
	{
		// W.x or W.z is the largest magnitude component, swap them
		const real invLength = 1.0f / sqrt( x*x + z*z );
		u[0] = -z*invLength;
		u[1] = (real)0.0;
		u[2] = +x*invLength;
		v[0] = y*u[2];
		v[1] = z*u[0] - x*u[2];
		v[2] = -y*u[0];
	}
	else
	{
		// W.y or W.z is the largest magnitude component, swap them
		const real invLength = 1.0f / sqrt( y*y + z*z );
		u[0] = (real)0.0;
		u[1] = +z*invLength;
		u[2] = -y*invLength;
		v[0] = y*u[2] - z*u[1];
		v[1] = -x*u[2];
		v[2] = x*u[1];
	}
}

typedef real3<float>  float3;
typedef real3<double> double3;

} // namespace rtu

#endif
