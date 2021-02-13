#pragma once
#ifndef _RTC_RAY_H_
#define _RTC_RAY_H_

#include <rtu/common.h>
#include <rtu/float3.h>
#include <rtu/sse.h>
#include <rt/definitions.h>

namespace rtc {

struct Ray
{
	// Pre-compute invDir and dirSigns, used in ray-box intersection and hierarchy traversal.
	// Must be called whenever the ray direction changes (i.e. ray is transformed)
	inline void update();

	rtu::float3 origin;
	rtu::float3 direction;
	float tnear;
	float tfar;

	// Pre-computations
	rtu::float3 invDir;
	unsigned int dirSigns[3];
};

inline void Ray::update()
{
	invDir.x = 1.0f / direction.x;
	invDir.y = 1.0f / direction.y;
	invDir.z = 1.0f / direction.z;
	dirSigns[0] = rtu::mathf::signBit( direction.x );
	dirSigns[1] = rtu::mathf::signBit( direction.y );
	dirSigns[2] = rtu::mathf::signBit( direction.z );
}

// TODO: may add additional members like arauna does
// TODO: CHECK ALIGNMENT OF SUBSECTIONS!
struct RayPacket
{
	// Pre-compute inverse ray directions, used in ray-box intersection and hierarchy traversal.
	// Updates ray direction masks and boolean coherence flag.
	// Must be called whenever the ray direction changes (i.e. ray is transformed)
	inline void preCompute();

	// Ray descriptors
	union
	{
		// Default accessors
		struct
		{
			// Ray origin
			union { float ox[RT_PACKET_SIZE]; __m128 ox4[RT_PACKET_SIMD_SIZE]; };
			union { float oy[RT_PACKET_SIZE]; __m128 oy4[RT_PACKET_SIMD_SIZE]; };
			union { float oz[RT_PACKET_SIZE]; __m128 oz4[RT_PACKET_SIMD_SIZE]; };

			// Ray direction
			union { float dx[RT_PACKET_SIZE]; __m128 dx4[RT_PACKET_SIMD_SIZE]; };
			union { float dy[RT_PACKET_SIZE]; __m128 dy4[RT_PACKET_SIMD_SIZE]; };
			union { float dz[RT_PACKET_SIZE]; __m128 dz4[RT_PACKET_SIMD_SIZE]; };
		};

		// Axis accessors
		struct
		{
			// Ray origin
			union { float oa[RT_PACKET_SIZE*3]; __m128 oa4[RT_PACKET_SIMD_SIZE*3]; };
			// Ray direction
			union { float da[RT_PACKET_SIZE*3]; __m128 da4[RT_PACKET_SIMD_SIZE*3]; };
		};
	};

	// Ray near
	union { float tnear[RT_PACKET_SIZE]; __m128 tnear4[RT_PACKET_SIMD_SIZE]; };

	// Ray far
	union { float tfar[RT_PACKET_SIZE]; __m128 tfar4[RT_PACKET_SIMD_SIZE]; };

	// Active rays mask
	union { unsigned int mask[RT_PACKET_SIZE]; __m128 mask4[RT_PACKET_SIMD_SIZE]; __m128i imask4[RT_PACKET_SIMD_SIZE]; };
	
	// Pre-computations
	union
	{
		// Default accessors
		struct  
		{
			union { float rdx[RT_PACKET_SIZE]; __m128 rdx4[RT_PACKET_SIMD_SIZE]; };
			union { float rdy[RT_PACKET_SIZE]; __m128 rdy4[RT_PACKET_SIMD_SIZE]; };
			union { float rdz[RT_PACKET_SIZE]; __m128 rdz4[RT_PACKET_SIMD_SIZE]; };
		};

		// Axis accessors
		union { float rda[RT_PACKET_SIZE*3]; __m128 rda4[RT_PACKET_SIMD_SIZE*3]; };
	};

	// Ray direction masks to check coherence
	unsigned int xmask, ymask, zmask;
	bool isCoherent;
};

inline void RayPacket::preCompute()
{
	for( unsigned int r = 0; r < RT_PACKET_SIMD_SIZE; ++r )
	{
		// Reciprocal directions
		rdx4[r] = rtu::sseFastRcp( dx4[r] );
		rdy4[r] = rtu::sseFastRcp( dy4[r] );
		rdz4[r] = rtu::sseFastRcp( dz4[r] );
	}

	// Fill masks with all bits of all ray direction signs
	xmask = _mm_movemask_ps( dx4[0] ) + 
		   (_mm_movemask_ps( dx4[1] ) << 4) +
		   (_mm_movemask_ps( dx4[2] ) << 8) + 
		   (_mm_movemask_ps( dx4[3] ) << 12);

	ymask = _mm_movemask_ps( dy4[0] ) + 
		   (_mm_movemask_ps( dy4[1] ) << 4) +
		   (_mm_movemask_ps( dy4[2] ) << 8) + 
		   (_mm_movemask_ps( dy4[3] ) << 12);

	zmask = _mm_movemask_ps( dz4[0] ) + 
		   (_mm_movemask_ps( dz4[1] ) << 4) +
		   (_mm_movemask_ps( dz4[2] ) << 8) + 
		   (_mm_movemask_ps( dz4[3] ) << 12);

	// If all bits are zero or one, all ray directions have same sign and the ray packet is coherent!
	isCoherent = (((xmask == 0)||(xmask == 65535)) && 
		          ((ymask == 0)||(ymask == 65535)) &&
		          ((zmask == 0)||(zmask == 65535)));
}

} // namespace rtc

#endif // RT_RAY_H
