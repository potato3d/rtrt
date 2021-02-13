#pragma once
#ifndef _RTC_RAYSTATE_H_
#define _RTC_RAYSTATE_H_

#include <rtu/common.h>
#include <rtc/Ray.h>
#include <rtc/Hit.h>

namespace rtc {

struct RayState
{
	Ray ray;
	Hit hit;
	rtu::float3 resultColor;
	unsigned int recursionDepth;

	// Computable attributes by the Shader Programming Interface.
	// These are necessary for inter-shader communication and several rts functions.
	rtu::float3 hitPosition;
	rtu::float3 shadingNormal;
};

// TODO: SoA for other members! check everything!
RTU_CACHE_ALIGN( 16 )
struct RayPacketState
{
	RayPacket packet;
	HitPacket hit;
	rtu::float3 resultColor[RT_PACKET_SIZE];
	unsigned int recursionDepth[RT_PACKET_SIZE];

	// Computable attributes by the Shader Programming Interface.
	// These are necessary for inter-shader communication and several rts functions.
	rtu::float3 hitPosition; // TODO: parallelize
	rtu::float3 shadingNormal; // TODO: parallelize
};

} // namespace rtc

#endif // _RTC_RAYSTATE_H_
