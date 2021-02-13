#include <rtc/RayTracer.h>
#include <rtc/Plugins.h>
#include <omp.h>

namespace rtc {

#define _TO_RAY_STATE(s) reinterpret_cast<RayState&>( (s) )
#define _TO_RAY_PACKET_STATE(s) reinterpret_cast<rtc::RayPacketState&>( (s) )

#define _TO_RT_STATE(s) reinterpret_cast<rts::RTstate&>( (s) )

#define KU _modulo[acc.k+1]
#define KV _modulo[acc.k+2]

#define QKU _quadModulo[acc.k+1]
#define QKV _quadModulo[acc.k+2]

__declspec(thread) static RayTracer::SingleStack s_instanceStack;
__declspec(thread) static RayTracer::SingleStack s_geometryStack;
__declspec(thread) static unsigned int* s_dirSigns;

// Needs to be cache aligned to 16 bytes, cannot be inside thread local storage!
static RayTracer::PacketStack s_packetStacks[RT_MAX_THREAD_COUNT][2];

static const unsigned int INSTANCE_STACK = 0;
static const unsigned int GEOMETRY_STACK = 1;
	
static const float INTERSECT_EPSILON = 1e-4f;
static const __m128 SSE_INTERSECT_EPSILON = _mm_set_ps1( INTERSECT_EPSILON );

RayTracer::RayTracer()
{
	_modulo[0] = 0;
	_modulo[1] = 1;
	_modulo[2] = 2;
	_modulo[3] = 0;
	_modulo[4] = 1;

	_quadModulo[0] = 0;
	_quadModulo[1] = 4;
	_quadModulo[2] = 8;
	_quadModulo[3] = 0;
	_quadModulo[4] = 4;

	/*
	 *	000 111
	 *	100 011
	 *	010 101
	 *	110 001
	 *	001 110
	 *	101 010
	 *	011 100
	 *	111 000
	 */
	for( int i = 0; i < 8; ++i )
	{
		const unsigned int rdx = i & 1;
		const unsigned int rdy = (i >> 1) & 1;
		const unsigned int rdz = (i >> 2) & 1;
		_rayDirSigns[i][0][0] = rdx, _rayDirSigns[i][0][1] = rdx ^ 1;
		_rayDirSigns[i][1][0] = rdy, _rayDirSigns[i][1][1] = rdy ^ 1;
		_rayDirSigns[i][2][0] = rdz, _rayDirSigns[i][2][1] = rdz ^ 1;
	}
}

void RayTracer::bruteFroce( rts::RTstate& state )
{
	RayState& rs = _TO_RAY_STATE( state );
	Ray& ray = rs.ray;
	Hit& hit = rs.hit;

	// Init ray
	ray.tnear = rtc::Scene::rayEpsilon;
	ray.tfar = rtu::mathf::MAX_VALUE;
	ray.update();

	// Init hit
	hit.instance = NULL;
	hit.geometry = NULL;
	hit.distance = rtu::mathf::MAX_VALUE;

	const Geometry& geometry = Scene::geometries[0];
	const std::vector<TriAccel>& triangles = geometry.triAccel;
	hit.distance = ray.tfar;

	if( !geometry.kdTree.bbox.clipRay( ray ) )
	{
		Plugins::environment->shade( state );
		return;
	}

	float bestDistance = hit.distance;

	for( unsigned int i = 0, limit = triangles.size(); i < limit; ++i )
	{
		intersectSingle( geometry.triAccel[geometry.kdTree.elements[i]], ray, hit, bestDistance );
	}

	if( bestDistance < hit.distance )
	{
		hit.instance = &Scene::instances[0];
		hit.geometry = &geometry;
		hit.distance = bestDistance;
		Plugins::materials[hit.geometry->triDesc[hit.triangleId].materialId]->shade( state );
	}
	else
	{
		Plugins::environment->shade( state );
		return;
	}
}

bool RayTracer::bruteForceShadow( rts::RTstate& state )
{
	RayState& rs = _TO_RAY_STATE( state );
	Ray& ray = rs.ray;
	Hit& hit = rs.hit;

	// Init ray
	ray.tnear = rtc::Scene::rayEpsilon;
	ray.update();

	const Geometry& geometry = Scene::geometries[0];
	const std::vector<TriAccel>& triangles = geometry.triAccel;
	hit.distance = ray.tfar;

	if( !geometry.kdTree.bbox.clipRay( ray ) )
		return false;

	float bestDistance = hit.distance;

	for( unsigned int i = 0, limit = triangles.size(); i < limit; ++i )
	{
		intersectSingle( geometry.triAccel[geometry.kdTree.elements[i]], ray, hit, bestDistance );
	}

	if( bestDistance < hit.distance )
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////
// Ray-kdtree traversal routines

// Trace a single ray against the entire scene
void RayTracer::traceSingle( rts::RTstate& state )
{
	RayState& rs = _TO_RAY_STATE( state );
	Ray& ray = rs.ray;
	Hit& hit = rs.hit;

	// Init ray
	ray.tnear = rtc::Scene::rayEpsilon;
	ray.tfar = rtu::mathf::MAX_VALUE;
	ray.update();

	// Init hit
	hit.instance = NULL;
	hit.geometry = NULL;
	hit.distance = rtu::mathf::MAX_VALUE;

	const KdTree& tree = Scene::instanceTree;

	if( !tree.bbox.clipRay( ray ) )
	{
		Plugins::environment->shade( state );
		return;
	}

	const Ray originalRay( ray );
	const KdNode* node = tree.root;
    s_instanceStack.clear();

	while( true )
	{
		findLeafSingle( node, ray, s_instanceStack );

		for( unsigned int i = node->elemStart(), limit = i + node->elemCount(); i < limit; ++i )
		{
			const Instance& instance = Scene::instances[tree.elements[i]];

			instance.transform.inverseTransform( ray );
			ray.update();

			traceGeometrySingle( instance, ray, hit );

			ray = originalRay;
		}

		if( hit.geometry )
		{
			Plugins::materials[hit.geometry->triDesc[hit.triangleId].materialId]->shade( state );
			return;
		}

		if( s_instanceStack.empty() )
		{
			Plugins::environment->shade( state );
			return;
		}

		const TraversalData& data = s_instanceStack.top();
		s_instanceStack.pop();
		node = data.node;
		ray.tnear = data.tnear;
		ray.tfar = data.tfar;
	}
}

void RayTracer::traceGeometrySingle( const Instance& instance, Ray& ray, Hit& hit )
{
	const Geometry& geometry = Scene::geometries[instance.geometryId];
	const KdTree& tree = geometry.kdTree;

	if( !tree.bbox.clipRay( ray ) )
		return;

	const KdNode* node = tree.root;
	float bestDistance = hit.distance;
    s_geometryStack.clear();

	while( true )
	{
		findLeafSingle( node, ray, s_geometryStack );

		for( unsigned int i = node->elemStart(), limit = i + node->elemCount(); i < limit; ++i )
		{
			intersectSingle( geometry.triAccel[tree.elements[i]], ray, hit, bestDistance );
		}

		if( bestDistance < hit.distance )
		{
			hit.distance = bestDistance;
			hit.instance = &instance;
			hit.geometry = &geometry;
			return;
		}

		if( s_geometryStack.empty() )
			return;

		const TraversalData& data = s_geometryStack.top();
		s_geometryStack.pop();
		node = data.node;
		ray.tnear = data.tnear;
		ray.tfar = data.tfar;
	}
}

// Returns true if ray hits any object, false otherwise
bool RayTracer::traceHitSingle( rts::RTstate& state )
{
	RayState& rs = _TO_RAY_STATE( state );
	Ray& ray = rs.ray;

	// Init ray
	ray.tnear = rtc::Scene::rayEpsilon;
	ray.update();

	const KdTree& tree = Scene::instanceTree;

	if( !tree.bbox.clipRay( ray ) )
		return false;

	const Ray originalRay( ray );
	const KdNode* node = tree.root;
	s_instanceStack.clear();

	while( true )
	{
		findLeafSingle( node, ray, s_instanceStack );

		for( unsigned int i = node->elemStart(), limit = i + node->elemCount(); i < limit; ++i )
		{
			const Instance& instance = Scene::instances[tree.elements[i]];

			instance.transform.inverseTransform( ray );
			ray.update();

			//////////////////////////////////////////////////////////////////////////
			// Trace Geometry Single
			const Geometry& geometry = Scene::geometries[instance.geometryId];
			const KdTree& gtree = geometry.kdTree;

			if( gtree.bbox.clipRay( ray ) )
			{
				const KdNode* gnode = gtree.root;
				s_geometryStack.clear();

				while( true )
				{
					findLeafSingle( gnode, ray, s_geometryStack );

					for( unsigned int i = gnode->elemStart(), limit = i + gnode->elemCount(); i < limit; ++i )
					{
						//////////////////////////////////////////////////////////////////////////
						// Intersect Single
						const TriAccel& acc = geometry.triAccel[gtree.elements[i]];

						// Start high-latency division as early as possible
						const float nd = 1.0f / ( ray.direction[acc.k] + acc.n_u * ray.direction[KU] + acc.n_v * ray.direction[KV] );
						const float f = nd * ( acc.n_d - ray.origin[acc.k] - acc.n_u * ray.origin[KU] - acc.n_v * ray.origin[KV] );

						// Check for valid distance
						// TODO: find a correct way to get rid of these epsilons and check scene06 for any errors
						if( ( f < ray.tnear - INTERSECT_EPSILON ) || ( f > ray.tfar + INTERSECT_EPSILON ) )
							continue;

						// Compute hit point positions on uv plane
						const float hu = ray.origin[KU] + f * ray.direction[KU];
						const float hv = ray.origin[KV] + f * ray.direction[KV];

						// Check first barycentric coordinate
						const float lambda = hu * acc.b_nu + hv * acc.b_nv + acc.b_d;
						if( lambda < 0.0f )
							continue;

						// Check second barycentric coordinate
						const float mue = hu * acc.c_nu + hv * acc.c_nv + acc.c_d;
						if( mue < 0.0f )
							continue;

						// Check third barycentric coordinate
						const float psi = 1.0f - lambda - mue;
						if( psi < 0.0f )
							continue;

						// Valid hit
						return true;
						// Intersect Single
						//////////////////////////////////////////////////////////////////////////
					}

					if( s_geometryStack.empty() )
						break;

					const TraversalData& gdata = s_geometryStack.top();
					s_geometryStack.pop();
					gnode = gdata.node;
					ray.tnear = gdata.tnear;
					ray.tfar = gdata.tfar;
				}
			}
			// Trace Geometry Single
			//////////////////////////////////////////////////////////////////////////

			ray = originalRay;
		}

		if( s_instanceStack.empty() )
			return false;

		const TraversalData& data = s_instanceStack.top();
		s_instanceStack.pop();
		node = data.node;
		ray.tnear = data.tnear;
		ray.tfar = data.tfar;
	}
}

// Trace a bundle of rays against the entire scene
void RayTracer::tracePacket( rts::RTstate& state, unsigned int inQ )
{
	RayState shadeState;
	RayPacketState& rs = _TO_RAY_PACKET_STATE( state );
	RayPacket& packet = rs.packet;
	HitPacket& hit = rs.hit;
	const KdTree& tree = Scene::instanceTree;

	// Init ray
	// Packet pre-computation is done outside, to detect incoherent ray bundles
	std::fill_n( packet.tnear, RT_PACKET_SIZE, rtc::Scene::rayEpsilon );
	std::fill_n( packet.tfar, RT_PACKET_SIZE, rtu::mathf::MAX_VALUE );

	// Get ray direction sign bits according to coherence masks computed
	s_dirSigns = &_rayDirSigns[inQ][0][0];

	// Active ray mask for instance traversal and intersection
	union
	{ 
		unsigned int activeMask[RT_PACKET_SIZE];
	    __m128       activeMask4[RT_PACKET_SIMD_SIZE];
	};

	// Clip rays against scene bounding box
	// Disable incoherent rays using packet.mask4
	if( !clipRayPacket( tree.bbox, packet, packet.mask4, activeMask4 ) )
	{
		// TODO: very slow, need to improve this!
		for( int r = 0; r < RT_PACKET_SIZE; ++r )
		{
			// Skip if incoherent ray (will be traced later)
			if( packet.mask[r] == 0 )
				continue;

			// Setup state for shading
			setupShadingRay( shadeState.ray, packet, r );
			shadeState.recursionDepth = rs.recursionDepth[r];
			Plugins::environment->shade( _TO_RT_STATE( shadeState ) );
			rs.resultColor[r] = shadeState.resultColor;
		}
		return;
	}

	// Init hit
	std::fill_n( reinterpret_cast<unsigned int*>( hit.inst ), RT_PACKET_SIZE, NULL );
	std::fill_n( reinterpret_cast<unsigned int*>( hit.geom ), RT_PACKET_SIZE, NULL );
	std::fill_n( hit.dist, RT_PACKET_SIZE, rtu::mathf::MAX_VALUE );

	// Mask for early ray termination
	// Identifies rays that have completed tracing, including shading computations
	union
	{
		unsigned int done[RT_PACKET_SIZE];
		__m128       done4[RT_PACKET_SIMD_SIZE];
	};

	// No need to trace nor shade incoherent rays (use packet.mask4)
	// Inactive rays still need shading from environment, so don't consider activeMask4 here
	for( int p = 0; p < RT_PACKET_SIMD_SIZE; ++p )
	{
		done4[p] = _mm_andnot_ps( packet.mask4[p], rtu::SSE_ALL_ON );
	}

	bool allHit;
	const RayPacket originalPacket( packet );
	const KdNode* node = tree.root;
	PacketStack& s_instancePacketStack = s_packetStacks[omp_get_thread_num()][INSTANCE_STACK];
	s_instancePacketStack.clear();

	while( true )
	{
		findLeafPacket( node, packet, s_instancePacketStack, activeMask4 );

		for( unsigned int i = node->elemStart(), limit = i + node->elemCount(); i < limit; ++i )
		{
			const Instance& instance = Scene::instances[tree.elements[i]];

			instance.transform.inverseTransform( packet );
			packet.preCompute();

			// Check coherence and split packet only if it has become incoherent,
			// if not than the split is being done by the caller and we shouldn't handle it here
			if( !originalPacket.isCoherent || packet.isCoherent )
			{
				traceGeometryPacket( instance, packet, hit, activeMask4 );
			}
			else if( ( packet.xmask != originalPacket.xmask ) || 
				     ( packet.ymask != originalPacket.ymask ) || 
					 ( packet.zmask != originalPacket.zmask ) )
			{
				// TODO: UNTESTED CODE!!
				unsigned int qmask = 0;
				for( unsigned int b = 1, i = 0; i < 16; i++, b <<= 1 )
				{
					const int q = ((packet.xmask & b)?1:0) + ((packet.ymask & b)?2:0) + ((packet.zmask & b)?4:0);
					if( !( qmask & (1 << q) ) )
					{
						qmask |= 1 << q;
						std::fill_n( packet.mask, RT_PACKET_SIZE, 0 );
						for( unsigned int b1 = b, i1 = i; i1 < 16; i1++, b1 <<= 1 )
						{
							const int cq = ((packet.xmask & b1)?1:0) + ((packet.ymask & b1)?2:0) + ((packet.zmask & b1)?4:0);
							if( q == cq )
								packet.mask[i1] = originalPacket.mask[i1];
						}
						packet.isCoherent = false;
						s_dirSigns = &_rayDirSigns[q][0][0];
						traceGeometryPacket( instance, packet, hit, activeMask4 );
					}
				}
			}

			s_dirSigns = &_rayDirSigns[inQ][0][0];
			packet = originalPacket;
		}

		// Check early exit
		allHit = true;

		// TODO: very slow, need to improve this!
		for( int r = 0; r < RT_PACKET_SIZE; ++r )
		{
			const bool haveHit = hit.geom[r] != NULL;
			allHit &= haveHit;

			if( activeMask[r] == 0 )
				continue;

			// Update early termination
			done[r] |= activeMask[r] & ( ( hit.geom[r] != NULL ) ? 0xFFFFFFFF : 0 );
			activeMask[r] &= ~done[r];

			if( !haveHit )
				continue;

			// Setup state for shading
			setupShadingRay( shadeState.ray, packet, r );
			setupShadingHit( shadeState.hit, hit, r );
			shadeState.recursionDepth = rs.recursionDepth[r];
			Plugins::materials[hit.geom[r]->triDesc[hit.tId[r]].materialId]->shade( _TO_RT_STATE( shadeState ) );
			rs.resultColor[r] = shadeState.resultColor;
		}

		// Early ray termination
		if( allHit )
			return;

		if( s_instancePacketStack.empty() )
		{
			// TODO: very slow, need to improve this!
			for( int r = 0; r < RT_PACKET_SIZE; ++r )
			{
				// Skip if incoherent ray or if have hit (already computed shading for these)
				if( ( packet.mask[r] == 0 ) || ( hit.geom[r] != NULL ) )
					continue;

				// Setup state for shading
				setupShadingRay( shadeState.ray, packet, r );
				shadeState.recursionDepth = rs.recursionDepth[r];
				Plugins::environment->shade( _TO_RT_STATE( shadeState ) );
				rs.resultColor[r] = shadeState.resultColor;
			}
			return;
		}

		const PacketTraversalData& data = s_instancePacketStack.top();
		s_instancePacketStack.pop();

		// Deactivate rays that are done (found a valid hit and have already been shaded)
		node = data.node;
		packet.tnear4[0] = data.tnear4[0];
		packet.tfar4[0] = data.tfar4[0];
		activeMask4[0] = _mm_andnot_ps( done4[0], _mm_cmple_ps( data.tnear4[0], data.tfar4[0] ) );

		packet.tnear4[1] = data.tnear4[1];
		packet.tfar4[1] = data.tfar4[1];
		activeMask4[1] = _mm_andnot_ps( done4[1], _mm_cmple_ps( data.tnear4[1], data.tfar4[1] ) );

		packet.tnear4[2] = data.tnear4[2];
		packet.tfar4[2] = data.tfar4[2];
		activeMask4[2] = _mm_andnot_ps( done4[2], _mm_cmple_ps( data.tnear4[2], data.tfar4[2] ) );

		packet.tnear4[3] = data.tnear4[3];
		packet.tfar4[3] = data.tfar4[3];
		activeMask4[3] = _mm_andnot_ps( done4[3], _mm_cmple_ps( data.tnear4[3], data.tfar4[3] ) );
	}
}

// Disable pointer truncation warning
#pragma warning( disable : 4311 )

void RayTracer::traceGeometryPacket( const Instance& instance, RayPacket& packet, HitPacket& hit,
									 __m128 instActiveMask4[RT_PACKET_SIMD_SIZE] )
{
	const Geometry& geometry = Scene::geometries[instance.geometryId];
	const KdTree& tree = geometry.kdTree;

	// Active ray mask for geometry traversal and intersection
	union
	{ 
		unsigned int activeMask[RT_PACKET_SIZE];
		__m128       activeMask4[RT_PACKET_SIMD_SIZE];
	};

	// Clip ray packet against geometry bounding box
	// Disable rays that we don't need to trace with instActiveMask4 (incoherent rays were previously disabled)
	if( !clipRayPacket( tree.bbox, packet, instActiveMask4, activeMask4 ) )
		return;

	// Save previous best distances
	union
	{
		float  bestDist[RT_PACKET_SIZE];
		__m128 bestDist4[RT_PACKET_SIMD_SIZE];
	};
	std::copy( hit.dist, hit.dist + RT_PACKET_SIZE, bestDist );

	// Mask for early ray termination
	__m128 done4[RT_PACKET_SIMD_SIZE];

	// No need to trace rays that came disabled from instance (i.e. incoherent)
	// Additionally, no need to trace clipped rays as well
	for( int p = 0; p < RT_PACKET_SIMD_SIZE; ++p )
	{
		done4[p] = _mm_andnot_ps( activeMask4[p], rtu::SSE_ALL_ON );
	}

	// Mask for updating hit data
	union
	{
		__m128  mask4;
		__m128i imask4;
	};

	bool allHit;
	const KdNode* node = tree.root;
	PacketStack& s_geometryPacketStack = s_packetStacks[omp_get_thread_num()][GEOMETRY_STACK];
	s_geometryPacketStack.clear();

	while( true )
	{
		findLeafPacket( node, packet, s_geometryPacketStack, activeMask4 );

		if( node->elemCount() > 0 )
		{
			for( unsigned int i = node->elemStart(), limit = i + node->elemCount(); i < limit; ++i )
			{
				intersectPacket( geometry.triAccel[tree.elements[i]], packet, hit, bestDist4, activeMask4 );
			}

			// Check early exit
			allHit = true;

			// TODO: very slow, need to improve this!
			for( int p = 0; p < RT_PACKET_SIMD_SIZE; ++p )
			{
				// TODO: branch here? if( _mm_movemask_ps( activeMask4[r] ) == 0 ) continue;

				// If we found a hit in an active ray
				mask4 = _mm_and_ps( activeMask4[p], _mm_cmplt_ps( bestDist4[p], hit.dist4[p] ) );

				// TODO: branch here? if( _mm_movemask_ps( mask4 ) == 0 ) continue;

				// Update hit parameters
				hit.dist4[p] = _mm_or_ps( _mm_andnot_ps( mask4, hit.dist4[p] ), _mm_and_ps( mask4, bestDist4[p] ) );
				hit.inst4[p] = _mm_or_si128( _mm_andnot_si128( imask4, hit.inst4[p] ), _mm_and_si128( imask4, _mm_set1_epi32( (unsigned int)&instance ) ) );
				hit.geom4[p] = _mm_or_si128( _mm_andnot_si128( imask4, hit.geom4[p] ), _mm_and_si128( imask4, _mm_set1_epi32( (unsigned int)&geometry ) ) );

				// Update early termination
				done4[p] = _mm_or_ps( done4[p], mask4 );
				allHit &= ( _mm_movemask_ps( done4[p] ) == 0xF );
			}

			// Early ray termination
			if( allHit )
				return;
		}

		if( s_geometryPacketStack.empty() )
			return;

		const PacketTraversalData& data = s_geometryPacketStack.top();
		s_geometryPacketStack.pop();

		// Deactivate rays that found a valid hit
		node = data.node;
		packet.tnear4[0] = data.tnear4[0];
		packet.tfar4[0] = data.tfar4[0];
		activeMask4[0] = _mm_andnot_ps( done4[0], _mm_cmple_ps( data.tnear4[0], data.tfar4[0] ) );

		packet.tnear4[1] = data.tnear4[1];
		packet.tfar4[1] = data.tfar4[1];
		activeMask4[1] = _mm_andnot_ps( done4[1], _mm_cmple_ps( data.tnear4[1], data.tfar4[1] ) );

		packet.tnear4[2] = data.tnear4[2];
		packet.tfar4[2] = data.tfar4[2];
		activeMask4[2] = _mm_andnot_ps( done4[2], _mm_cmple_ps( data.tnear4[2], data.tfar4[2] ) );

		packet.tnear4[3] = data.tnear4[3];
		packet.tfar4[3] = data.tfar4[3];
		activeMask4[3] = _mm_andnot_ps( done4[3], _mm_cmple_ps( data.tnear4[3], data.tfar4[3] ) );
	}
}

// Enable pointer truncation warning
#pragma warning( default : 4311 )

// Returns how many rays hit any object
unsigned int RayTracer::traceHitPacket( rts::RTstate& state )
{
	// TODO: 
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Ray-kdtree traversal routines

void RayTracer::findLeafSingle( const KdNode*& node, Ray& ray, SingleStack& stack )
{
    while( !node->isLeaf() )
    {
        // Need to avoid NaN's when split - ray.origin[axis] == 0 and ray.invDir[axis] == +/- INF
        const unsigned int axis = node->axis();
        const float d = ( node->splitPos() - ray.origin[axis] ) * ray.invDir[axis];

        const unsigned int bit = ray.dirSigns[axis];

        const KdNode* const front = node->leftChild() + bit;
        const KdNode* const back = node->leftChild() + !bit;

        // Using < and > instead of <= and >= because of flat cells and triangles in the split plane.
        // In this case, we must traverse both children to guarantee that we hit the triangle we want.
        if( d < ray.tnear )
        {
            node = back;
        }
        else if( d > ray.tfar )
        {
            node = front;
        }
        else
        {
            stack.push();
            TraversalData& data = stack.top();

            // Store far child for later traversal
            data.node  = back;
            //data.tnear = d;
            data.tnear = ( d > ray.tnear ) ? d : ray.tnear; // avoid NaNs
            data.tfar  = ray.tfar;

            // Continue with front child
            node = front;
            //ray.tfar = d;
            ray.tfar = ( d < ray.tfar ) ? d : ray.tfar; // avoid NaNs
        }
    }
}

// Returns leaf in node
void RayTracer::findLeafPacket( const KdNode*& node, RayPacket& packet, PacketStack& stack,
					            __m128 activeMask4[RT_PACKET_SIMD_SIZE] )
{
	// TODO: from arauna_src
	// TODO: avoid nans
	// TODO: check if we can use mask4 inside ray packet or if we need commented out static global mask
	// TODO: try to use loops instead (see [Benthin] phd.pdf pg.62)
	while( !node->isLeaf() )
	{
		// Correct axis index to access direction signs (axis*2)
		unsigned int axis = node->axis() << 1;
		const __m128 splitPos4 = _mm_set_ps1( node->splitPos() );

		// Front and back children, according to packet direction signs
		const KdNode* const front = node->leftChild() + s_dirSigns[axis];
		const KdNode* const back  = node->leftChild() + s_dirSigns[axis + 1];

		// Correct axis index to access packet data (axis*4)
		axis <<= 1;

		// Compute split plane intersection with ray segments
		__m128 d4[4];
		d4[0] = _mm_mul_ps( _mm_sub_ps( splitPos4, packet.oa4[axis+0] ), packet.rda4[axis+0] );
		d4[1] = _mm_mul_ps( _mm_sub_ps( splitPos4, packet.oa4[axis+1] ), packet.rda4[axis+1] );
		d4[2] = _mm_mul_ps( _mm_sub_ps( splitPos4, packet.oa4[axis+2] ), packet.rda4[axis+2] );
		d4[3] = _mm_mul_ps( _mm_sub_ps( splitPos4, packet.oa4[axis+3] ), packet.rda4[axis+3] );

		// Original version, does not handle NaNs generated in d4 correctly!
		// Check if all rays go to back child
		unsigned int dnear = _mm_movemask_ps( _mm_and_ps( activeMask4[0], _mm_cmple_ps( packet.tnear4[0], d4[0] ) ) );
		dnear |= _mm_movemask_ps( _mm_and_ps( activeMask4[1], _mm_cmple_ps( packet.tnear4[1], d4[1] ) ) );
		dnear |= _mm_movemask_ps( _mm_and_ps( activeMask4[2], _mm_cmple_ps( packet.tnear4[2], d4[2] ) ) );
		dnear |= _mm_movemask_ps( _mm_and_ps( activeMask4[3], _mm_cmple_ps( packet.tnear4[3], d4[3] ) ) );
		node = back;
		if( dnear == 0 )
			continue; // traverse back child

		// Check if all rays go to front child
		unsigned int dfar = _mm_movemask_ps( _mm_and_ps( activeMask4[0], _mm_cmpge_ps( packet.tfar4[0], d4[0] ) ) );
		dfar  |= _mm_movemask_ps( _mm_and_ps( activeMask4[1], _mm_cmpge_ps( packet.tfar4[1], d4[1] ) ) );
		dfar  |= _mm_movemask_ps( _mm_and_ps( activeMask4[2], _mm_cmpge_ps( packet.tfar4[2], d4[2] ) ) );
		dfar  |= _mm_movemask_ps( _mm_and_ps( activeMask4[3], _mm_cmpge_ps( packet.tfar4[3], d4[3] ) ) );
		node = front;
		if( dfar == 0 )
			continue; // traverse front child

		// Push back child to stack and traverse front one
		// NaNs are handled by placing d4 in first operand of min and max
		stack.push();
		PacketTraversalData& data = stack.top();

		data.node = back;

		data.tfar4[0] = packet.tfar4[0];
		data.tnear4[0] = _mm_max_ps( d4[0], packet.tnear4[0] );
		packet.tfar4[0] = _mm_min_ps( d4[0], packet.tfar4[0] );
		activeMask4[0] = _mm_and_ps( activeMask4[0], _mm_cmple_ps( packet.tnear4[0], packet.tfar4[0] ) );

		data.tfar4[1] = packet.tfar4[1];
		data.tnear4[1] = _mm_max_ps( d4[1], packet.tnear4[1] );
		packet.tfar4[1] = _mm_min_ps( d4[1], packet.tfar4[1] );
		activeMask4[1] = _mm_and_ps( activeMask4[1], _mm_cmple_ps( packet.tnear4[1], packet.tfar4[1] ) );

		data.tfar4[2] = packet.tfar4[2];
		data.tnear4[2] = _mm_max_ps( d4[2], packet.tnear4[2] );
		packet.tfar4[2] = _mm_min_ps( d4[2], packet.tfar4[2] );
		activeMask4[2] = _mm_and_ps( activeMask4[2], _mm_cmple_ps( packet.tnear4[2], packet.tfar4[2] ) );

		data.tfar4[3] = packet.tfar4[3];
		data.tnear4[3] = _mm_max_ps( d4[3], packet.tnear4[3] );
		packet.tfar4[3] = _mm_min_ps( d4[3], packet.tfar4[3] );
		activeMask4[3] = _mm_and_ps( activeMask4[3], _mm_cmple_ps( packet.tnear4[3], packet.tfar4[3] ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// Ray-triangle intersection routines

// Intersect a single triangle with a single ray
void RayTracer::intersectSingle( const TriAccel& acc, const Ray& ray, Hit& hit, float& bestDistance )
{
	// Start high-latency division as early as possible
	const float nd = 1.0f / ( ray.direction[acc.k] + acc.n_u * ray.direction[KU] + acc.n_v * ray.direction[KV] );
	const float f = nd * ( acc.n_d - ray.origin[acc.k] - acc.n_u * ray.origin[KU] - acc.n_v * ray.origin[KV] );

	// Check for valid distance
	// TODO: find a correct way to get rid of these epsilons and check scene06 for any errors
	if( ( f >= bestDistance ) || ( f < ray.tnear - INTERSECT_EPSILON ) || ( f > ray.tfar + INTERSECT_EPSILON ) )
		return;

	// Compute hit point positions on uv plane
	const float hu = ray.origin[KU] + f * ray.direction[KU];
	const float hv = ray.origin[KV] + f * ray.direction[KV];

	// Check first barycentric coordinate
	const float lambda = hu * acc.b_nu + hv * acc.b_nv + acc.b_d;
	if( lambda < 0.0f )
		return;

	// Check second barycentric coordinate
	const float mue = hu * acc.c_nu + hv * acc.c_nv + acc.c_d;
	if( mue < 0.0f )
		return;

	// Check third barycentric coordinate
	const float psi = 1.0f - lambda - mue;
	if( psi < 0.0f )
		return;

	// Have a valid hit point here. Store it.
	bestDistance = f;
	hit.triangleId = acc.triangleId;
	hit.v0Coord = psi;
	hit.v1Coord = lambda;
	hit.v2Coord = mue;
}

// Intersect a single triangle with a packet of rays
void RayTracer::intersectPacket( const TriAccel& acc, const RayPacket& packet, HitPacket& hit, 
								 __m128 bestDist4[RT_PACKET_SIMD_SIZE], __m128 activeMask4[RT_PACKET_SIMD_SIZE] )
{
	union
	{
		__m128   mask4;
		__m128i imask4;
	};

	// Accurate axis index to access packet data (axis*4)
	const unsigned int qk = acc.k << 2;

	for( int p = 0; p < RT_PACKET_SIMD_SIZE; ++p )
	{
		if( _mm_movemask_ps( activeMask4[p] ) == 0 )
			continue;

		// TODO: testing fast reciprocal
		// Start high-latency division as early as possible
		const __m128 nd4 = rtu::sseFastRcp( _mm_add_ps( packet.da4[qk+p], 
			                                            _mm_add_ps( _mm_mul_ps( _mm_set_ps1( acc.n_u ), 
													                            packet.da4[QKU+p] ), 
			                                                        _mm_mul_ps( _mm_set_ps1( acc.n_v ), 
																                packet.da4[QKV+p] ) ) ) );

		// Compute intersection distance
		const __m128 f4 = _mm_mul_ps( nd4, 
			                          _mm_sub_ps( _mm_sub_ps( _mm_sub_ps( _mm_set_ps1( acc.n_d ), packet.oa4[qk+p] ), 
									                          _mm_mul_ps( _mm_set_ps1( acc.n_u ), packet.oa4[QKU+p] ) ), 
												  _mm_mul_ps( _mm_set_ps1( acc.n_v ), packet.oa4[QKV+p] ) ) );

		// TODO: find a correct way to get rid of these epsilons and check scene06 for any errors
		// Tolerances to near and far comparisons
		const __m128 ntol4 = _mm_sub_ps( packet.tnear4[p], SSE_INTERSECT_EPSILON );
		const __m128 ftol4 = _mm_add_ps( packet.tfar4[p], SSE_INTERSECT_EPSILON );

		// Check for valid distance
		mask4 = _mm_and_ps( _mm_and_ps( _mm_cmplt_ps( f4, bestDist4[p] ), 
			                            _mm_cmpge_ps( f4, ntol4 ) ), 
			                _mm_cmple_ps( f4, ftol4 ) );

		if( _mm_movemask_ps( mask4 ) == 0 )
			continue; // invalid distance

		// Compute hit point positions on uv plane
		const __m128 hu = _mm_add_ps( packet.oa4[QKU+p], _mm_mul_ps( f4, packet.da4[QKU+p] ) );
		const __m128 hv = _mm_add_ps( packet.oa4[QKV+p], _mm_mul_ps( f4, packet.da4[QKV+p] ) );

		// Compute first barycentric coordinate (lambda)
		const __m128 lambda4 = _mm_add_ps( _mm_set_ps1( acc.b_d ), 
			                               _mm_add_ps( _mm_mul_ps( hu, _mm_set_ps1( acc.b_nu ) ),
			                                           _mm_mul_ps( hv, _mm_set_ps1( acc.b_nv ) ) ) );
		mask4 = _mm_and_ps( mask4, _mm_cmpge_ps( lambda4, _mm_setzero_ps() ) );

		if( _mm_movemask_ps( mask4 ) == 0 )
			continue; // invalid lambda

		// Compute second barycentric coordinate (mue)
		const __m128 mue4 = _mm_add_ps( _mm_set_ps1( acc.c_d ), 
			                            _mm_add_ps( _mm_mul_ps( hu, _mm_set_ps1( acc.c_nu ) ),
			                                        _mm_mul_ps( hv, _mm_set_ps1( acc.c_nv ) ) ) );
		mask4 = _mm_and_ps( mask4, _mm_cmpge_ps( mue4, _mm_setzero_ps() ) );

		if( _mm_movemask_ps( mask4 ) == 0 )
			continue; // invalid mue

		// Compute third barycentric coordinate (psi)
		const __m128 psi4 = _mm_sub_ps( rtu::SSE_ONE, _mm_add_ps( lambda4, mue4 ) );
		mask4 = _mm_and_ps( mask4, _mm_cmpge_ps( psi4, _mm_setzero_ps() ) );

		if( _mm_movemask_ps( mask4 ) == 0 )
			continue; // invalid psi

		// Have a valid hit point here. Store it.
		bestDist4[p] = _mm_or_ps( _mm_andnot_ps( mask4, bestDist4[p] ), _mm_and_ps( mask4, f4 ) );
		hit.tId4[p] = _mm_or_si128( _mm_andnot_si128( imask4, hit.tId4[p] ), _mm_and_si128( imask4, _mm_set1_epi32( acc.triangleId ) ) );
		hit.v0c4[p] = _mm_or_ps( _mm_andnot_ps( mask4, hit.v0c4[p] ), _mm_and_ps( mask4, psi4 ) );
		hit.v1c4[p] = _mm_or_ps( _mm_andnot_ps( mask4, hit.v1c4[p] ), _mm_and_ps( mask4, lambda4 ) );
		hit.v2c4[p] = _mm_or_ps( _mm_andnot_ps( mask4, hit.v2c4[p] ), _mm_and_ps( mask4, mue4 ) );
	}
}

// Clip all rays in packet to given AABB
bool RayTracer::clipRayPacket( const AABB& bbox, RayPacket& p, const __m128 inputMask4[RT_PACKET_SIMD_SIZE],
				               __m128 outputMask4[RT_PACKET_SIMD_SIZE] )
{
	// xmin, xmax, ymin, ymax, zmin, zmax
	const __m128 bb[6] = { _mm_set_ps1( bbox.minv.x ), _mm_set_ps1( bbox.maxv.x ),
		                   _mm_set_ps1( bbox.minv.y ), _mm_set_ps1( bbox.maxv.y ),
		                   _mm_set_ps1( bbox.minv.z ), _mm_set_ps1( bbox.maxv.z ) };

	bool hit = false;

	for( int i = 0; i < RT_PACKET_SIMD_SIZE; ++i )
	{
		for( int a = 0; a < 6; a += 2 )
		{
			// Using reciprocal directions in first argument of min and max to filter them out in case of NaNs
			p.tnear4[i] = _mm_max_ps( _mm_mul_ps( _mm_sub_ps( bb[a + s_dirSigns[a]], p.oa4[a*2+i] ), p.rda4[a*2+i] ), p.tnear4[i] );
			p.tfar4[i]  = _mm_min_ps( _mm_mul_ps( _mm_sub_ps( bb[a + s_dirSigns[a+1]], p.oa4[a*2+i] ), p.rda4[a*2+i] ), p.tfar4[i] );
		}

		// TODO: check if we need this
		// If packet is incoherent, use active ray mask to write invalid tfar and disable incoherent rays
		//p.tfar4[r] = _mm_or_ps( _mm_andnot_ps( inputMask[r], rtu::SSE_ZERO ), _mm_and_ps( p.mask4[r], p.tfar4[r] ) );

		outputMask4[i] = _mm_and_ps( inputMask4[i], _mm_cmple_ps( p.tnear4[i], p.tfar4[i] ) );
		hit |= ( _mm_movemask_ps( outputMask4[i] ) != 0 );
	}
	return hit;
}

void RayTracer::setupShadingRay( Ray& ray, const RayPacket& packet, unsigned int r )
{
	ray.origin.set( packet.ox[r], packet.oy[r], packet.oz[r] );
	ray.direction.set( packet.dx[r], packet.dy[r], packet.dz[r] );
	ray.tnear = packet.tnear[r];
	ray.tfar = packet.tfar[r];
}

void RayTracer::setupShadingHit( Hit& hit, const HitPacket& hitp, unsigned int r )
{
	hit.distance = hitp.dist[r];
	hit.geometry = hitp.geom[r];
	hit.instance = hitp.inst[r];
	hit.triangleId = hitp.tId[r];
	hit.v0Coord = hitp.v0c[r];
	hit.v1Coord = hitp.v1c[r];
	hit.v2Coord = hitp.v2c[r];
}

/*
// TODO: the version above is faster on a P4 2.6Ghz HT, 1,5GB RAM DDR 400
// TODO: and also on an Athlon XP 3800+ 2.4Ghz, 2GB RAM DDR 400
void RayTracer::findLeafSingle( const KdNode*& node, Ray& ray, Stack<TraversalData>& stack )
{
	while( !node->isLeaf() )
	{
		// Need to avoid NaN's when split - ray.origin[axis] == 0 and ray.invDir[axis] == +/- INF
		const unsigned int axis = node->axis();
		const float d = ( node->splitPos() - ray.origin[axis] ) * ray.invDir[axis];

		const bool negative = ray.dirSigns[axis];

		const KdNode* const left  = node->leftChild();
		const KdNode* const right = left + 1;

		// Using < and > instead of <= and >= because of flat cells and triangles in the split plane.
		// Must traverse both children to guarantee that we hit the triangle we want.
		const bool backside  = d < ray.tnear; // case one, d <= t_near <= t_far -> cull front side
		const bool frontside = d > ray.tfar; // case two, t_near <= t_far <= d -> cull back side
		const bool skip      = backside | frontside;
		const bool other     = negative ^ frontside;

		// here's a denser version of what happens to the node.
		// node = skip ? (other ? left : right) : (negative ? right : left);
		if( !skip )
		{
			stack.push();
			TraversalData& data = stack.top();

			// Store far child for later traversal
			data.node  = negative ? left : right;
			//data.tnear = d;
			data.tnear = ( d > ray.tnear ) ? d : ray.tnear; // avoid NaNs
			data.tfar  = ray.tfar;

			// Continue with front child
			node = negative ? right : left;
			//ray.tfar = d;
			ray.tfar = ( d < ray.tfar ) ? d : ray.tfar; // avoid NaNs
		}
		else
		{
			node = other ? left : right;
		}
	}
}
*/

#undef KU
#undef KV

#undef QKU
#undef QKV

#undef _TO_RAY_STATE
#undef _TO_RAY_PACKET_STATE

} // namespace rtc
