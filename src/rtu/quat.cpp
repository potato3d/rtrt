//#include "precompiled.h"
#include <rtu/quat.h>

namespace rtu {

namespace
{
	template<typename real>
	RTU_FORCEINLINE void getRotation( const quat<real>& q, real& radians, real& x, real& y, real& z )
	{
		real sinHalfAngle = sqrt( q[0]*q[0] + q[1]*q[1] + q[2]*q[2] );

		radians = static_cast<real>( 2.0 ) * atan2( sinHalfAngle, q[3] );

		if( !math<real>::isEqual( sinHalfAngle, 0 ) )
		{
			real invSinHalfAngle = 1 / sinHalfAngle;
			x = q[0] * invSinHalfAngle;
			y = q[1] * invSinHalfAngle;
			z = q[2] * invSinHalfAngle;
		}
		else
		{
			x = 0;
			y = 0;
			z = 1;
		}
	}

	template<typename real>
	RTU_FORCEINLINE void makeRotation( quat<real>& q, real radians, real x, real y, real z )
	{
		real len = sqrt( x*x + y*y + z*z );

		if( len < math<real>::ZERO_TOLERANCE )
		{
			// ~zero length axis, so reset rotation to zero.
			q[0] = 0;
			q[1] = 0;
			q[2] = 0;
			q[3] = 1;
			return;
		}

		real inversenorm = static_cast<real>(1.0) / len;
		real coshalfradians = cos( static_cast<real>(0.5) * radians );
		real sinhalfradians = sin( static_cast<real>(0.5) * radians );

		q[0] = x * sinhalfradians * inversenorm;
		q[1] = y * sinhalfradians * inversenorm;
		q[2] = z * sinhalfradians * inversenorm;
		q[3] = coshalfradians;
	}

	/** Make a rotation Quaternion which will rotate vec1 to vec2

	This routine uses only fast geometric transforms, without costly acos/sin computations. 
	It's exact, fast, and with less degenerate cases than the acos/sin method.

	For an explanation of the math used, you may see for example: 
	http://logiciels.cnes.fr/MARMOTTES/marmottes-mathematique.pdf

	@note This is the rotation with shortest radians, which is the one equivalent to the 
	acos/sin transform method. Other rotations exists, for example to additionally keep 
	a local horizontal attitude.

	@author Nicolas Brodu
	*/
	template<typename real>
	RTU_FORCEINLINE void makeRotation( quat<real>& q, const real3<real>& from, const real3<real>& to )
	{
		// This routine takes any vector as argument but normalized 
		// vectors are necessary, if only for computing the dot product.
		// Too bad the API is that generic, it leads to performance loss.
		// Even in the case the 2 vectors are not normalized but same length,
		// the sqrt could be shared, but we have no way to know beforehand
		// at this point, while the caller may know.
		// So, we have to test... in the hope of saving at least a sqrt
		real3<real> sourceVector( from );
		real3<real> targetVector( to );

		sourceVector.normalize();
		targetVector.normalize();

		// Now let's get into the real stuff
		// Use "dot product plus one" as test as it can be re-used later on
		real dotProdPlus1 = static_cast<real>(1.0) + sourceVector.dot( targetVector );

		// Check for degenerate case of full u-turn. Use epsilon for detection
		if( dotProdPlus1 < math<real>::ZERO_TOLERANCE )
		{
			// Get an orthogonal vector of the given vector
			// in a plane with maximum vector coordinates.
			// Then use it as quaternion axis with pi radians
			// Trick is to realize one value at least is >0.6 for a normalized vector.
			if( abs( sourceVector.x ) < static_cast<real>(0.6) )
			{
				const real invnorm = static_cast<real>(1.0) / sqrt( static_cast<real>(1.0) - sourceVector.x*sourceVector.x );
				q[0] = 0; 
				q[1] = sourceVector.z * invnorm;
				q[2] = -sourceVector.y * invnorm;
				q[3] = 0;
			}
			else if( abs( sourceVector.y ) < static_cast<real>(0.6) )
			{
				const real invnorm = static_cast<real>(1.0) / sqrt( static_cast<real>(1.0) - sourceVector.y*sourceVector.y );
				q[0] = -sourceVector.z * invnorm;
				q[1] = 0;
				q[2] = sourceVector.x * invnorm;
				q[3] = 0;
			}
			else
			{
				const real invnorm = static_cast<real>(1.0) / sqrt( static_cast<real>(1.0) - sourceVector.z*sourceVector.z );
				q[0] = sourceVector.y * invnorm;
				q[1] = -sourceVector.x * invnorm;
				q[2] = 0;
				q[3] = 0;
			}
		}
		else
		{
			// Find the shortest radians quaternion that transforms normalized vectors
			// into one other. Formula is still valid when vectors are colinear
			const real s = sqrt( static_cast<real>(0.5) * dotProdPlus1 );
			const real3<real> tmp = sourceVector.cross( targetVector ) * ( static_cast<real>(1.0) / ( static_cast<real>(2.0) * s ) );
			q[0] = tmp.x;
			q[1] = tmp.y;
			q[2] = tmp.z;
			q[3] = s;
		}
	}

