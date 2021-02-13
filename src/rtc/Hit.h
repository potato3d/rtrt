#pragma once
#ifndef _RTC_HIT_H_
#define _RTC_HIT_H_

#include <rtu/common.h>
#include <rtc/Instance.h>
#include <rtc/Geometry.h>

namespace rtc {

struct Hit
{
	unsigned int triangleId;
	float v0Coord;
	float v1Coord;
	float v2Coord;
	float distance;
	const Instance* instance;
	const Geometry* geometry;
};

struct HitPacket
{
	union { unsigned int tId[RT_PACKET_SIZE]; __m128i tId4[RT_PACKET_SIMD_SIZE]; };
	union { float v0c[RT_PACKET_SIZE]; __m128 v0c4[RT_PACKET_SIMD_SIZE]; };
	union { float v1c[RT_PACKET_SIZE]; __m128 v1c4[RT_PACKET_SIMD_SIZE]; };
	union { float v2c[RT_PACKET_SIZE]; __m128 v2c4[RT_PACKET_SIMD_SIZE]; };
	union { float dist[RT_PACKET_SIZE]; __m128 dist4[RT_PACKET_SIMD_SIZE]; };
	union { const Instance* inst[RT_PACKET_SIZE]; __m128i inst4[RT_PACKET_SIMD_SIZE]; __m128 finst4[RT_PACKET_SIMD_SIZE]; };
	union { const Geometry* geom[RT_PACKET_SIZE]; __m128i geom4[RT_PACKET_SIMD_SIZE]; __m128 fgeom4[RT_PACKET_SIMD_SIZE]; };
};

} // namespace rtc

#endif // _RTC_HIT_H_
