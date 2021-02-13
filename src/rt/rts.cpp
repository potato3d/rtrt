#include <rt/rts.h>

#include <rtc/Plugins.h>
#include <rtc/Scene.h>

#include <rtc/RayTracer.h>
#include <rtc/RayState.h>

// TODO: remove
#include <rtu/random.h>

/************************************************************************/
/* Macros                                                               */
/************************************************************************/
#define _TO_RAY_STATE(s)       reinterpret_cast<rtc::RayState&>( (s) )
#define _TO_CONST_RAY_STATE(s) reinterpret_cast<const rtc::RayState&>( (s) )
#define _TO_LIGHT_ARRAY(s)     reinterpret_cast< void** >( (s) )
#define _TO_LIGHT_REF_PTR(s)   ( *reinterpret_cast< rtu::ref_ptr<rts::ILight>* >( (&s) ) )

#define _TO_RAY_PACKET_STATE(s)       reinterpret_cast<rtc::RayPacketState&>( (s) )
#define _TO_CONST_RAY_PACKET_STATE(s) reinterpret_cast<const rtc::RayPacketState&>( (s) )

/************************************************************************/
/* Global objects                                                       */
/************************************************************************/

// Core ray tracing
static rtc::RayTracer s_rayTracer;

/************************************************************************/
/* Shader Programming Interface                                         */
/************************************************************************/

//////////////////////////////////////////////////////////////////////////
// Helper functions to correctly initialize states

// Use camera to setup primary ray state.
// Initializes ray origin and direction.
// Resets ray recursion depth.
void rtsInitPrimaryRayState( rts::RTstate& state, float x, float y )
{
	rtc::Plugins::camera->getRay( state, x, y );

	// Reset ray state parameters
	rtc::RayState& rs = _TO_RAY_STATE( state );
	rs.recursionDepth = 0;
}

// Initialize state information for querying light radiance samples.
// Hit-position and shading normal must have been previously computed in state.
// TODO: shading normal is used to displace the position and avoid precision problems for shadow rays
void rtsInitLightState( const rts::RTstate& state, rts::RTstate& light )
{
	const rtc::RayState& rs = _TO_CONST_RAY_STATE( state );
	rtc::RayState& lig = _TO_RAY_STATE( light );

	lig.ray.direction = rs.ray.direction;
	// TODO: shoudn't need an epsilon here...
	lig.hitPosition = rs.hitPosition + rs.shadingNormal * rtc::Scene::rayEpsilon;
	lig.shadingNormal = rs.shadingNormal;
}

// Initialize state information for shadow rays.
// Given state must have been previously initialized as a light state.
// If surface points away from light, returns false and state is invalid.
// Else returns true and state is valid.
// Hit-position and shading normal must have been previously computed in light state.
bool rtsInitShadowRayState( const rtu::float3& directionTowardsLight, float rayMaxDistance, rts::RTstate& light )
{
	rtc::RayState& rs = _TO_RAY_STATE( light );

	// Check if surface does not point away from light
	const float nDotL = rs.shadingNormal.dot( directionTowardsLight );
	if( nDotL <= 0.0f )
		return false;

	// Set new ray parameters
	rs.ray.origin = rs.hitPosition;
	rs.ray.direction = directionTowardsLight;
	rs.ray.tfar = rayMaxDistance;
	return true;
}

// Initialize state information for reflection rays.
// Hit-position and shading normal must have been previously computed in state.
void rtsInitReflectionRayState( const rts::RTstate& state, rts::RTstate& reflection )
{
	const rtc::RayState& rs = _TO_CONST_RAY_STATE( state );
	rtc::RayState& ref = _TO_RAY_STATE( reflection );

	// Set new ray origin and direction
	ref.ray.origin = rs.hitPosition;
	rtsComputeReflectedDirection( rs.ray.direction, rs.shadingNormal, ref.ray.direction );

	// Increment recursion depth
	ref.recursionDepth = rs.recursionDepth + 1;
}

