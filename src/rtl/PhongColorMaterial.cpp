#include <rtl/PhongColorMaterial.h>

namespace rtl {

PhongColorMaterial::PhongColorMaterial()
{
	_ambient.set( 0.1f, 0.1f, 0.1f );
	_diffuse.set( 1.0f, 1.0f, 1.0f );
	_specularColor.set( 1.0f, 1.0f, 1.0f );
	_specularExponent = 32.0f;
	_reflexCoeff = 0.0f;
	_refractionIndex = 1.0f;
	_opacity = 1.0f;
	_textureId = 0;
}

void PhongColorMaterial::receiveParameter( int paramId, void* paramValue )
{
	switch( paramId )
	{
	case RT_TEXTURE_ID:
		_textureId = *reinterpret_cast<unsigned int*>( paramValue );
		break;

	default:
	    break;
	}
}

void PhongColorMaterial::shade( rts::RTstate& state )
{
	// Create new state for light samples
	rts::RTstate lightSample;

	// Get data from current state
	const rtu::float3& normal = rtsComputeShadingNormal( state );
	rtsComputeHitPosition( state );
	rtu::float3& returnColor = rtsResultColor( state );
	rtu::float3 specularVector;
	rtsComputeSpecularVector( state, specularVector );

	// Query light sources
	void** lights;
	const int lightCount = rtsGlobalLights( lights );

	// Accumulate light contributions
	rtu::float3 diffuse( 0.0f, 0.0f, 0.0f );
	rtu::float3 specular( 0.0f, 0.0f, 0.0f );

	for( int i = 0; i < lightCount; ++i )
	{
		// Setup light state
		rtsInitLightState( state, lightSample );

		// Update lightSample with light radiance and direction
		bool ok = rtsIlluminate( lightSample, lights[i] );

		// Probably light is occluded or points away from hit point
		if( !ok )
			continue;

		// Query light sample direction and radiance
		const rtu::float3& lightIntensity = rtsResultColor( lightSample );
		rtu::float3& L = rtsRayDirection( lightSample );
		L.normalize();

		// Diffuse shading
		const float nDotL = normal.dot( L );
		diffuse.r += lightIntensity.r * nDotL;
		diffuse.g += lightIntensity.g * nDotL;
		diffuse.b += lightIntensity.b * nDotL;

		// Specular shading
		const float specDotL = specularVector.dot( L );
		if( specDotL > 0.0f )
			specular += lightIntensity * pow( specDotL, _specularExponent );
	}

	// Gather all lighting contributions
	returnColor.r = _diffuse.r * ( _ambient.r + diffuse.r ) + ( _specularColor.r * specular.r );
	returnColor.g = _diffuse.g * ( _ambient.g + diffuse.g ) + ( _specularColor.g * specular.g );
	returnColor.b = _diffuse.b * ( _ambient.b + diffuse.b ) + ( _specularColor.b * specular.b );

	// Apply texture
	if( _textureId > 0 )
		rtsApplyTexture( state, _textureId );

	// Avoid infinite recursion
	if( rtsStopRayRecursion( state ) )
		return;

	// Compute reflection contribution
	if( _reflexCoeff > 0.0f )
	{
		rts::RTstate secondarySample;
		rtsInitReflectionRayState( state, secondarySample );

		// Trace reflection ray and add contribution
		rtsTraceRay( secondarySample );
		returnColor += rtsResultColor( secondarySample ) * _reflexCoeff;
	}

	// Compute refraction contribution
	if( _opacity < 1.0f )
	{
		rts::RTstate refractionSample;
		const bool haveRefraction = rtsInitRefractionRayState( state, _refractionIndex, refractionSample );

		// Check for total internal reflection
		if( haveRefraction )
		{
			// Trace refraction ray and add contribution
			rtsTraceRay( refractionSample );
			returnColor += rtsResultColor( refractionSample ) * ( 1.0f - _opacity );
		}
	}
}

void PhongColorMaterial::setAmbient( float r, float g, float b )
{
	_ambient.set( r, g, b );
}

void PhongColorMaterial::setDiffuse( float r, float g, float b )
{
	_diffuse.set( r, g, b );
}

void PhongColorMaterial::setReflexCoeff( float coeff )
{
	_reflexCoeff = coeff;
}

void PhongColorMaterial::setOpacity( float opacity )
{
	_opacity = opacity;
}

void PhongColorMaterial::setRefractionIndex( float index )
{
	_refractionIndex = index;
}

} // namespace rtl
