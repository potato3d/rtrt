#include <rtl/DepthMaterial.h>

namespace rtl {

void DepthMaterial::shade( rts::RTstate& state )
{
	const rtu::float3& hitPos = rtsComputeHitPosition( state );
	rtu::float3& resultColor = rtsResultColor( state );

	float distance = ( hitPos - rtsRayOrigin( state ) ).length();
	resultColor.set( distance, distance, distance );
}

} // namespace rtl