// Initialize state information for refraction rays.
// Uses shading normal to determine if ray is entering or exiting the object.
// Computes the critical angle between the medium index and the given refraction index.
// If there is total internal reflection, returns false and refraction state is invalid.
// Else returns true and refraction state is valid.
// Hit-position and shading normal must have been previously computed in state.
bool rtsInitRefractionRayState( const rts::RTstate& state, float refractionIndex, rts::RTstate& refraction )
{
	const rtc::RayState& rs = _TO_CONST_RAY_STATE( state );
	rtc::RayState& ref = _TO_RAY_STATE( refraction );

	// First, we need to determine if we are entering or exiting the surface we hit in order to get the correct 
	// normal orientation and ratio of refraction indexes.
	rtu::float3 normal;
	float n;

	// If front face
	if( rtsFrontFace( state ) )
	{
		// Entering the object
		normal = rs.shadingNormal;
		n = rtc::Scene::mediumRefractionIndex / refractionIndex;
	}
	else
	{
		// Exiting the object
		normal = -rs.shadingNormal;
		n = refractionIndex / rtc::Scene::mediumRefractionIndex;
	}
	
	// Get cosine of incident angle: need normalized ray direction
	rtu::float3 normalizedRayDir = rs.ray.direction;
	normalizedRayDir.normalize();
	const float cosI = -( normal.dot( normalizedRayDir ) );

	// Now compute transmitted angle and check for total internal reflection
	const float cosTsquare = 1.0f - n * n * ( 1.0f - cosI * cosI );
	if( cosTsquare <= 0.0f )
		return false;

	// Finally, compute transmitted ray direction and the other easy stuff
	const float cosT = sqrt( cosTsquare );

	ref.ray.direction.x = n * normalizedRayDir.x + ( n * cosI - cosT ) * normal.x;
	ref.ray.direction.y = n * normalizedRayDir.y + ( n * cosI - cosT ) * normal.y;
	ref.ray.direction.z = n * normalizedRayDir.z + ( n * cosI - cosT ) * normal.z;

	ref.ray.origin = rs.hitPosition;
	ref.recursionDepth = rs.recursionDepth + 1;
	return true;
}

/************************************************************************/
/* Packet versions                                                      */
/************************************************************************/

// Use camera to setup primary ray state.
// Initializes ray origin and direction.
// Resets ray recursion depth.
// Number of packet rays is given by RT_PACKET_SIZE
void rtsInitPrimaryRayStatePacket( rts::RTstate& state, float* rayXYCoords )
{
	rtc::Plugins::camera->getRayPacket( state, rayXYCoords );

	// Reset ray state parameters
	std::fill_n( _TO_RAY_PACKET_STATE( state ).recursionDepth, RT_PACKET_SIZE, 0 );
}

//////////////////////////////////////////////////////////////////////////
// Main ray-tracing functions

// Trace single primary ray
void rtsTraceRay( rts::RTstate& state )
{
	s_rayTracer.traceSingle( state );
	//s_rayTracer.bruteFroce( state );
}

// Trace single ray and only test for occlusion
bool rtsTraceHit( rts::RTstate& state )
{
	return s_rayTracer.traceHitSingle( state );
	//return s_rayTracer.bruteForceShadow( state );
}

/************************************************************************/
/* Packet versions                                                      */
/************************************************************************/

// Trace a ray packet using SIMD
void rtsTraceRayPacket( rts::RTstate& state )
{
	rtc::RayPacketState& rps = _TO_RAY_PACKET_STATE( state );
	rtc::RayPacket& packet = rps.packet;
	packet.preCompute();
	if( packet.isCoherent )
	{
		std::fill_n( packet.mask, RT_PACKET_SIZE, 0xFFFFFFFF );
		s_rayTracer.tracePacket( state, (packet.xmask & 1) + (packet.ymask & 2) + (packet.zmask & 4) );
	}
	else
	{
		//std::fill_n( rps.resultColor, RT_PACKET_SIZE, rtu::float3( 1.0f, 0.0f, 0.0f ) );
		//return;

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
						packet.mask[i1] = 0xFFFFFFFF;
				}
				packet.isCoherent = false;
				s_rayTracer.tracePacket( state, q );
			}
		}
		// TODO: why we need this?
		//std::fill_n( packet.mask, RT_PACKET_SIZE, 0xFFFFFFFF );
	}
}

//////////////////////////////////////////////////////////////////////////
// Store result in state for later use by rtsInit...State functions and other shaders

// Compute hit position from given state
rtu::float3& rtsComputeHitPosition( rts::RTstate& state )
{
	rtc::RayState& rs = _TO_RAY_STATE( state );
	rtc::Ray& ray = rs.ray;
	rtc::Hit& hit = rs.hit;

	rs.hitPosition.x = ray.origin.x + ray.direction.x * hit.distance;
	rs.hitPosition.y = ray.origin.y + ray.direction.y * hit.distance;
	rs.hitPosition.z = ray.origin.z + ray.direction.z * hit.distance;
	return rs.hitPosition;
}

