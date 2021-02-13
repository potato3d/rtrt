#include <rtl/MultiThreadRenderer.h>

#include <omp.h>

namespace rtl {

void MultiThreadRenderer::render()
{
	unsigned int width;
	unsigned int height;
	rtsViewport( width, height );
	int w = (int)width;
	int h = (int)height;
	float* frameBuffer = rtsFrameBuffer();
	rts::RTstate state;
	unsigned int pixel = 0;
	int x, y;

	int chunk = 16;

    // get the number of processors in this system
    //int iCPU = omp_get_num_procs();

    // set the number of threads
    //omp_set_num_threads( iCPU );

	//////////////////// Using do/for directive ////////////////////////////
	//#pragma omp parallel for shared( frameBuffer, h, w ) private( x, y, state ) schedule( dynamic, chunk )
	#pragma omp parallel for shared( frameBuffer, h, w ) private( y ) schedule( dynamic, chunk )
    for( y = 0; y < h; ++y )
    {
		#pragma omp parallel for shared( frameBuffer, h, w, y ) private( x, state ) schedule( dynamic, chunk )
        for( x = 0; x < w; ++x )
        {
            rtsInitPrimaryRayState( state, x, y );
            rtsTraceRay( state );
            const rtu::float3& color = rtsResultColor( state );

            //printf( "--------------\n" );
            //printf( "thread ID: %d\n", omp_get_thread_num() );
            //printf( "pixel: %d\n", pixel );
            //printf( "--------------\n" );
            frameBuffer[(x+y*w)*3]   = color.r;
            frameBuffer[(x+y*w)*3+1] = color.g;
            frameBuffer[(x+y*w)*3+2] = color.b;
        }
    }
    

	////////////////// Using sections directive ////////////////////////////

	//#pragma omp parallel sections shared( frameBuffer, w, h ) private( x, y, pixel, state )
	//{
	//	#pragma omp section
	//	{
	//		pixel = 0;

	//		for( y = 0; y < h / 2; ++y )
	//		{
	//			for( x = 0; x < w; ++x )
	//			{
	//				rtsInitPrimaryRayState( x, y, state );
	//				rtsTraceSingle( state );
	//				const rtu::float3& color = rtsResultColor( state );

	//				frameBuffer[pixel] = color.r;
	//				++pixel;
	//				frameBuffer[pixel] = color.g;
	//				++pixel;
	//				frameBuffer[pixel] = color.b;
	//				++pixel;
	//			}
	//		}
	//	}

	//	#pragma omp section
	//	{
	//		pixel = 3 * (h / 2) * w;

	//		for( y = h / 2; y < h; ++y )
	//		{
	//			for( x = 0; x < w; ++x )
	//			{
	//				rtsInitPrimaryRayState( x, y, state );
	//				rtsTraceSingle( state );
	//				const rtu::float3& color = rtsResultColor( state );

	//				frameBuffer[pixel] = color.r;
	//				++pixel;
	//				frameBuffer[pixel] = color.g;
	//				++pixel;
	//				frameBuffer[pixel] = color.b;
	//				++pixel;
	//			}
	//		}
	//	}
	//} // parallel sections
}

} // namespace rtl
