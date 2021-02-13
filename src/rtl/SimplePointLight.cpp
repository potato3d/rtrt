#include <rtl/SimplePointLight.h>

namespace rtl {

SimplePointLight::SimplePointLight()
: _castShadows( true ), _intensity( 1.0f, 1.0f, 1.0f ), _position( 0.0f, 0.0f, 0.0f ), 
  _constAtten( 0.0f ), _linearAtten( 0.005f ), _quadAtten( 0.01f )
{
	// empty
}

bool SimplePointLight::illuminate( rts::RTstate& state )
{
	// Avoid back face lighting
	if( !rtsFrontFace( state ) )
		return false;

	// Surface hit point
	const rtu::float3& hitPos = rtsHitPosition( state );

	// Direction towards light
	const rtu::float3& L = _position - hitPos;

	// Setup shadow ray
	// Since we did not normalize the direction, every parametric t step walks the length of the direction along the ray.
	// So, when t == 1 we are right at the light position, which is the farthest we want to go.
	const bool ok = rtsInitShadowRayState( L, 1.0f, state );

	// Avoid computing light contribution for triangles facing away
	if( !ok )
		return false;

	// If light is occluded, we avoid computing its contribution
	if( _castShadows && rtsTraceHit( state ) )
		return false;

	// Quadratic distance attenuation
	const float distance = L.length(); // TODO: if it's slow, we can use only squared distance and attenuation
	const float attenFactor = 1.0f / ( _constAtten + _linearAtten * distance + _quadAtten * distance * distance );

	// Compute and return light intensity
	rtu::float3& I = rtsResultColor( state );
	I = _intensity * attenFactor;
	return true;
}

void SimplePointLight::setCastShadows( bool enabled )
{
	_castShadows = enabled;
}

void SimplePointLight::setIntensity( float x, float y, float z )
{
	_intensity.set( x, y, z );
}

void SimplePointLight::setPosition( float x, float y, float z )
{
	_position.set( x, y, z );
}

void SimplePointLight::setConstantAttenuation( float atten )
{
	_constAtten = atten;
}

void SimplePointLight::setLinearAttenuation( float atten )
{
	_linearAtten = atten;
}

void SimplePointLight::setQuadraticAttenuation( float atten )
{
	_quadAtten = atten;
}

} // namespace rtl
