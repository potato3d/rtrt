#pragma once
#ifndef _RTC_AABB_H_
#define _RTC_AABB_H_

#include <rtu/common.h>
#include <rtc/SplitPlane.h>
#include <rtc/Ray.h>

namespace rtc {

struct AABB
{
	// TODO: inline critical methods
	// TODO: optimize implementations
	void buildFrom( const rtu::float3* vertices, unsigned int vertexCount );

	void expandBy( const rtu::float3* vertices, unsigned int vertexCount );
	void expandBy( const AABB& other );

	void clipTriangle( AABB& result, const rtu::float3& v0, const rtu::float3& v1, const rtu::float3& v2 ) const;
	void split( AABB& left, AABB& right, const SplitPlane& plane ) const;

	bool isPlanar( unsigned int dimension ) const;
	bool isDegenerate() const;
	float surfaceArea() const;

	// Returns false if ray segment is completely outside box. Returns true otherwise.
	// Only update ray if found valid clipping (return true).
	bool clipRay( Ray& ray ) const;

	// Geimer/Muller branchless version
	// TODO: not actually clipping, only testing for intersection.
	// TODO: we need to update ray interval!
	bool clipRayBranchless( Ray& ray ) const;

	// Version from tbp
	// TODO: very slow, perhaps loads are the problem...
	bool clipRaySSE( Ray& ray ) const;

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
	void computeVertices( rtu::float3* vertices ) const;

	rtu::float3 minv;
	rtu::float3 maxv;

private:
	void clip( unsigned int& vertexCount, float pos, float dir, unsigned int dim ) const;
};

} // namespace rtc

#endif // _RTC_AABB_H_
