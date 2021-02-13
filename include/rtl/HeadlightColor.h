#pragma once
#ifndef _RTL_HEADLIGHTCOLOR_H_
#define _RTL_HEADLIGHTCOLOR_H_

#include <rts/IMaterial.h>

namespace rtl {

class HeadlightColor : public rts::IMaterial
{
public:
	virtual void init();
	virtual void shade( rts::RTstate& state );

private:
	rtu::float3 _ambient;
	rtu::float3 _diffuse;
};

} // namespace rtl

#endif // _RTL_HEADLIGHTCOLOR_H_
