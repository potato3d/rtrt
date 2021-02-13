#pragma once
#ifndef _RTL_DEPTHMATERIAL_H_
#define _RTL_DEPTHMATERIAL_H_

#include <rts/IMaterial.h>

namespace rtl {

class DepthMaterial: public rts::IMaterial
{
public:
	virtual void shade( rts::RTstate& state );
};

} // namespace rtl

#endif // _RTL_HEADLIGHT_H_
