#include <rtl/TiledRenderer.h>

namespace rtl {

void TiledRenderer::render()
{
	unsigned int width;
	unsigned int height;
	rtsViewport( width, height );
	const int w = (int)width;
	const int h = (int)height;
	float* frameBuffer = rtsFrameBuffer();
	rts::RTstate sample;

	const int tileSize = 16;
	const int chunk = 1;

	const int numTilesX = rtu::mathf::round( (float)w / (float)tileSize );
	const int numTilesY = rtu::mathf::round( (float)h / (float)tileSize );
	const float invNumTilesX = 1.0f / (float)numTilesX;
	const int limit = numTilesX * numTilesY;

	#pragma omp parallel for shared( frameBuffer, tileSize, h, w, numTilesX ) private( sample ) schedule( dynamic, chunk )
	for( int i = 0; i < limit; ++i )
	{
		const int ty = ( i / numTilesX ) * tileSize;
		const int tx = ( i % numTilesX ) * tileSize;

		for( int dy = 0; dy < tileSize; ++dy )
		{
			const int y = ty + dy;
			if( y >= h )
				continue;

			for( int dx = 0; dx < tileSize; ++dx )
			{
				const int x = tx + dx;
				if( x >= w )
					continue;

				rtsInitPrimaryRayState( sample, x, y );
				rtsTraceRay( sample );
				const rtu::float3& color = rtsResultColor( sample );

				frameBuffer[(x+y*w)*3]   = color.r;
				frameBuffer[(x+y*w)*3+1] = color.g;
				frameBuffer[(x+y*w)*3+2] = color.b;
			}
		}
	}

	////#pragma omp parallel for shared( frameBuffer, tileSize, h, w ) private( ty ) schedule( dynamic, chunk )
	//for( ty = 0; ty < h; ty+=tileSize )
	//{
	//	#pragma omp parallel for shared( frameBuffer, tileSize, h, w, ty ) private( tx, dx, dy, x, y, sample ) schedule( dynamic, chunk )
	//	for( tx = 0; tx < w; tx+=tileSize )
	//	{
	//		for( dy = 0; dy < tileSize; ++dy )
	//		{
	//			y = ty + dy;
	//			if( y >= h )
	//				continue;

	//			for( dx = 0; dx < tileSize; ++dx )
	//			{
	//				x = tx + dx;
	//				if( x >= w )
	//					continue;

	//				rtsInitPrimaryRayState( x, y, sample );
	//				rtsTraceSingle( sample );
	//				const rtu::float3& color = rtsResultColor( sample );

	//				frameBuffer[(x+y*w)*3]   = color.r;
	//				frameBuffer[(x+y*w)*3+1] = color.g;
	//				frameBuffer[(x+y*w)*3+2] = color.b;
	//			}
	//		}
	//	}
	//}
}

} // namespace rtl