// Compute interpolated shading normal
rtu::float3& rtsComputeShadingNormal( rts::RTstate& state )
{
	rtc::RayState& rs = _TO_RAY_STATE( state );
	const rtc::Hit& hit = _TO_RAY_STATE( state ).hit;

	const rtc::Geometry& geometry = *hit.geometry;

	const float v0Coord = hit.v0Coord;
	const float v1Coord = hit.v1Coord;
	const float v2Coord = hit.v2Coord;

	const rtc::TriDesc& triangle = geometry.triDesc[hit.triangleId];
	const rtu::float3& v0Normal = geometry.normals[triangle.v0];
	const rtu::float3& v1Normal = geometry.normals[triangle.v1];
	const rtu::float3& v2Normal = geometry.normals[triangle.v2];

	rs.shadingNormal.x = v0Normal.x*v0Coord + v1Normal.x*v1Coord + v2Normal.x*v2Coord;
	rs.shadingNormal.y = v0Normal.y*v0Coord + v1Normal.y*v1Coord + v2Normal.y*v2Coord;
	rs.shadingNormal.z = v0Normal.z*v0Coord + v1Normal.z*v1Coord + v2Normal.z*v2Coord;

	hit.instance->transform.transformNormal( rs.shadingNormal );
	rs.shadingNormal.normalize();
	return rs.shadingNormal;
}

//////////////////////////////////////////////////////////////////////////
// Don't store results

// Compute interpolated shading color
void rtsComputeShadingColor( const rts::RTstate& state, rtu::float3& color )
{
	const rtc::RayState& rs = _TO_CONST_RAY_STATE( state );
	const rtc::Hit& hit = rs.hit;

	const rtc::Geometry& geometry = *hit.geometry;

	const float v0Coord = hit.v0Coord;
	const float v1Coord = hit.v1Coord;
	const float v2Coord = hit.v2Coord;

	const rtc::TriDesc& triangle = geometry.triDesc[hit.triangleId];
	const rtu::float3& v0Color = geometry.colors[triangle.v0];
	const rtu::float3& v1Color = geometry.colors[triangle.v1];
	const rtu::float3& v2Color = geometry.colors[triangle.v2];

	color.x = v0Color.x*v0Coord + v1Color.x*v1Coord + v2Color.x*v2Coord;
	color.y = v0Color.y*v0Coord + v1Color.y*v1Coord + v2Color.y*v2Coord;
	color.z = v0Color.z*v0Coord + v1Color.z*v1Coord + v2Color.z*v2Coord;
}

// Compute interpolated texture coordinates
void rtsComputeTextureCoords( const rts::RTstate& state, rtu::float3& texCoords )
{
	const rtc::RayState& rs = _TO_CONST_RAY_STATE( state );
	const rtc::Hit& hit = rs.hit;

	const rtc::Geometry& geometry = *hit.geometry;

	const float v0Coord = hit.v0Coord;
	const float v1Coord = hit.v1Coord;
	const float v2Coord = hit.v2Coord;

	const rtc::TriDesc& triangle = geometry.triDesc[hit.triangleId];
	const rtu::float3& v0TexCoord = geometry.texCoords[triangle.v0];
	const rtu::float3& v1TexCoord = geometry.texCoords[triangle.v1];
	const rtu::float3& v2TexCoord = geometry.texCoords[triangle.v2];

	texCoords.x = v0TexCoord.x*v0Coord + v1TexCoord.x*v1Coord + v2TexCoord.x*v2Coord;
	texCoords.y = v0TexCoord.y*v0Coord + v1TexCoord.y*v1Coord + v2TexCoord.y*v2Coord;
	texCoords.z = v0TexCoord.z*v0Coord + v1TexCoord.z*v1Coord + v2TexCoord.z*v2Coord;
}

// Compute flat triangle normal (take the cross vector of two triangle edges)
void rtsComputeFlatNormal( const rts::RTstate& state, rtu::float3& normal )
{
	const rtc::RayState& rs = _TO_CONST_RAY_STATE( state );
	const rtc::Hit& hit = rs.hit;

	const rtc::Geometry& geometry = *hit.geometry;

	const rtc::TriDesc& triangle = geometry.triDesc[hit.triangleId];
	const rtu::float3& v0 = geometry.vertices[triangle.v0];
	const rtu::float3& v1 = geometry.vertices[triangle.v1];
	const rtu::float3& v2 = geometry.vertices[triangle.v2];

	normal = ( v1 - v0 ).cross( v2 - v0 );

	hit.instance->transform.transformNormal( normal );
	normal.normalize();
}

// Compute normalized vector for shading specular reflections.
// Uses ray origin as viewpoint.
// Hit-position and shading normal must have been previously computed in state.
void rtsComputeSpecularVector( const rts::RTstate& state, rtu::float3& specularVector )
{
	const rtc::RayState& rs = _TO_CONST_RAY_STATE( state );
	rtsComputeReflectedDirection( rs.ray.direction, rs.shadingNormal, specularVector );
	specularVector.normalize();
}

// Reflects a given incident direction over a normal vector
void rtsComputeReflectedDirection( const rtu::float3& incident, const rtu::float3& normal, rtu::float3& reflected )
{
	const float proj = incident.dot( normal ) * 2.0f;
	reflected.x = incident.x - normal.x * proj;
	reflected.y = incident.y - normal.y * proj;
	reflected.z = incident.z - normal.z * proj;
}

