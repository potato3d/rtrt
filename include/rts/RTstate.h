#pragma once
#ifndef _RTS_RTSTATE_H_
#define _RTS_RTSTATE_H_

#include <rtu/common.h>

namespace rts {

// TODO: check size with RayPacketState!

// Dummy ray state container
// Hides actual implementation from plugins
// Size must be greater than or equal to actual RayState/RayPacketState sizes
RTU_CACHE_ALIGN( 16 )
struct RTstate
{
private:
	char filler[2048];
};

} // namespace rts

#endif // _RTS_RTSTATE_H_
