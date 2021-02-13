#pragma once
#ifndef _RTS_H_
#define _RTS_H_

#include <rt/definitions.h>
#include <rts/RTstate.h>
#include <rtu/float3.h>
#include <rtu/float4x4.h>

/************************************************************************/
/* Shader Programming Interface                                         */
/************************************************************************/

//////////////////////////////////////////////////////////////////////////
// Helper functions to correctly initialize states

// Use camera to setup primary ray state.
// Initializes ray origin and direction.
// Resets ray recursion depth.
void rtsInitPrimaryRayState( rts::RTstate& state, float x, float y );

// Initialize state information for querying light radiance samples.
// Hit-position and shading normal must have been previously computed in state.
// TODO: shading normal is used to displace the position and avoid precision problems for shadow rays
void rtsInitLightState( const rts::RTstate& state, rts::RTstate& light );

// Initialize state information for shadow rays.
// Given state must have been previously initialized as a light state.
// If surface points away from light, returns false and state is invalid.
// Else returns true and state is valid.
// Hit-position and shading normal must have been previously computed in light state.
bool rtsInitShadowRayState( const rtu::float3& directionTowardsLight, float rayMaxDistance, rts::RTstate& light );

// Initialize state information for reflection rays.
// Hit-position and shading normal must have been previously computed in state.
void rtsInitReflectionRayState( const rts::RTstate& state, rts::RTstate& reflection );

// Initialize state information for refraction rays.
// Uses shading normal to determine if ray is entering or exiting the object.
// Computes the critical angle between the medium index and the given refraction index.
// If there is total internal reflection, returns false and refraction state is invalid.
// Else returns true and refraction state is valid.
// Hit-position and shading normal must have been previously computed in state.
bool rtsInitRefractionRayState( const rts::RTstate& state, float refractionIndex, rts::RTstate& refraction );

/************************************************************************/
/* Packet versions                                                      */
/************************************************************************/

// Use camera to setup primary ray state.
// Initializes ray origin and direction.
// Resets ray recursion depth.
// Number of packet rays is given by RT_PACKET_SIZE
void rtsInitPrimaryRayStatePacket( rts::RTstate& state, float* packetRays );

//////////////////////////////////////////////////////////////////////////
// Main ray-tracing functions

// Trace single primary ray
void rtsTraceRay( rts::RTstate& state );

// Trace single ray and only test for occlusion
bool rtsTraceHit( rts::RTstate& state );

/************************************************************************/
/* Packet versions                                                      */
/************************************************************************/

// Trace a ray packet using SIMD
void rtsTraceRayPacket( rts::RTstate& state );

//////////////////////////////////////////////////////////////////////////
// Store result in state for later use by other functions and shaders

// Compute hit position from given state
rtu::float3& rtsComputeHitPosition( rts::RTstate& state );

// Compute interpolated shading normal
rtu::float3& rtsComputeShadingNormal( rts::RTstate& state );

//////////////////////////////////////////////////////////////////////////
// Don't store results

// Compute interpolated shading color
void rtsComputeShadingColor( const rts::RTstate& state, rtu::float3& color );

// Compute interpolated texture coordinates
void rtsComputeTextureCoords( const rts::RTstate& state, rtu::float3& texCoords );

// Compute flat triangle normal (take the cross vector of two triangle edges)
void rtsComputeFlatNormal( const rts::RTstate& state, rtu::float3& normal );

// Compute normalized vector for shading specular reflections.
// Uses ray origin as viewpoint.
// Hit-position and shading normal must have been previously computed in state.
void rtsComputeSpecularVector( const rts::RTstate& state, rtu::float3& specularVector );

// Reflects a given input vector over a reference vector
void rtsComputeReflectedDirection( const rtu::float3& incident, const rtu::float3& normal, rtu::float3& reflected );

// Uses dot product between shading normal and ray direction to determine triangle facing
// Shading normal must have been previously computed in state.
bool rtsFrontFace( const rts::RTstate& state );

// TODO: other mathematical helper functions

//////////////////////////////////////////////////////////////////////////
// Accessors for state attributes

// Get ray origin
rtu::float3& rtsRayOrigin( rts::RTstate& state );

// Get ray direction
rtu::float3& rtsRayDirection( rts::RTstate& state );

// Get resulting color stored in state
rtu::float3& rtsResultColor( rts::RTstate& state );

// Hit-position must have been previously computed
rtu::float3& rtsHitPosition( rts::RTstate& state );

// Shading normal must have been previously computed
rtu::float3& rtsShadingNormal( rts::RTstate& state );

// Get ray maximum distance
float& rtsRayMaxDistance( rts::RTstate& state );

// Returns whether maximum ray recursion depth has been reached or not
bool rtsStopRayRecursion( const rts::RTstate& state );

// Get barycentric coordinates of hit
void rtsBarycentricCoords( const rts::RTstate& state, rtu::float3& coords );

/************************************************************************/
/* Packet versions                                                      */
/************************************************************************/

// Set ray origin
void rtsSetRayOriginPacket( rts::RTstate& state, unsigned int ray, const rtu::float3& origin );

// Set ray direction
void rtsSetRayDirectionPacket( rts::RTstate& state, unsigned int ray, const rtu::float3& direction );

// Get resulting color stored in state
rtu::float3& rtsResultColorPacket( rts::RTstate& state, unsigned int ray );

//////////////////////////////////////////////////////////////////////////
// Accessors for current ray-tracing context

// Get current viewport from camera
void rtsViewport( unsigned int& width, unsigned int& height );

// Get current frame buffer from renderer
float* rtsFrameBuffer();

// Get array of global lights, returns light count (number of elements in array)
// If there are no more lights, return value will be zero, and pointer will be invalid.
int rtsGlobalLights( void**& lights );

// Compute given light's radiance contribution.
// If light does not illuminates state, returns false. Else returns true.
bool rtsIlluminate( rts::RTstate& state, void* light );

// Compute given texture's color contribution and store it in resultColor.
// Automatically uses defined texture wrap modes, environment mode, filters, etc.
void rtsApplyTexture( rts::RTstate& state, unsigned int textureId );

#endif // _RTS_H_