// Uses dot product between shading normal and ray direction to determine triangle facing
// Shading normal must have been previously computed in state.
bool rtsFrontFace( const rts::RTstate& state )
{
	const rtc::RayState& rs = _TO_CONST_RAY_STATE( state );
	return ( rs.shadingNormal.dot( rs.ray.direction ) <= 0.0f );
}

//////////////////////////////////////////////////////////////////////////
// Accessors for state attributes

// Get ray origin
rtu::float3& rtsRayOrigin( rts::RTstate& state )
{
	return _TO_RAY_STATE( state ).ray.origin;
}

// Get ray direction
rtu::float3& rtsRayDirection( rts::RTstate& state )
{
	return _TO_RAY_STATE( state ).ray.direction;
}

// Get resulting color stored in state
rtu::float3& rtsResultColor( rts::RTstate& state )
{
	return _TO_RAY_STATE( state ).resultColor;
}

// Hit-position must have been previously computed
rtu::float3& rtsHitPosition( rts::RTstate& state )
{
	return _TO_RAY_STATE( state ).hitPosition;
}

// Shading normal must have been previously computed
rtu::float3& rtsShadingNormal( rts::RTstate& state )
{
	return _TO_RAY_STATE( state ).shadingNormal;
}

// Get ray maximum distance
float& rtsRayMaxDistance( rts::RTstate& state )
{
	return _TO_RAY_STATE( state ).ray.tfar;
}

// Returns whether maximum ray recursion depth has been reached or not
bool rtsStopRayRecursion( const rts::RTstate& state )
{
	return _TO_CONST_RAY_STATE( state ).recursionDepth >= rtc::Scene::maxRayRecursionDepth;
}

// Get barycentric coordinates of hit
void rtsBarycentricCoords( const rts::RTstate& state, rtu::float3& coords )
{
	const rtc::RayState& rs = _TO_CONST_RAY_STATE( state );
	coords.set( rs.hit.v0Coord, rs.hit.v1Coord, rs.hit.v2Coord );
}

/************************************************************************/
/* Packet versions                                                      */
/************************************************************************/

// Set ray origin
void rtsSetRayOriginPacket( rts::RTstate& state, unsigned int ray, const rtu::float3& origin )
{
	rtc::RayPacketState& rps = _TO_RAY_PACKET_STATE( state );
	rps.packet.ox[ray] = origin.x;
	rps.packet.oy[ray] = origin.y;
	rps.packet.oz[ray] = origin.z;
}

// Set ray direction
void rtsSetRayDirectionPacket( rts::RTstate& state, unsigned int ray, const rtu::float3& direction )
{
	rtc::RayPacketState& rps = _TO_RAY_PACKET_STATE( state );
	rps.packet.dx[ray] = direction.x;
	rps.packet.dy[ray] = direction.y;
	rps.packet.dz[ray] = direction.z;
}

// Get resulting color stored in state
rtu::float3& rtsResultColorPacket( rts::RTstate& state, unsigned int ray )
{
	// TODO: use parallel code
	return _TO_RAY_PACKET_STATE( state ).resultColor[ray];
}

//////////////////////////////////////////////////////////////////////////
// Accessors for current ray-tracing context

// Get current viewport from camera
void rtsViewport( unsigned int& width, unsigned int& height )
{
	rtc::Plugins::camera->getViewport( width, height );
}

// Get current frame buffer from renderer
float* rtsFrameBuffer()
{
	return rtc::Scene::frameBuffer;
}

// Get array of global lights, returns light count (number of elements in array)
// If there are no more lights, return value will be zero, and pointer will be invalid.
int rtsGlobalLights( void**& lights )
{
	// Light 0 is default invalid light id.
	lights = _TO_LIGHT_ARRAY( &rtc::Plugins::lights[0] + 1 );
	return rtc::Plugins::lights.size() - 1;
}

// Compute given light's radiance contribution.
// If light does not illuminates state, returns false. Else returns true.
bool rtsIlluminate( rts::RTstate& state, void* light )
{
	return _TO_LIGHT_REF_PTR( light )->illuminate( state );
}

// Compute given texture's color contribution.
// Automatically uses pre-defined texture wrap modes, environment modes, filters, etc.
void rtsApplyTexture( rts::RTstate& state, unsigned int textureId )
{
	rtc::Plugins::textures[textureId]->shade( state );
}

#undef _TO_RAY_STATE
#undef _TO_CONST_RAY_STATE
#undef _TO_LIGHT_ARRAY
#undef _TO_LIGHT_REF_PTR

#undef _TO_RAY_PACKET_STATE
#undef _TO_CONST_RAY_PACKET_STATE
