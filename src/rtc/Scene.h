#pragma once
#ifndef _RTC_SCENE_H_
#define _RTC_SCENE_H_

#include <rtu/common.h>
#include <rtc/KdTree.h>
#include <rtc/Instance.h>
#include <rtc/Geometry.h>
#include <vector>

namespace rtc {

struct Scene
{
	static KdTree instanceTree;
	static std::vector<Instance> instances;
	static std::vector<Geometry> geometries;
	static float* frameBuffer;
	static float rayEpsilon;
	static unsigned int maxRayRecursionDepth;
	static float mediumRefractionIndex;
};

} // namespace rtc

#endif // _RTC_SCENE_H_
