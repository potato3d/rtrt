#include <rtl/SingleRenderer.h>

namespace rtl {

// TODO: ganhamos 2 fps em 1024x1024 (athlon xp 3800+, 2gb ram 400, 7900gtx 512, nessa traulheteira
/*
	static rtu::float3 _position( 0.0f, 0.0f, 0.0f );
	static rtu::float3 _baseDir( 0.0f, 0.0f, -4.0f );
	static rtu::float3 _nearU( 1.0f, 0.0f, 0.0f );
	static rtu::float3 _nearV( 0.0f, 1.0f, 0.0f );
	static float _invWidth = 1.0f / 400.0f;
	static float _invHeight = 1.0f / 300.0f;

void porca( unsigned int x, unsigned int y, rts::RTstate& state )
{
	const float xStep = static_cast<float>( x ) * _invWidth;
	const float yStep = static_cast<float>( y ) * _invHeight;

	// Don't need to normalize ray direction
	rtsRayOrigin( state ) = _position;
	rtsRayDirection( state ).set( _baseDir.x + _nearU.x*xStep + _nearV.x*yStep,	    // x
		_baseDir.y + _nearU.y*xStep + _nearV.y*yStep,	    // y
		_baseDir.z + _nearU.z*xStep + _nearV.z*yStep );	// z
}
*/

void SingleRenderer::render()
{
	unsigned int width;
	unsigned int height;
	rtsViewport( width, height );
	float* frameBuffer = rtsFrameBuffer();
	rts::RTstate state;
	unsigned int pixel = 0;

	for( unsigned int y = 0; y < height; ++y )
	{
		for( unsigned int x = 0; x < width; ++x )
		{
			rtsInitPrimaryRayState( state, x, y );
			rtsTraceRay( state );
			const rtu::float3& color = rtsResultColor( state );

			frameBuffer[pixel] = color.r;
			++pixel;
			frameBuffer[pixel] = color.g;
			++pixel;
			frameBuffer[pixel] = color.b;
			++pixel;
		}
	}
}

} // namespace rtl
