#include <rtl/Headlight.h>

namespace rtl {

void Headlight::init()
{
	_ambient.set( 0.1f, 0.1f, 0.1f );
}

void Headlight::shade( rts::RTstate& state )
{
	const rtu::float3& normal = rtsComputeShadingNormal( state );
	//rtu::float3 normal; rtsComputeFlatNormal( state, normal );
	rtu::float3& resultColor = rtsResultColor( state );
	rtu::float3& rayDir = rtsRayDirection( state );
	rayDir.normalize();

	rtu::float3 color;
	rtsComputeShadingColor( state, color );

	const float nDotD = -( normal.dot( rayDir ) );
	//const float nDotD = rtu::mathf::abs( normal.dot( rayDir ) );

	resultColor.r = ( _ambient.r + ( color.r - _ambient.r ) * nDotD ) * color.r;
	resultColor.g = ( _ambient.g + ( color.g - _ambient.g ) * nDotD ) * color.g;
	resultColor.b = ( _ambient.b + ( color.b - _ambient.b ) * nDotD ) * color.b;
	
	//resultColor = normal;
	//resultColor.r = rtu::mathf::abs( normal.r );
	//resultColor.g = rtu::mathf::abs( normal.g );
	//resultColor.b = rtu::mathf::abs( normal.b );
}

} // namespace rtl
