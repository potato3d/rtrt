#pragma once
#ifndef _RTC_GEOMETRY_H_
#define _RTC_GEOMETRY_H_

#include <rtu/common.h>
#include <rtc/KdTree.h>
#include <rtc/Triangle.h>
#include <vector>

namespace rtc {

struct Geometry
{
	KdTree kdTree;
	std::vector<TriAccel> triAccel;
	std::vector<TriDesc>  triDesc;
	std::vector<rtu::float3> vertices;
	std::vector<rtu::float3> normals;
	std::vector<rtu::float3> colors;
	std::vector<rtu::float3> texCoords;
};

} // namespace rtc

#endif // _RTC_GEOMETRY_H_
