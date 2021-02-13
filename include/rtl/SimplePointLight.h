#pragma once
#ifndef _RTL_SIMPLEPOINTLIGHT_H_
#define _RTL_SIMPLEPOINTLIGHT_H_

#include <rts/ILight.h>

namespace rtl {

class SimplePointLight : public rts::ILight
{
public:
	SimplePointLight();

	virtual bool illuminate( rts::RTstate& state );

	void setCastShadows( bool enabled );
	void setIntensity( float x, float y, float z );
	void setPosition( float x, float y, float z );
	void setConstantAttenuation( float atten );
	void setLinearAttenuation( float atten );
	void setQuadraticAttenuation( float atten );

protected:
	bool _castShadows;
	rtu::float3 _intensity;
	rtu::float3 _position;
	float _constAtten;
	float _linearAtten;
	float _quadAtten;
};

} // namespace rtl

#endif // _RTL_SIMPLEPOINTLIGHT_H_
