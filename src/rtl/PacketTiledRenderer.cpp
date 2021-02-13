#include <rtl/PacketTiledRenderer.h>

namespace rtl {

// Second iteration of a Hilbert curve
static const float PACKET_GRID[] = { 0, 0,   1, 0,    1, 1,    0, 1,    0, 2,    0, 3,   1, 3,   1, 2,
                                     2, 2,   2, 3,    3, 3,    3, 2,    3, 1,    2, 1,   2, 0,   3, 0 };

void PacketTiledRenderer::render()
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
	const int coordSize = RT_PACKET_SIZE*2;

	float rayXYCoords[coordSize];

	const int numTilesX = rtu::mathf::round( (float)w / (float)tileSize );
	const int numTilesY = rtu::mathf::round( (float)h / (float)tileSize );
	const float invNumTilesX = 1.0f / (float)numTilesX;
	const int limit = numTilesX * numTilesY;

	#pragma omp parallel for shared( frameBuffer, tileSize, coordSize, h, w, numTilesX ) private( sample, rayXYCoords ) schedule( dynamic, chunk )
	for( int i = 0; i < limit; ++i )
	{
		const int ty = ( i / numTilesX ) * tileSize;
		const int tx = ( i % numTilesX ) * tileSize;

		for( int dy = 0; dy < tileSize; dy+=RT_PACKET_DIM )
		{
			const int y = ty + dy;
			if( y >= h )
				continue;

			for( int dx = 0; dx < tileSize; dx+=RT_PACKET_DIM )
			{
				const int x = tx + dx;
				if( x >= w )
					continue;

				// Fill packet x, y ray values
				for( int i = 0; i < coordSize; i+=2 )
				{
					rayXYCoords[i]   = x + PACKET_GRID[i];
					rayXYCoords[i+1] = y + PACKET_GRID[i+1];
				}

				// Setup ray packet with camera
				rtsInitPrimaryRayStatePacket( sample, rayXYCoords );

				// Trace rays
				rtsTraceRayPacket( sample );

				// Get color results
				for( int k = 0, r = 0; k < coordSize; k+=2, ++r )
				{
					const int destX = rayXYCoords[k];
					const int destY = rayXYCoords[k+1];
					const rtu::float3& color = rtsResultColorPacket( sample, r );

					frameBuffer[(destX+destY*w)*3]   = color.r;
					frameBuffer[(destX+destY*w)*3+1] = color.g;
					frameBuffer[(destX+destY*w)*3+2] = color.b;
				}
			}
		}
	}
}

} // namespace rtl