	template<typename real>
	RTU_FORCEINLINE void slerp( quat<real>& q, real t, const quat<real>& from, const quat<real>& to )
	{
		// Reference: Shoemake at SIGGRAPH 89
		// See also: http://www.gamasutra.com/features/programming/19980703/quaternions_01.htm
		real omega, cosomega, sinomega, scale_from, scale_to;

		quat<real> quatTo( to );

		// this is a dot product
		cosomega = from[0]*to[0] + from[1]*to[1] + from[2]*to[2] + from[3]*to[3];

		if( cosomega < math<real>::ZERO_TOLERANCE )
		{
			cosomega = -cosomega; 
			quatTo = -to;
		}

		if( ( static_cast<real>(1.0) - cosomega ) > math<real>::ZERO_TOLERANCE )
		{
			omega = acos( cosomega ) ;  // 0 <= omega <= Pi (see man acos)
			sinomega = sin( omega ) ;  // this sinomega should always be +ve so
			// could try sinomega=sqrt(1-cosomega*cosomega) to avoid a sin()?
			scale_from = sin( ( 1.0 - t ) * omega ) / sinomega;
			scale_to = sin( t * omega ) / sinomega;
		}
		else
		{
			/* --------------------------------------------------
			The ends of the vectors are very close
			we can use simple linear interpolation - no need
			to worry about the "spherical" interpolation
			-------------------------------------------------- */
			scale_from = static_cast<real>(1.0) - t;
			scale_to = t ;
		}

		// so that we get a Vec4
		q = ( from * scale_from ) + ( quatTo * scale_to );
	}
}

} // namespace rtu

/************************************************************************/
/* Exported 'Secret' Functions                                          */
/************************************************************************/

void _RTU_quat_getRotation( const rtu::quat<double>& q, double& radians, double& x, double& y, double& z )
{
	rtu::getRotation( q, radians, x, y, z );
}

void _RTU_quat_getRotation( const rtu::quat<float>& q, float& radians, float& x, float& y, float& z )
{
	rtu::getRotation( q, radians, x, y, z );
}

void _RTU_quat_makeRotation( rtu::quat<double>& q, double radians, double x, double y, double z )
{
	rtu::makeRotation( q, radians, x, y, z );
}

void _RTU_quat_makeRotation( rtu::quat<float>& q, float radians, float x, float y, float z )
{
	rtu::makeRotation( q, radians, x, y, z );
}

void _RTU_quat_makeRotation( rtu::quat<double>& q, const rtu::real3<double>& from, const rtu::real3<double>& to )
{
	rtu::makeRotation( q, from, to );
}

void _RTU_quat_makeRotation( rtu::quat<float>& q, const rtu::real3<float>& from, const rtu::real3<float>& to )
{
	rtu::makeRotation( q, from, to );
}

void _RTU_quat_slerp( rtu::quat<double>& q, double t, const rtu::quat<double>& from, const rtu::quat<double>& to )
{
	rtu::slerp( q, t, from, to );
}

void _RTU_quat_slerp( rtu::quat<float>& q, float t, const rtu::quat<float>& from, const rtu::quat<float>& to )
{
	rtu::slerp( q, t, from, to );
}
