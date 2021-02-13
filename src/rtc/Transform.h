#pragma once
#ifndef _RTC_TRANSFORM_H_
#define _RTC_TRANSFORM_H_

#include <rtu/common.h>
#include <rtc/Ray.h>
#include <rtu/float4x4.h>

namespace rtc {

class Transform
{
public:
	void setMatrix( const rtu::float4x4& matrix );

	void transformVertex( rtu::float3& vertex ) const;
	void transformNormal( rtu::float3& normal ) const;

	inline void inverseTransform( Ray& ray ) const;
	inline void inverseTransform( RayPacket& packet ) const;

	inline const rtu::float4x4& matrix();

private:
	rtu::float4x4 _matrix;
	rtu::float4x4 _inverseMatrix;
	// (M^T)^-1
	// Inverse of transposed matrix, first transpose then invert!
	rtu::float4x4 _inverseTransposedMatrix;
};

inline void Transform::inverseTransform( Ray& ray ) const
{
	// Transform ray origin
	_inverseMatrix.transform( ray.origin );

	// Transform ray direction without normalizing
	// Ignore translation part of matrix
	// Direction has w = 0 in homogeneous coordinates
	_inverseMatrix.transform3x3( ray.direction );
}

// TODO: must be a better/faster way of doing this!
inline void Transform::inverseTransform( RayPacket& packet ) const
{
	__m128 mat[4];
	__m128 tmp1;
	__m128 tmp2;
	rtu::floatSSE res;

	// Column-major
	mat[0] = _mm_set_ps( _inverseMatrix(0,0), _inverseMatrix(0,1), _inverseMatrix(0,2), _inverseMatrix(0,3) );
	mat[1] = _mm_set_ps( _inverseMatrix(1,0), _inverseMatrix(1,1), _inverseMatrix(1,2), _inverseMatrix(1,3) );
	mat[2] = _mm_set_ps( _inverseMatrix(2,0), _inverseMatrix(2,1), _inverseMatrix(2,2), _inverseMatrix(2,3) );
	mat[3] = _mm_set_ps( _inverseMatrix(3,0), _inverseMatrix(3,1), _inverseMatrix(3,2), _inverseMatrix(3,3) );

	for( int i = 0; i < RT_PACKET_SIZE; ++i )
	{
		// Transform origins
		tmp1 = _mm_mul_ps( mat[0], _mm_set_ps1( packet.ox[i] ) ); // column 0
		tmp2 = _mm_mul_ps( mat[1], _mm_set_ps1( packet.oy[i] ) ); // column 1
		tmp1 = _mm_add_ps( tmp1, tmp2 ); // add 1 to 0
		tmp2 = _mm_mul_ps( mat[2], _mm_set_ps1( packet.oz[i] ) ); // column 2
		tmp1 = _mm_add_ps( tmp1, tmp2 ); // add 2 to 0
		res.m = _mm_add_ps( tmp1, mat[3] ); // add 3 to 0, use column 3 directly because we assume w equals 1.0

		packet.ox[i] = res.x;
		packet.oy[i] = res.y;
		packet.oz[i] = res.z;

		// Transform directions
		tmp1 = _mm_mul_ps( mat[0], _mm_set_ps1( packet.dx[i] ) ); // column 0
		tmp2 = _mm_mul_ps( mat[1], _mm_set_ps1( packet.dy[i] ) ); // column 1
		tmp1 = _mm_add_ps( tmp1, tmp2 ); // add 1 to 0
		tmp2 = _mm_mul_ps( mat[2], _mm_set_ps1( packet.dz[i] ) ); // column 2
		res.m = _mm_add_ps( tmp1, tmp2 ); // add 2 to 0
		// don't use column 3 since direction has w equal to zero

		packet.dx[i] = res.x;
		packet.dy[i] = res.y;
		packet.dz[i] = res.z;
	}
}

inline const rtu::float4x4& Transform::matrix()
{
	return _matrix;
}

} // namespace rtc

#endif // _RTC_INSTANCE_H_
