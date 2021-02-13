#pragma once
#ifndef _RTC_TRIANGLE_H_
#define _RTC_TRIANGLE_H_

#include <rtu/common.h>
#include <rtu/float3.h>

namespace rtc {

struct TriDesc
{
	unsigned int v0;
	unsigned int v1;
	unsigned int v2;
	unsigned int materialId;
};

struct TriAccel
{
	void buildFrom( const rtu::float3& v0, const rtu::float3& v1, const rtu::float3& v2 );
	bool valid();

	// first 16 byte half cache line
	unsigned int k; // projection dimension
	// plane:
	float n_u; // normal.u / normal.k
	float n_v; // normal.v / normal.k
	float n_d; // constant of plane equation

	// second 16 byte half cache line
	// line equation for line ac
	float b_nu;
	float b_nv;
	float b_d;
	unsigned int pad; // pad to next cache line

	// third 16 byte half cache line
	// line equation for line ab
	float c_nu;
	float c_nv;
	float c_d;
	unsigned int triangleId; // pad to 48 bytes for cache alignment purposes
};

} // namespace rtc

#endif // _RTC_TRIANGLE_H_
