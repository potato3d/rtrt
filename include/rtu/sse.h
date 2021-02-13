#pragma once
#ifndef _RTU_SSE_H_
#define _RTU_SSE_H_

#include <rtu/common.h>
#include <rtu/math.h>
#include <emmintrin.h> // SSE & SSE2 intrinsics

namespace rtu {

// Constants
static const __m128  SSE_PLUS_INF  = _mm_set_ps1( -logf( 0.0f ) );
static const __m128  SSE_MINUS_INF = _mm_set_ps1( logf( 0.0f ) );
static const __m128  SSE_ZERO      = _mm_setzero_ps();
static const __m128  SSE_ONE       = _mm_set_ps1( 1.0f );
static const __m128  SSE_ALL_ON    = _mm_cmpeq_ps( SSE_ZERO, SSE_ZERO );
static const __m128i SSE_ZERO_INT   = _mm_set1_epi32( 0 );
static const __m128i SSE_ONE_INT   = _mm_set1_epi32( 1 );

// Helper constructs
struct floatSSE
{
	union
	{
		struct { float w, z, y, x; };
		float f[4]; // access in reverse order! i.e. f[3] equals x coordinate
		__m128 m;
	};
};

// Fast reciprocal using one Newton-Raphson iteration
// Watch out for 1.0 / 0.0 = +-INF! Would generate NaNs, if not for _mm_min_ps!
inline static __m128 sseFastRcp( __m128 v )
{
	const __m128 n = _mm_rcp_ps( v );
	return _mm_min_ps( _mm_sub_ps( _mm_add_ps( n, n ), _mm_mul_ps( _mm_mul_ps( v, n ), n ) ), rtu::SSE_PLUS_INF );
}

// Rotate left
inline static __m128 sseRotateLeft( __m128 v )
{
	return _mm_shuffle_ps( v, v, 0x39 ); // a,b,c,d -> b,c,d,a
}

} // namespace rtu

#endif // _RTU_SSE_H_
