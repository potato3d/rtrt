#include <rtl/HeadlightColor.h>

namespace rtl {

void HeadlightColor::init()
{
	_ambient.set( 0.1f, 0.1f, 0.1f );
	_diffuse.set( 1.0f, 1.0f, 1.0f );
}

void HeadlightColor::shade( rts::RTstate& state )
{
	const rtu::float3& normal = rtsComputeShadingNormal( state );
	//rtu::float3 normal; rtsComputeFlatNormal( state, normal );
	rtu::float3& resultColor = rtsResultColor( state );
	rtu::float3& rayDir = rtsRayDirection( state );
	rayDir.normalize();

	const float nDotD = -( normal.dot( rayDir ) );
	//const float nDotD = rtu::mathf::abs( normal.dot( rayDir ) );

	resultColor.r = ( _ambient.r + ( _diffuse.r - _ambient.r ) * nDotD ) * _diffuse.r;
	resultColor.g = ( _ambient.g + ( _diffuse.g - _ambient.g ) * nDotD ) * _diffuse.g;
	resultColor.b = ( _ambient.b + ( _diffuse.b - _ambient.b ) * nDotD ) * _diffuse.b;
	
	//resultColor = normal;
	//resultColor.r = rtu::mathf::abs( normal.r );
	//resultColor.g = rtu::mathf::abs( normal.g );
	//resultColor.b = rtu::mathf::abs( normal.b );
}

} // namespace rtl
