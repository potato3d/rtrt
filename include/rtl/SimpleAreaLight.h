#pragma once
#ifndef _RTL_SIMPLEAREALIGHT_H_
#define _RTL_SIMPLEAREALIGHT_H_

#include <rtl/SimplePointLight.h>

namespace rtl {

class SimpleAreaLight : public SimplePointLight
{
public:
	SimpleAreaLight();

	virtual void init();
	virtual bool illuminate( rts::RTstate& state );

private:
	void randomDisk( float& x, float& y );

	float _radius;
	float _area;
	unsigned int _sampleCount;
};

} // namespace rtl

#endif // _RTL_SIMPLEAREALIGHT_H_
