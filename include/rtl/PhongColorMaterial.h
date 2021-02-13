#pragma once
#ifndef _RTL_PHONGCOLORMATERIAL_H_
#define _RTL_PHONGCOLORMATERIAL_H_

#include <rts/IMaterial.h>

namespace rtl {

class PhongColorMaterial : public rts::IMaterial
{
public:
	PhongColorMaterial();

	virtual void receiveParameter( int paramId, void* paramValue );
	virtual void shade( rts::RTstate& state );

	void setAmbient( float r, float g, float b );
	void setDiffuse( float r, float g, float b );
	void setReflexCoeff( float coeff );
	void setOpacity( float opacity );
	void setRefractionIndex( float index );

private:
	rtu::float3 _ambient;
	rtu::float3 _diffuse;
	rtu::float3 _specularColor;
	float _specularExponent;
	float _reflexCoeff;
	float _refractionIndex;
	float _opacity;
	unsigned int _textureId;
};

} // namespace rtl

#endif // _RTL_PHONGCOLORMATERIAL_H_
