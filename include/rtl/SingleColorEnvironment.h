#pragma once
#ifndef _RTL_SINGLECOLORENVIRONMENT_H_
#define _RTL_SINGLECOLORENVIRONMENT_H_

#include <rts/IEnvironment.h>

namespace rtl {

class SingleColorEnvironment : public rts::IEnvironment
{
public:
	virtual void init();
	virtual void shade( rts::RTstate& state );

private:
	rtu::float3 _background;
};

} // namespace rtl

#endif // _RTL_SINGLECOLORENVIRONMENT_H_
