#include <rtc/AABB.h>
#include <algorithm>

namespace rtc {

// Reusable memory for triangle clipping
static rtu::float3 s_vertexBuffer[8];
static rtu::float3 s_tempBuffer[8];

void AABB::buildFrom( const rtu::float3* vertices, unsigned int vertexCount )
{
	// Check degenerate box
	if( vertexCount == 0 )
	{
		minv.set( rtu::mathf::MAX_VALUE, rtu::mathf::MAX_VALUE, rtu::mathf::MAX_VALUE );
		maxv.set( rtu::mathf::MIN_VALUE, rtu::mathf::MIN_VALUE, rtu::mathf::MIN_VALUE );
		return;
	}
	
	// Get initial values
	minv = vertices[0];
	maxv = vertices[0];

	// For each vertex beyond first
	expandBy( vertices + 1, vertexCount - 1 );
}

void AABB::expandBy( const rtu::float3* vertices, unsigned int vertexCount )
{
	for( unsigned int i = 0; i < vertexCount; ++i )
	{
		const rtu::float3& current = vertices[i];

		if( current.x < minv.x )
			minv.x = current.x;
		else if( current.x > maxv.x )
			maxv.x = current.x;

		if( current.y < minv.y )
			minv.y = current.y;
		else if( current.y > maxv.y )
			maxv.y = current.y;

		if( current.z < minv.z )
			minv.z = current.z;
		else if( current.z > maxv.z )
			maxv.z = current.z;
	}
}

void AABB::expandBy( const AABB& other )
{
	// Min vertex
	if( other.minv.x < minv.x )
		minv.x = other.minv.x;

	if( other.minv.y < minv.y )
		minv.y = other.minv.y;

	if( other.minv.z < minv.z )
		minv.z = other.minv.z;

	// Max vertex
	if( other.maxv.x > maxv.x )
		maxv.x = other.maxv.x;

	if( other.maxv.y > maxv.y )
		maxv.y = other.maxv.y;

	if( other.maxv.z > maxv.z )
		maxv.z = other.maxv.z;
}

void AABB::clipTriangle( AABB& result, const rtu::float3& v0, const rtu::float3& v1, const rtu::float3& v2 ) const
{
	// TODO: Sutherland-Hodgman Clipping
	// may optimize by reusing triangle aabb and checking if it needs to be updated (see arauna src)
	// TODO: global variables will cause problems when code becomes multi-threaded / parallelized!!!
	s_vertexBuffer[0] = v0;
	s_vertexBuffer[1] = v1;
	s_vertexBuffer[2] = v2;

	unsigned int vertexCount = 3;

	clip( vertexCount, minv.x, 1.0f, 0 );
	clip( vertexCount, minv.y, 1.0f, 1 );
	clip( vertexCount, minv.z, 1.0f, 2 );
	clip( vertexCount, maxv.x, -1.0f, 0 );
	clip( vertexCount, maxv.y, -1.0f, 1 );
	clip( vertexCount, maxv.z, -1.0f, 2 );

	result.buildFrom( s_vertexBuffer, vertexCount );
}

void AABB::split( AABB& left, AABB& right, const SplitPlane& plane ) const
{
	// TODO: not sure if these first two tests are needed
	if( plane.position < minv[plane.axis] )
	{
		// Plane is in the left, outside box
		right = *this;
		left.minv.set( 0, 0, 0 );
		left.maxv = left.minv;
	}
	else if( plane.position > maxv[plane.axis] )
	{
		// Plane is in the right, outside box
		left = *this;
		right.minv.set( 0, 0, 0 );
		right.maxv = right.minv;
	}
	else
	{
		// Plane splits box in two
		left = *this;
		right = *this;
		left.maxv[plane.axis] = plane.position;
		right.minv[plane.axis] = plane.position;
	}
}

bool AABB::isPlanar( unsigned int dimension ) const
{
	return minv[dimension] == maxv[dimension];
}

bool AABB::isDegenerate() const
{
	return ( minv.x > maxv.x ) || ( minv.y > maxv.y ) || ( minv.z > maxv.z );
}

float AABB::surfaceArea() const
{
	float a = maxv.x - minv.x;
	float b = maxv.y - minv.y;
	float c = maxv.z - minv.z;
	return a*b + a*c + b*c;
}

bool AABB::clipRay( Ray& ray ) const
{
	float tNear;
	float tFar;
	// Only update ray if found valid clipping
	float rayNear = ray.tnear;
	float rayFar = ray.tfar;

	// TODO: branch first based on ray dir sign to avoid swaps (check)
	// TODO: check ray.dir[i] == 0 (ray is parallel)
	// if ray.dir[0] && ( ray.origin < minv[i] || ray.origin[i] > maxv[i] )
	// return false
	// TODO: at the end of each loop: if ray.tfar < 0, return false (box is behind)
	// TODO: optimize, unroll loop, use SSE, etc

	for( unsigned int i = 0; i < 3; ++i )
	{
		// Update interval for ith bounding box slab
		tNear = ( minv[i] - ray.origin[i] ) * ray.invDir[i];
		tFar  = ( maxv[i] - ray.origin[i] ) * ray.invDir[i];

		// Update parametric interval from slab intersection tnear/tfar
 		if( tNear > tFar )
			std::swap( tNear, tFar );

		rayNear = rtu::mathf::max( rayNear, tNear );
		rayFar = rtu::mathf::min( rayFar, tFar );
	}

	if( rayNear > rayFar )
		return false;

	// Update ray interval
	ray.tnear = rayNear;
	ray.tfar = rayFar;
	return true;
}

// Geimer/Muller branchless version
// TODO: not actually clipping, only testing for intersection.
// TODO: we need to update ray interval!
bool AABB::clipRayBranchless( Ray& ray ) const
{
	float
		l1	= (minv.x - ray.origin.x) * ray.invDir.x,
		l2	= (maxv.x - ray.origin.x) * ray.invDir.x,
		lmin	= rtu::mathf::min(l1,l2),
		lmax	= rtu::mathf::max(l1,l2);

	l1	= (minv.y - ray.origin.y) * ray.invDir.y;
	l2	= (maxv.y - ray.origin.y) * ray.invDir.y;
	lmin	= rtu::mathf::max(rtu::mathf::min(l1,l2), lmin);
	lmax	= rtu::mathf::min(rtu::mathf::max(l1,l2), lmax);

	l1	= (minv.z - ray.origin.z) * ray.invDir.z;
	l2	= (maxv.z - ray.origin.z) * ray.invDir.z;
	lmin	= rtu::mathf::max(rtu::mathf::min(l1,l2), lmin);
	lmax	= rtu::mathf::min(rtu::mathf::max(l1,l2), lmax);

	// TODO: my code
	// TODO: must not update ray segment if not valid intersection!
	// TODO: gives wrong results for multiple geometries/instances
	ray.tnear = lmin;
	ray.tfar = lmax;

	//return ((lmax > 0.f) & (lmax >= lmin));
	//return ((lmax > 0.f) & (lmax > lmin));
	return ((lmax >= 0.f) & (lmax >= lmin));
}

bool AABB::clipRaySSE( Ray& ray ) const
{
	// Use whatever's appropriate to load.
	const __m128 box_min = _mm_set_ps( 1.0f, minv.z, minv.y, minv.x );
	const __m128 box_max = _mm_set_ps( 1.0f, maxv.z, maxv.y, maxv.x );
	const __m128 pos = _mm_set_ps( 1.0f, ray.origin.z, ray.origin.y, ray.origin.x );
	const __m128 inv_dir = _mm_set_ps( 1.0f, ray.invDir.z, ray.invDir.y, ray.invDir.x );

	// Use a div if inverted directions aren't available
	const __m128 l1 = _mm_mul_ps(_mm_sub_ps( box_min, pos ), inv_dir );
	const __m128 l2 = _mm_mul_ps(_mm_sub_ps( box_max, pos ), inv_dir );

	// The order we use for those min/max is vital to filter out
	// NaNs that happens when an inv_dir is +/- inf and
	// (box_min - pos) is 0. inf * 0 = NaN
	const __m128 filtered_l1a = _mm_min_ps( l1, rtu::SSE_PLUS_INF );
	const __m128 filtered_l2a = _mm_min_ps( l2, rtu::SSE_PLUS_INF );

	const __m128 filtered_l1b = _mm_max_ps( l1, rtu::SSE_MINUS_INF );
	const __m128 filtered_l2b = _mm_max_ps( l2, rtu::SSE_MINUS_INF );

	// Now that we're back on our feet, test those slabs.
	__m128 lmax = _mm_max_ps( filtered_l1a, filtered_l2a );
	__m128 lmin = _mm_min_ps( filtered_l1b, filtered_l2b );

	// Unfold back. try to hide the latency of the shufps & co.
	const __m128 lmax0 = rtu::sseRotateLeft( lmax );
	const __m128 lmin0 = rtu::sseRotateLeft( lmin );
	lmax = _mm_min_ss(lmax, lmax0);
	lmin = _mm_max_ss(lmin, lmin0);

	const __m128 lmax1 = _mm_movehl_ps(lmax,lmax);
	const __m128 lmin1 = _mm_movehl_ps(lmin,lmin);
	lmax = _mm_min_ss(lmax, lmax1);
	lmin = _mm_max_ss(lmin, lmin1);

	const int ret = _mm_comige_ss(lmax, _mm_setzero_ps()) & _mm_comige_ss(lmax,lmin);

	// TODO: must not update ray segment if not valid intersection!
	// TODO: gives wrong results for multiple geometries/instances
	_mm_store_ss( &ray.tnear, lmin );
	_mm_store_ss( &ray.tfar, lmax );

	return ( ret != 0 );
}

/*
*  Vertices are computed as follows:
*     7+------+6
*     /|     /|      y
*    / |    / |      |
*   / 3+---/--+2     |
* 4+------+5 /       *---x
*  | /    | /       /
*  |/     |/       z
* 0+------+1      
*
* Assumes array is pre-allocated!
*/
void AABB::computeVertices( rtu::float3* vertices ) const
{
	vertices[0].set( minv.x, minv.y, maxv.z );
	vertices[1].set( maxv.x, minv.y, maxv.z );
	vertices[2].set( maxv.x, minv.y, minv.z );
	vertices[3] = minv;
	vertices[4].set( minv.x, maxv.y, maxv.z );
	vertices[5] = maxv;
	vertices[6].set( maxv.x, maxv.y, minv.z );
	vertices[7].set( minv.x, maxv.y, minv.z );
}

// Private methods
void AABB::clip( unsigned int& vertexCount, float pos, float dir, unsigned int dim ) const
{
	bool allin = true;
	bool allout = true;

	// Try to accept or reject all vertices
	for( unsigned int i = 0; i < vertexCount; ++i )
	{
		float dist = dir * ( s_vertexBuffer[i][dim] - pos );
		if( dist < 0 )
			allin = false;
		else
			allout = false;
	}

	// Check if all vertices were accepted or rejected
	if( allin )
		return;

	if( allout )
	{
		vertexCount = 0;
		return;
	}

	// Need to add each vertex and potential intersection points
	rtu::float3 v1 = s_vertexBuffer[0];
	float d1 = dir * ( v1[dim] - pos );
	bool inside = ( d1 >= 0 );
	unsigned int count = 0;

	for( unsigned int i = 0; i < vertexCount; ++i )
	{
		const rtu::float3& v2 = s_vertexBuffer[(i + 1) % vertexCount];
		float d2 = dir * ( v2[dim] - pos );

		if( inside && ( d2 >= 0 ) ) 
		{
			// Previous and current are inside, add current (assume first has been added)
			s_tempBuffer[count++] = v2;
		}
		else if( !inside && ( d2 >= 0 ) )
		{
			// Previous outside and current inside, add intersection point and then current
			float d = d1 / (d1 - d2);
			rtu::float3& vc = v1 + ( (v2 - v1) * d );
			vc[dim] = pos;
			s_tempBuffer[count++] = vc;
			s_tempBuffer[count++] = v2;
			inside = true;
		}
		else if( inside && ( d2 < 0 ) )
		{
			// Previous inside and current outside, add intersection point
			float d = d2 / (d2 - d1);
			rtu::float3& vc = v2 + ( (v1 - v2) * d );
			vc[dim] = pos;
			s_tempBuffer[count++] = vc;
			inside = false;
		}
		// Update previous vertex info
		v1 = v2;
		d1 = d2;
	}

	// New vertex count
	vertexCount = 0;

	for( unsigned int i = 0; i < count; i++ )
	{
		const rtu::float3& dist = s_tempBuffer[i] - s_tempBuffer[(i + count - 1) % count];
		if( dist.length() > rtu::mathf::ZERO_TOLERANCE )
			s_vertexBuffer[vertexCount++] = s_tempBuffer[i];
	}
}

} // namespace rtc
