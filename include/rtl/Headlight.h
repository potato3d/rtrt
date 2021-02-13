#pragma once
#ifndef _RTL_HEADLIGHT_H_
#define _RTL_HEADLIGHT_H_

#include <rts/IMaterial.h>

namespace rtl {

class Headlight : public rts::IMaterial
{
public:
	virtual void init();
	virtual void shade( rts::RTstate& state );

private:
	rtu::float3 _ambient;
};

} // namespace rtl

#endif // _RTL_HEADLIGHT_H_
