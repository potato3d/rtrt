#pragma once
#ifndef _RTC_INSTANCE_H_
#define _RTC_INSTANCE_H_

#include <rtu/common.h>
#include <rtc/Transform.h>
#include <rtc/AABB.h>

namespace rtc {

struct Instance
{
	unsigned int geometryId;
	Transform    transform;
	AABB	     bbox;
};

} // namespace rtc

#endif // _RTC_INSTANCE_H_
