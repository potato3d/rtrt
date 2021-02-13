#include <rtc/Triangle.h>

namespace rtc {

void TriAccel::buildFrom( const rtu::float3& v0, const rtu::float3& v1, const rtu::float3& v2 )
{
	// Compute edges and normal
	rtu::float3 b = v2 - v0;
	rtu::float3 c = v1 - v0;
	rtu::float3 normal = c.cross( b );

	// Determine projection dimension
	unsigned int dim;
	if( rtu::mathf::abs( normal.x ) > rtu::mathf::abs( normal.y ) )
	{
		if( rtu::mathf::abs( normal.x ) > rtu::mathf::abs( normal.z ) )
			dim = 0; /* X */
		else 
			dim = 2; /* Z */
	}
	else
	{
		if( rtu::mathf::abs( normal.y ) > rtu::mathf::abs( normal.z ) )
			dim = 1; /* Y */
		else 
			dim = 2; /* Z */
	}
	k = dim;

	// Secondary dimensions
	unsigned int u = ( k + 1 ) % 3;
	unsigned int v = ( k + 2 ) % 3;

	// Store normal
	n_u = normal[u] / normal[k];
	n_v = normal[v] / normal[k];

	// Compute N'
	rtu::float3 nLine( normal );
	nLine *= 1.0f / normal[k];

	// Store plane distance
	n_d = v0.dot( nLine );

	// Compute constant for line ac
	float invDenom = 1.0f / ( b[u]*c[v] - b[v]*c[u] );
	b_nu = -b[v] * invDenom;
	b_nv =  b[u] * invDenom;
	b_d  = ( b[v]*v0[u] - b[u]*v0[v] ) * invDenom;

	// Compute constant for line ab
	c_nu =  c[v] * invDenom;
	c_nv = -c[u] * invDenom;
	c_d  = ( c[u]*v0[v] - c[v]*v0[u] ) * invDenom;
}

bool TriAccel::valid()
{
	// Check for INFs and NaNs
	return !( rtu::mathf::isInvalid( n_u )  || rtu::mathf::isInvalid( n_v )  || rtu::mathf::isInvalid( n_d )  || 
		      rtu::mathf::isInvalid( b_nu ) || rtu::mathf::isInvalid( b_nv ) || rtu::mathf::isInvalid( b_d )  ||
		      rtu::mathf::isInvalid( c_nu ) || rtu::mathf::isInvalid( c_nv ) || rtu::mathf::isInvalid( c_d )  );
}

} // namespace rtc
