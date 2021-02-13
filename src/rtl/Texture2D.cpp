#include <rtl/Texture2D.h>
#include <algorithm>

namespace rtl {

Texture2D::Texture2D()
: _texels( NULL )
{
	// empty
}

void Texture2D::receiveParameter( int paramId, void* paramValue )
{
#pragma warning( disable : 4311 )
	switch( paramId )
	{
	case RT_TEXTURE_FILTER:
		_filter = reinterpret_cast<unsigned int>( paramValue );
		break;

	case RT_TEXTURE_WRAP_S:
		_wrapS = reinterpret_cast<unsigned int>( paramValue );
		break;

	case RT_TEXTURE_WRAP_T:
		_wrapT = reinterpret_cast<unsigned int>( paramValue );
		break;

	case RT_TEXTURE_ENV_MODE:
		_envMode = reinterpret_cast<unsigned int>( paramValue );
		break;

	default:
	    break;
	}
#pragma warning( default : 4311 )
}

void Texture2D::textureImage2D( unsigned int width, unsigned int height, unsigned char* texels )
{
	_width = width;
	_height = height;
	if( _texels )
		delete [] _texels;

	_texels = new unsigned char[width*height*3]; // hard-coded RGB format

	std::copy( texels, texels + width*height*3, _texels );
}

void Texture2D::shade( rts::RTstate& state )
{
	// Compute texture coordinates from interpolated vertex attributes.
	// Instead, we could use different texture mapping algorithms to automatically generate texture coordinates.
	// Ex: planar, sphere, cylinder, etc
	rtu::float3 coords;
	rtsComputeTextureCoords( state, coords );

	// Just to be safe
	// TODO: can be removed?
	if( ( coords.x < 0.0f ) || ( coords.y < 0.0f ) )
		return;

	// Compute correct texture coordinates given wrap modes
	switch( _wrapS )
	{
	case RT_REPEAT:
		coords.x = rtu::mathf::frac( coords.x );
		break;

	case RT_CLAMP:
		coords.x = rtu::mathf::max( rtu::mathf::min( coords.x, 1.0f ), 0.0f );
		break;

	default:
		break;
	}

	switch( _wrapT )
	{
	case RT_REPEAT:
		coords.y = rtu::mathf::frac( coords.y );
		break;

	case RT_CLAMP:
		coords.y = rtu::mathf::max( rtu::mathf::min( coords.y, 1.0f ), 0.0f );
		break;

	default:
		break;
	}
	
	// Final texel color
	rtu::float3 texel;

	// Compute texel color from correct texture coordinates
	switch( _filter )
	{
	case RT_NEAREST:
		{
			// TODO: optimize
			int s = (int)rtu::mathf::round( coords.x * ( _width - 1 ) );
			int t = (int)rtu::mathf::round( coords.y * ( _height - 1 ) );
			unsigned char* imgTexel = _texels + ( s + t*_width )*3;
			texel.set( (float)imgTexel[0]/255.0f, (float)imgTexel[1]/255.0f, (float)imgTexel[2]/255.0f );
			break;
		}

	case RT_LINEAR:
		{
			// TODO: convert to unsigned char
			// Get 4 corner pixels and lerp between them
			//int u0 = (int)coords.x;
			//int v0 = (int)coords.y;
			//int u1 = u0 + 1;
			//int v1 = v0 + 1;

			//float ds = rtu::mathf::frac( coords.x * _width );
			//float dt = rtu::mathf::frac( coords.y * _height );

			//rtu::float3 lowerLeft(  _texels + u0 + v0*_width );
			//rtu::float3 lowerRight( _texels + u1 + v0*_width );
			//rtu::float3 upperLeft(  _texels + u0 + v1*_width );
			//rtu::float3 upperRight( _texels + u1 + v1*_width );

			//rtu::float3 lowerInterp( lowerLeft * ( 1.0f - ds ) + lowerRight * ds );
			//rtu::float3 upperInterp( upperLeft * ( 1.0f - ds ) + upperRight * ds );
			//texel = lowerInterp * ( 1.0f - dt ) + upperInterp * dt;
			break;
		}

	default:
		break;
	}

	// Finally, apply texture color to given state color
	switch( _envMode )
	{
	case RT_MODULATE:
		rtsResultColor( state ) *= texel;
		break;

	case RT_REPLACE:
		rtsResultColor( state ) = texel;
		break;

	default:
		break;
	}
}

} // namespace rtl
