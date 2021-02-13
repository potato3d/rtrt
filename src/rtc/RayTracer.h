#pragma once
#ifndef _RTC_RAYTRACER_H_
#define _RTC_RAYTRACER_H_

#include <rtu/common.h>
#include <rtc/RayState.h>
#include <rtc/KdTree.h>
#include <rtc/Triangle.h>
#include <rtc/Stack.h>
#include <rtc/Scene.h>
#include <rts/RTstate.h>

namespace rtc {

class RayTracer
{
public:
	static const unsigned int MAX_STACK_SIZE = 128;

	// Traversal stack information
	struct TraversalData
	{
		const KdNode* node;
		float tnear;
		float tfar;
		int pad;
	};

	struct PacketTraversalData
	{
		const KdNode* node; int pad1, pad2, pad3;
		union { float tnear[16]; __m128 tnear4[4]; };
		union { float tfar[16];  __m128 tfar4[4]; };
	};

	typedef StaticStack<TraversalData, MAX_STACK_SIZE>       SingleStack;
	typedef StaticStack<PacketTraversalData, MAX_STACK_SIZE> PacketStack;

	RayTracer();

	void bruteFroce( rts::RTstate& state );
	bool bruteForceShadow( rts::RTstate& state );

	//////////////////////////////////////////////////////////////////////////
	// Ray-kdtree traversal routines

	// Trace a single ray against the entire scene
	void traceSingle( rts::RTstate& state );
	void traceGeometrySingle( const Instance& instance, Ray& ray, Hit& hit );

	// Returns true if ray hits any object, false otherwise
	bool traceHitSingle( rts::RTstate& state );

	// Trace a bundle of rays against the entire scene
	void tracePacket( rts::RTstate& state, unsigned int q );
	void traceGeometryPacket( const Instance& instance, RayPacket& packet, HitPacket& hit, 
		                      __m128 instActiveMask4[RT_PACKET_SIMD_SIZE] );

	// Returns how many rays hit any object
	unsigned int traceHitPacket( rts::RTstate& state );

private:
	//////////////////////////////////////////////////////////////////////////
	// Ray-kdtree traversal routines

	// Returns leaf in node
    void findLeafSingle( const KdNode*& node, Ray& ray, SingleStack& stack );

	// Returns leaf in node
	//void findLeafPacket( const KdNode*& node, RayPacket& packet, PacketStack& stack,
	//	                 __m128 activeMask4[RT_PACKET_SIMD_SIZE] );

	void findLeafPacket( const KdNode*& node, RayPacket& packet, PacketStack& stack,
		                 __m128 activeMask4[RT_PACKET_SIMD_SIZE] );

	//////////////////////////////////////////////////////////////////////////
	// Ray-triangle intersection routines

	// Intersect a single triangle with a single ray
	void intersectSingle( const TriAccel& acc, const Ray& ray, Hit& hit, float& bestDistance );

	// Intersect a single triangle with a packet of rays
	void intersectPacket( const TriAccel& acc, const RayPacket& packet, HitPacket& hit, 
		                  __m128 bestDist4[RT_PACKET_SIMD_SIZE], __m128 activeMask4[RT_PACKET_SIMD_SIZE] );

	// Clip all rays in packet to given AABB
	bool clipRayPacket( const AABB& bbox, RayPacket& p, const __m128 inputMask4[RT_PACKET_SIMD_SIZE],
		                __m128 outputMask4[RT_PACKET_SIMD_SIZE] );

	void setupShadingRay( Ray& ray, const RayPacket& packet, unsigned int r );
	void setupShadingHit( Hit& hit, const HitPacket& hitp, unsigned int r );

	// Intersection
	unsigned int _modulo[5];
	unsigned int _quadModulo[5];
	/*
	 *	Ray Direction Signs used in bounding box clipping and determining near and far nodes
	 *	000 111
	 *	100 011
	 *	010 101
	 *	110 001
	 *	001 110
	 *	101 010
	 *	011 100
	 *	111 000
	 */
	unsigned int _rayDirSigns[8][3][2];
};

} // namespace rtc

#endif // _RTC_RAYTRACER_H_
