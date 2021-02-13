#include <rtl/JitteredRenderer.h>
#include <rtu/random.h>

namespace rtl {

static const float TWO_BY_TWO_GRID[] = { -0.25f, -0.25f, // line 0
                                     0.25f, -0.25f, 
								    -0.25f,  0.25f, // line 1
								     0.25f,  0.25f };

static const float FOUR_BY_FOUR_GRID[] = { -0.375f, -0.375f, // line 0 
									  -0.125f, -0.375f, 
									   0.125f, -0.375f, 
									   0.375f, -0.375f,
									  -0.375f, -0.125f, // line 1
									  -0.125f, -0.125f,
									   0.125f, -0.125f,
									   0.375f, -0.125f,
									  -0.375f,  0.125f, // line 2
									  -0.125f,  0.125f,
									   0.125f,  0.125f,
									   0.375f,  0.125f,
									  -0.375f,  0.375f, // line 3
									  -0.125f,  0.375f,
									   0.125f,  0.375f,
									   0.375f,  0.375f };

void JitteredRenderer::init()
{
	rtu::Random::autoSeed();
	_gridRes = FOUR_BY_FOUR;
}

void JitteredRenderer::render()
{
	unsigned int width;
	unsigned int height;
	rtsViewport( width, height );
	int w = (int)width;
	int h = (int)height;
	float* frameBuffer = rtsFrameBuffer();
	rts::RTstate sample;
	rtu::float3 resultColor;
	int x, y;

	const float* grid;
	unsigned int gridSize;
	float ratio;

	switch( _gridRes )
	{
	case TWO_BY_TWO:
		grid = TWO_BY_TWO_GRID;
		gridSize = 8;
		ratio = 0.25f;
		break;

	case FOUR_BY_FOUR:
		grid = FOUR_BY_FOUR_GRID;
		gridSize = 32;
		ratio = 0.0625f;
		break;

	default:
	    return;
	}

	int chunk = 16;

	#pragma omp parallel for shared( frameBuffer, h, w ) private( y ) schedule( dynamic, chunk )
	for( y = 0; y < h; ++y )
	{
		#pragma omp parallel for shared( frameBuffer, h, w, y, grid, gridSize, ratio ) private( x, resultColor, sample ) schedule( dynamic, chunk )
		for( x = 0; x < w; ++x )
		{
			resultColor.set( 0.0f, 0.0f, 0.0f );

			unsigned int i = 0;
			while( i < gridSize )
			{
				rtsInitPrimaryRayState( sample, (float)x + grid[i++] + rtu::Random::real( -ratio, ratio ), 
					                            (float)y + grid[i++] + rtu::Random::real( -ratio, ratio ) );
				rtsTraceRay( sample );
				resultColor += rtsResultColor( sample );
			}
			
			resultColor *= ratio;
			
			frameBuffer[(x+y*w)*3]   = resultColor.r;
			frameBuffer[(x+y*w)*3+1] = resultColor.g;
			frameBuffer[(x+y*w)*3+2] = resultColor.b;
		}
	}
}

void JitteredRenderer::setGridResolution( GridResolution res )
{
	_gridRes = res;
}

} // namespace rtl
