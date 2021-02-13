#pragma once
#ifndef _RTC_SPLITPLANE_H_
#define _RTC_SPLITPLANE_H_

#include <rtu/common.h>

namespace rtc {

struct SplitPlane
{
	inline bool operator==( const SplitPlane& other ) const;

	// 0: x axis
	// 1: y axis
	// 2: z axis
	unsigned int axis;
	float position;
};

inline bool SplitPlane::operator==( const SplitPlane& other ) const
{
	return ( axis == other.axis ) && ( position == other.position );
}


} // namespace rtc

#endif // _RTC_SPLITPLANE_H_
