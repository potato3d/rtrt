#include <rtl/AdaptiveRenderer.h>
#include <rtu/random.h>

#include <omp.h>

namespace rtl {

void AdaptiveRenderer::init()
{
	rtu::Random::autoSeed();
	_epsilon = 0.01f;
	_maxRecursionDepth = 3;
}

void AdaptiveRenderer::render()
{
	unsigned int width;
	unsigned int height;
	rtsViewport( width, height );
	int w = (int)width;
	int h = (int)height;
	float* frameBuffer = rtsFrameBuffer();
	rtu::float3 resultColor;
	int x, y;

	int chunk = 16;

	#pragma omp parallel for shared( frameBuffer, h, w ) private( y ) schedule( dynamic, chunk )
	for( y = 0; y < h; ++y )
	{
		#pragma omp parallel for shared( frameBuffer, h, w, y ) private( x, resultColor ) schedule( dynamic, chunk )
		for( x = 0; x < w; ++x )
		{
			adaptiveSupersample( x, y, resultColor, 1 );
			
			frameBuffer[(x+y*w)*3]   = resultColor.r;
			frameBuffer[(x+y*w)*3+1] = resultColor.g;
			frameBuffer[(x+y*w)*3+2] = resultColor.b;
		}
	}
}

void AdaptiveRenderer::adaptiveSupersample( float x, float y, rtu::float3& resultColor, unsigned int recursionDepth )
{
	const float deltaRatio = 0.5f / (float)recursionDepth;
	float deltaX;
	float deltaY;
	rts::RTstate upperLeftSample;
	rts::RTstate upperRightSample;
	rts::RTstate lowerLeftSample;
	rts::RTstate lowerRightSample;

	// Upper left (A)
	deltaX = rtu::Random::real( -deltaRatio, 0.0f );
	deltaY = rtu::Random::real( 0.0f, deltaRatio );
	rtsInitPrimaryRayState( upperLeftSample, x + deltaX, y + deltaY );
	rtsTraceRay( upperLeftSample );
	rtu::float3 upperLeftColor = rtsResultColor( upperLeftSample );

	// Upper right (B)
	deltaX = rtu::Random::real( 0.0f, deltaRatio );
	deltaY = rtu::Random::real( 0.0f, deltaRatio );
	rtsInitPrimaryRayState( upperRightSample, x + deltaX, y + deltaY );
	rtsTraceRay( upperRightSample );
	rtu::float3 upperRightColor = rtsResultColor( upperRightSample );

	// Lower left (C)
	deltaX = rtu::Random::real( -deltaRatio, 0.0f );
	deltaY = rtu::Random::real( -deltaRatio, 0.0f );
	rtsInitPrimaryRayState( lowerLeftSample, x + deltaX, y + deltaY );
	rtsTraceRay( lowerLeftSample );
	rtu::float3 lowerLeftColor = rtsResultColor( lowerLeftSample );

	// Lower right (D)
	deltaX = rtu::Random::real( 0.0f, deltaRatio );
	deltaY = rtu::Random::real( -deltaRatio, 0.0f );
	rtsInitPrimaryRayState( lowerRightSample, x + deltaX, y + deltaY );
	rtsTraceRay( lowerRightSample );
	rtu::float3 lowerRightColor = rtsResultColor( lowerRightSample );

	if( recursionDepth < _maxRecursionDepth )
	{
		// If too much difference in sample values
		rtu::float3& ab = upperLeftColor - upperRightColor;
		ab.set( rtu::mathf::abs( ab.r ), rtu::mathf::abs( ab.g ), rtu::mathf::abs( ab.b ) );

		rtu::float3& ac = upperLeftColor - lowerLeftColor;
		ac.set( rtu::mathf::abs( ac.r ), rtu::mathf::abs( ac.g ), rtu::mathf::abs( ac.b ) );

		rtu::float3& ad = upperLeftColor - lowerRightColor;
		ad.set( rtu::mathf::abs( ad.r ), rtu::mathf::abs( ad.g ), rtu::mathf::abs( ad.b ) );

		rtu::float3& bc = upperRightColor - lowerLeftColor;
		bc.set( rtu::mathf::abs( bc.r ), rtu::mathf::abs( bc.g ), rtu::mathf::abs( bc.b ) );

		rtu::float3& bd = upperRightColor - lowerRightColor;
		bd.set( rtu::mathf::abs( bd.r ), rtu::mathf::abs( bd.g ), rtu::mathf::abs( bd.b ) );

		rtu::float3& cd = upperLeftColor - lowerRightColor;
		cd.set( rtu::mathf::abs( cd.r ), rtu::mathf::abs( cd.g ), rtu::mathf::abs( cd.b ) );

		if( ( ab.r > _epsilon ) || ( ab.g > _epsilon ) || ( ab.b > _epsilon ) ||
			( ac.r > _epsilon ) || ( ac.g > _epsilon ) || ( ac.b > _epsilon ) ||
			( ad.r > _epsilon ) || ( ad.g > _epsilon ) || ( ad.b > _epsilon ) ||
			( bc.r > _epsilon ) || ( bc.g > _epsilon ) || ( bc.b > _epsilon ) ||
			( bd.r > _epsilon ) || ( bd.g > _epsilon ) || ( bd.b > _epsilon ) ||
			( cd.r > _epsilon ) || ( cd.g > _epsilon ) || ( cd.b > _epsilon ) )
		{
			// Recursive supersample
			const float recDelta = 0.5f / ( (float)recursionDepth + 1.0f );

			rtu::float3 recUpperLeft;
			rtu::float3 recUpperRight;
			rtu::float3 recLowerLeft;
			rtu::float3 recLowerRight;

			adaptiveSupersample( x - recDelta, y + recDelta, recUpperLeft, recursionDepth + 1 );
			adaptiveSupersample( x + recDelta, y + recDelta, recUpperRight, recursionDepth + 1 );
			adaptiveSupersample( x - recDelta, y - recDelta, recLowerLeft, recursionDepth + 1 );
			adaptiveSupersample( x + recDelta, y - recDelta, recLowerRight, recursionDepth + 1 );

			// Average results
			resultColor = ( upperLeftColor * 0.125f ) + ( recUpperLeft * 0.125f ) + 
				          ( upperRightColor * 0.125f ) + ( recUpperRight * 0.125f ) + 
						  ( lowerLeftColor * 0.125f ) + ( recLowerLeft * 0.125f ) + 
						  ( lowerRightColor * 0.125f ) + ( recLowerRight * 0.125f );
		}
		else
		{
			// Average results
			resultColor = ( upperLeftColor * 0.25f ) + ( upperRightColor * 0.25f ) + ( lowerLeftColor * 0.25f ) + 
				          ( lowerRightColor * 0.25f );
		}
	}
	else
	{
		// Average results
		resultColor = ( upperLeftColor * 0.25f ) + ( upperRightColor * 0.25f ) + ( lowerLeftColor * 0.25f ) + 
			          ( lowerRightColor * 0.25f );
	}
}

} // namespace rtl
