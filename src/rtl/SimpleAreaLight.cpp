#include <rtl/SimpleAreaLight.h>
#include <rtu/random.h>

namespace rtl {

SimpleAreaLight::SimpleAreaLight()
: SimplePointLight(), _radius( 1.0f ), _sampleCount( 4 )
{
	// empty
}
void SimpleAreaLight::init()
{
	rtu::Random::autoSeed();
}

bool SimpleAreaLight::illuminate( rts::RTstate& state )
{
	// Avoid back face lighting
	if( !rtsFrontFace( state ) )
		return false;

	// Surface hit point
	const rtu::float3& hitPos = rtsHitPosition( state );

	// Direction towards light
	const rtu::float3& L = _position - hitPos;

	// Assume a disk perpendicular to direction using _radius and compute _sampleCount samples randomly inside it
	rtu::float3 uAxis;
	rtu::float3 vAxis;
	rtu::float3 samplePos;
	rtu::float3 dir = L;
	dir.normalize();
	dir.orthonormalBasis( uAxis, vAxis );
	float x;
	float y;
	unsigned int successfulSamples = 0;

	for( unsigned int i = 0; i < _sampleCount; ++i )
	{
		randomDisk( x, y );
		x *= _radius;
		y *= _radius;
		samplePos = L + ( uAxis * x ) + ( vAxis * y );

		// Setup shadow ray
		// Since we did not normalize the direction, every parametric t step walks the length of the direction along the ray.
		// So, when t == 1 we are right at the light position, which is the farthest we want to go.
		const bool ok = rtsInitShadowRayState( samplePos - hitPos, 1.0f, state );

		// Avoid computing light contribution for triangles facing away
		if( !ok )
			continue;

		// If light sample is occluded, we avoid computing its contribution
		if( _castShadows && rtsTraceHit( state ) )
			continue;

		++successfulSamples;
	}

	// If no samples hit light
	if( successfulSamples == 0 )
		return false;

	// Quadratic distance attenuation
	const float distance = L.length(); // TODO: if it's slow, we can use only squared distance and attenuation
	const float attenFactor = 1.0f / ( _constAtten + _linearAtten * distance + _quadAtten * distance * distance );

	// Compute and return light intensity
	rtu::float3& I = rtsResultColor( state );
	I = _intensity * attenFactor * ( (float)successfulSamples / (float)_sampleCount );

	// Store original direction to light for shading computations
	rtsRayDirection( state ) = L;

	return true;
}

void SimpleAreaLight::randomDisk( float& x, float& y )
{
	const float r = sqrt( rtu::Random::realInIn() );
	const float theta = rtu::mathf::TWO_PI*rtu::Random::realInIn();

	x = r*cos( theta );
	y = r*sin( theta );
}


} // namespace rtl
