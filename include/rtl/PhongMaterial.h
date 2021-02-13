#pragma once
#ifndef _RTL_PHONGMATERIAL_H_
#define _RTL_PHONGMATERIAL_H_

#include <rts/IMaterial.h>

namespace rtl {

class PhongMaterial : public rts::IMaterial
{
public:
	PhongMaterial();

	virtual void receiveParameter( int paramId, void* paramValue );
	virtual void shade( rts::RTstate& state );

	void setReflexCoeff( float coeff );
	void setOpacity( float opacity );
	void setRefractionIndex( float index );

private:
	rtu::float3 _ambient;
	rtu::float3 _specularColor;
	float _specularExponent;
	float _reflexCoeff;
	float _refractionIndex;
	float _opacity;
	unsigned int _textureId;
};

} // namespace rtl

#endif // _RTL_PHONGMATERIAL_H_
