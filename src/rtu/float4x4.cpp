//#include "precompiled.h"
#include <rtu/float4x4.h>

namespace rtu {

namespace
{
	// Getters
	// Source: Gamasutra, Rotating Objects Using Quaternions
	//http://www.gamasutra.com/features/programming/19980703/quaternions_01.htm
	template<typename real>
	RTU_FORCEINLINE void getRotation( const rtu::real4x4<real>& r, quat<real>& q )
	{
		real tr, s;
		real tq[4];
		int32 i, j, k;

		int32 nxt[3] = { 1, 2, 0 };

		tr = r(0,0) + r(1,1) + r(2,2) + static_cast<real>(1.0);

		// check the diagonal
		if( tr > math<real>::ZERO_TOLERANCE )
		{
			s = sqrt( tr );
			q[3] = s / static_cast<real>(2.0);
			s = static_cast<real>(0.5) / s;
			q[0] = ( r(1,2) - r(2,1) ) * s;
			q[1] = ( r(2,0) - r(0,2) ) * s;
			q[2] = ( r(0,1) - r(1,0) ) * s;
		}
		else
		{
			// diagonal is negative
			i = 0;
			if( r(1,1) > r(0,0) )
				i = 1;
			if( r(2,2) > r(i,i) )
				i = 2;
			j = nxt[i];
			k = nxt[j];

			s = sqrt( ( r(i,i) - ( r(j,j) + r(k,k) ) ) + static_cast<real>(1.0) );

			tq[i] = s * static_cast<real>(0.5);

			if( s != 0 )
				s = static_cast<real>(0.5) / s;

			tq[3] = ( r(j,k) - r(k,j) ) * s;
			tq[j] = ( r(i,j) + r(j,i) ) * s;
			tq[k] = ( r(i,k) + r(k,i) ) * s;

			q[0] = tq[0];
			q[1] = tq[1];
			q[2] = tq[2];
			q[3] = tq[3];
		}
	}

	// Setters
	// Source: Gamasutra, Rotating Objects Using Quaternions
	//http://www.gamasutra.com/features/19980703/quaternions_01.htm
	template<typename real>
	void set( rtu::real4x4<real>& r, const quat<real>& q_in )
	{
		quat<real> q( q_in );
		real length2 = q.length2();
		if( !math<real>::isEqual( length2, 1 ) && !math<real>::isEqual( length2, 0 ) )
		{
			// normalize quat if required.
			q *= static_cast<real>(1.0) / sqrt( length2 );
		}

		real wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

		// calculate coefficients
		x2 = q[0] + q[0];
		y2 = q[1] + q[1];
		z2 = q[2] + q[2];

		xx = q[0] * x2;
		xy = q[0] * y2;
		xz = q[0] * z2;

		yy = q[1] * y2;
		yz = q[1] * z2;
		zz = q[2] * z2;

		wx = q[3] * x2;
		wy = q[3] * y2;
		wz = q[3] * z2;

		// Note.  Gamasutra gets the matrix assignments inverted, resulting
		// in left-handed rotations, which is contrary to OpenGL and OSG's 
		// methodology.  The matrix assignment has been altered in the next
		// few lines of code to do the right thing.
		// Don Burns - Oct 13, 2001
		r(0,0) = static_cast<real>(1.0) - ( yy + zz );
		r(1,0) = xy - wz;
		r(2,0) = xz + wy;
		r(3,0) = 0;

		r(0,1) = xy + wz;
		r(1,1) = static_cast<real>(1.0) - ( xx + zz );
		r(2,1) = yz - wx;
		r(3,1) = 0;

		r(0,2) = xz - wy;
		r(1,2) = yz + wx;
		r(2,2) = static_cast<real>(1.0) - ( xx + yy );
		r(3,2) = 0;

		r(0,3) = 0;
		r(1,3) = 0;
		r(2,3) = 0;
		r(3,3) = 1;
	}

	// Makes this a specific kind of matrix
	template<typename real>
	void makeIdentity( rtu::real4x4<real>& r )
	{
		r.setRow( 0, 1, 0, 0, 0 );
		r.setRow( 1, 0, 1, 0, 0 );
		r.setRow( 2, 0, 0, 1, 0 );
		r.setRow( 3, 0, 0, 0, 1 );
	}

	template<typename real>
	void makeScale( rtu::real4x4<real>& r, const rtu::real3<real>& scale )
	{
		r.makeScale( scale.x, scale.y, scale.z );
	}

	template<typename real>
	void makeScale( rtu::real4x4<real>& r, real x, real y, real z )
	{
		r.setRow( 0, x, 0, 0, 0 );
		r.setRow( 1, 0, y, 0, 0 );
		r.setRow( 2, 0, 0, z, 0 );
		r.setRow( 3, 0, 0, 0, 1 );
	}

	template<typename real>
	void makeTranslation( rtu::real4x4<real>& r, const rtu::real3<real>& trans )
	{
		r.makeTranslation( trans.x, trans.y, trans.z );
	}

	template<typename real>
	void makeTranslation( rtu::real4x4<real>& r, real x, real y, real z )
	{
		r.setRow( 0, 1, 0, 0, 0 );
		r.setRow( 1, 0, 1, 0, 0 );
		r.setRow( 2, 0, 0, 1, 0 );
		r.setRow( 3, x, y, z, 1 );
	}

	template<typename real>
	void makeRotation( rtu::real4x4<real>& r, const rtu::real3<real>& from, const rtu::real3<real>& to )
	{
		quat<real> q;
		q.makeRotation( from, to );
		r.set( q );
	}

	template<typename real>
	void makeRotation( rtu::real4x4<real>& r, real radians, const rtu::real3<real>& axis )
	{
		quat<real> q;
		q.makeRotation( radians, axis );
		r.set( q );
	}

	template<typename real>
	void makeRotation( rtu::real4x4<real>& r, real radians, real x, real y, float z )
	{
		quat<real> q;
		q.makeRotation( radians, x, y, z );
		r.set( q );
	}

	template<typename real>
	void makeRotation( rtu::real4x4<real>& r, real angle1, const rtu::real3<real>& axis1,  
										 real angle2, const rtu::real3<real>& axis2,  
										 real angle3, const rtu::real3<real>& axis3 )
	{
		quat<real> q;
		q.makeRotation( angle1, axis1, angle2, axis2, angle3, axis3 );
		r.set( q );
	}

	// OpenGL Matrices
	template<typename real>
	void makeOrtho( rtu::real4x4<real>& r, real left, real right, real bottom, real top, real zNear, real zFar )
	{
		// note transpose of Matrix_implementation wr.t OpenGL documentation, since the OSG use post multiplication rather than pre.
		real tx = -( right + left ) / ( right - left );
		real ty = -( top + bottom ) / ( top - bottom );
		real tz = -( zFar + zNear ) / ( zFar - zNear );
		r.setRow( 0, 2 / ( right - left ), 0, 0, 0 );
		r.setRow( 1, 0, 2 / ( top - bottom ), 0, 0 );
		r.setRow( 2, 0, 0, -2 / ( zFar - zNear ), 0 );
		r.setRow( 3, tx, ty, tz, 1 );
	}

	template<typename real>
	bool getOrtho( const rtu::real4x4<real>& r, real& left, real& right, real& bottom, real& top, real& zNear, real& zFar )
	{
		if( !math<real>::isEqual( r(0,3), 0 ) || !math<real>::isEqual( r(1,3), 0 ) || 
			!math<real>::isEqual( r(2,3), 0 ) || !math<real>::isEqual( r(3,3), 1 ) )
			return false;

		zNear = ( r(3,2) + static_cast<real>(1.0) ) / r(2,2);
		zFar = ( r(3,2) - static_cast<real>(1.0) ) / r(2,2);

		left = -( static_cast<real>(1.0) + r(3,0) ) / r(0,0);
		right = ( static_cast<real>(1.0) - r(3,0) ) / r(0,0);

		bottom = -( static_cast<real>(1.0) + r(3,1) ) / r(1,1);
		top = ( static_cast<real>(1.0) - r(3,1) ) / r(1,1);

		return true;
	}

	template<typename real>
	void makeOrtho2D( rtu::real4x4<real>& r, real left, real right, real bottom, real top )
	{
		r.makeOrtho( left, right, bottom, top, -1, 1 );
	}

	template<typename real>
	void makeFrustum( rtu::real4x4<real>& r, real left, real right, real bottom, real top, real zNear, real zFar )
	{
		// note transpose of Matrix_implementation wr.t OpenGL documentation, since the OSG use post multiplication rather than pre.
		real A = ( right + left ) / ( right - left );
		real B = ( top + bottom ) / ( top - bottom );
		real C = -( zFar + zNear ) / ( zFar - zNear );
		real D = -2.0 * zFar *zNear / ( zFar - zNear );
		r.setRow( 0, 2 * zNear / ( right - left ), 0, 0, 0 );
		r.setRow( 1, 0, 2 * zNear / ( top - bottom ), 0, 0 );
		r.setRow( 2, A, B, C, -1 );
		r.setRow( 3, 0, 0, D, 0 );
	}

	template<typename real>
	bool getFrustum( const rtu::real4x4<real>& r, real& left, real& right, real& bottom, real& top, real& zNear, real& zFar )
	{
		if( !math<real>::isEqual( r(0,3), 0 ) || !math<real>::isEqual( r(1,3), 0 ) || 
			!math<real>::isEqual( r(2,3), -1 ) || !math<real>::isEqual( r(3,3), 0 ) )
			return false;

		zNear = r(3,2) / ( r(2,2) - static_cast<real>(1.0) );
		zFar = r(3,2) / ( static_cast<real>(1.0) + r(2,2) );

		left = zNear * ( r(2,0) - static_cast<real>(1.0) ) / r(0,0);
		right = zNear * ( static_cast<real>(1.0) + r(2,0) ) / r(0,0);

		top = zNear * ( static_cast<real>(1.0) + r(2,1) ) / r(1,1);
		bottom = zNear * ( r(2,1) - static_cast<real>(1.0) ) / r(1,1);

		return true;
	}

	template<typename real>
	void makePerspective( rtu::real4x4<real>& r, real fovy, real aspectRatio, real zNear, real zFar )
	{
		// calculate the appropriate left, right etc.
		real tan_fovy = tan( math<real>::toRadians( fovy * static_cast<real>(0.5) ) );
		real right = tan_fovy *aspectRatio *zNear;
		real left = -right;
		real top = tan_fovy *zNear;
		real bottom = -top;
		r.makeFrustum( left, right, bottom, top, zNear, zFar );
	}

	template<typename real>
	bool getPerspective( const rtu::real4x4<real>& r, real& fovy, real& aspectRatio, real& zNear, real& zFar )
	{
		real right = 0;
		real left = 0;
		real top = 0;
		real bottom = 0;
		if( r.getFrustum( left, right, bottom, top, zNear, zFar ) )
		{
			fovy = math<real>::toDegrees( atan( top / zNear ) - atan( bottom / zNear ) );
			aspectRatio = ( right - left ) / ( top - bottom );
			return true;
		}
		return false;
	}

	template<typename real>
	void makeLookAt( rtu::real4x4<real>& r, const rtu::real3<real>& eye, const rtu::real3<real>& center, const rtu::real3<real>& up )
	{
		real3<real> f( center - eye );
		f.normalize();
		real3<real> s( f.cross(up) );
		s.normalize();
		real3<real> u( s.cross(f) );
		u.normalize();

		r.set( s[0], u[0], -f[0], 0, s[1], u[1], -f[1], 0, s[2], u[2], -f[2], 0, 0, 0, 0, 1 );

		real4x4<real> tmp;
		tmp.makeTranslation( -eye );
		r.product( tmp, r );
	}

	template<typename real>
	void getLookAt( const rtu::real4x4<real>& r, rtu::real3<real>& eye, rtu::real3<real>& center, rtu::real3<real>& up, real lookDistance = 1.0f )
	{
		real4x4<real> inv( r );
		inv.invert();
		eye.set( 0, 0, 0 );
		inv.transform( eye );
		up.set( 0, 1, 0 );
		r.transform3x3( up );
		center.set( 0, 0, -1 );
		r.transform3x3( center );
		center.normalize();
		center = eye + center * lookDistance;
	}

	// Matrix operations
	template<typename real>
	void transpose( rtu::real4x4<real>& r )
	{
		real temp = r(1,0);
		r(1,0) = r(0,1);
		r(0,1) = temp;

		temp = r(2,0);
		r(2,0) = r(0,2);
		r(0,2) = temp;

		temp = r(2,1);
		r(2,1) = r(1,2);
		r(1,2) = temp;

		temp = r(3,0);
		r(3,0) = r(0,3);
		r(0,3) = temp;

		temp = r(3,1);
		r(3,1) = r(1,3);
		r(1,3) = temp;

		temp = r(3,2);
		r(3,2) = r(2,3);
		r(2,3) = temp;
	}

	template<typename real>
	void invert( rtu::real4x4<real>& r )
	{
		real r00, r01, r02,
			r10, r11, r12,
			r20, r21, r22;

		real bkp1 = r(0,3);
		real bkp2 = r(1,3);
		real bkp3 = r(2,3);
		real bkp4 = r(3,0);
		real bkp5 = r(3,1);
		real bkp6 = r(3,2);

		// Copy rotation components directly into registers for speed
		r00 = r(0,0); r01 = r(0,1); r02 = r(0,2);
		r10 = r(1,0); r11 = r(1,1); r12 = r(1,2);
		r20 = r(2,0); r21 = r(2,1); r22 = r(2,2);

		// Partially compute inverse of rot
		r(0,0) = r11*r22 - r12*r21;
		r(0,1) = r02*r21 - r01*r22;
		r(0,2) = r01*r12 - r02*r11;

		// Compute determinant of rot from 3 elements just computed
		real one_over_det = static_cast<real>(1.0) / ( r00 * r(0,0) + r10 * r(0,1) + r20 * r(0,2) );
		r00 *= one_over_det; r10 *= one_over_det; r20 *= one_over_det;  // Saves on later computations

		// Finish computing inverse of rot
		r(0,0) *= one_over_det;
		r(0,1) *= one_over_det;
		r(0,2) *= one_over_det;
		r(0,3) = 0;
		r(1,0) = r12*r20 - r10*r22; // Have already been divided by det
		r(1,1) = r00*r22 - r02*r20; // same
		r(1,2) = r02*r10 - r00*r12; // same
		r(1,3) = 0;
		r(2,0) = r10*r21 - r11*r20; // Have already been divided by det
		r(2,1) = r01*r20 - r00*r21; // same
		r(2,2) = r00*r11 - r01*r10; // same
		r(2,3) = 0;

	#define d r22
		d = r(3,3);

		r(3,3) = 1;

		// We no longer need the rxx or det variables anymore, so we can reuse them for whatever we want.  
		// But we will still rename them for the sake of clarity.

		if( ( d - static_cast<real>(1.0) )*( d - static_cast<real>(1.0) ) > math<real>::ZERO_TOLERANCE )  // Involves perspective, so we must
		{
			// compute the full inverse

			real4x4<real> TPinv;
			r(3,0) = r(3,1) = r(3,2) = 0;

	#define px r00
	#define py r01
	#define pz r02
	#define one_over_s  one_over_det
	#define a  r10
	#define b  r11
	#define c  r12

			a = bkp1; b = bkp2; c = bkp3;
			px = r(0,0) * a + r(0,1) * b + r(0,2) * c;
			py = r(1,0) * a + r(1,1) * b + r(1,2) * c;
			pz = r(2,0) * a + r(2,1) * b + r(2,2) * c;

	#undef a
	#undef b
	#undef c
	#define tx r10
	#define ty r11
	#define tz r12

			tx = bkp4; ty = bkp5; tz = bkp6;
			one_over_s = static_cast<real>(1.0) / ( d - ( tx * px + ty * py + tz * pz ) );

			tx *= one_over_s; ty *= one_over_s; tz *= one_over_s;  // Reduces number of calculations later on

			// Compute inverse of trans*corr
			TPinv(0,0) = tx * px + static_cast<real>(1.0);
			TPinv(0,1) = ty * px;
			TPinv(0,2) = tz * px;
			TPinv(0,3) = -px * one_over_s;
			TPinv(1,0) = tx * py;
			TPinv(1,1) = ty * py + static_cast<real>(1.0);
			TPinv(1,2) = tz * py;
			TPinv(1,3) = -py * one_over_s;
			TPinv(2,0) = tx * pz;
			TPinv(2,1) = ty * pz;
			TPinv(2,2) = tz * pz + static_cast<real>(1.0);
			TPinv(2,3) = -pz * one_over_s;
			TPinv(3,0) = -tx;
			TPinv(3,1) = -ty;
			TPinv(3,2) = -tz;
			TPinv(3,3) = one_over_s;

			r.product( TPinv, r ); // Finish computing full inverse of mat

	#undef px
	#undef py
	#undef pz
	#undef one_over_s
	#undef d
		}
		else // Rightmost column is [0; 0; 0; 1] so it can be ignored
		{
			tx = bkp4; ty = bkp5; tz = bkp6;

			// Compute translation components of mat'
			r(3,0) = -( tx*r(0,0) + ty*r(1,0) + tz*r(2,0) );
			r(3,1) = -( tx*r(0,1) + ty*r(1,1) + tz*r(2,1) );
			r(3,2) = -( tx*r(0,2) + ty*r(1,2) + tz*r(2,2) );

	#undef tx
	#undef ty
	#undef tz
		}
	}

	template<typename real>
	void product( rtu::real4x4<real>& r, const rtu::real4x4<real>& a, const rtu::real4x4<real>& b )
	{
		real a00 = a(0,0), a01 = a(0,1), a02 = a(0,2), a03 = a(0,3);
		real a10 = a(1,0), a11 = a(1,1), a12 = a(1,2), a13 = a(1,3);
		real a20 = a(2,0), a21 = a(2,1), a22 = a(2,2), a23 = a(2,3);
		real a30 = a(3,0), a31 = a(3,1), a32 = a(3,2), a33 = a(3,3);

		real b00 = b(0,0), b01 = b(0,1), b02 = b(0,2), b03 = b(0,3);
		real b10 = b(1,0), b11 = b(1,1), b12 = b(1,2), b13 = b(1,3);
		real b20 = b(2,0), b21 = b(2,1), b22 = b(2,2), b23 = b(2,3);
		real b30 = b(3,0), b31 = b(3,1), b32 = b(3,2), b33 = b(3,3);

		if( &a == &r )
		{
			real t0, t1, t2, t3;
			for( uint32 row = 0; row < 4; ++row )
			{
				t0 = r(row,0)*b00 + r(row,1)*b10 + r(row,2)*b20 + r(row,3)*b30;
				t1 = r(row,0)*b01 + r(row,1)*b11 + r(row,2)*b21 + r(row,3)*b31;
				t2 = r(row,0)*b02 + r(row,1)*b12 + r(row,2)*b22 + r(row,3)*b32;
				t3 = r(row,0)*b03 + r(row,1)*b13 + r(row,2)*b23 + r(row,3)*b33;
				r.setRow( row, t0, t1, t2, t3 );
			}
			return;
		}

		if( &b == &r )
		{
			real t0, t1, t2, t3;
			for( uint32 col = 0; col < 4; ++col )
			{
				t0 = r(0,col)*a00 + r(1,col)*a01 + r(2,col)*a02 + r(3,col)*a03;
				t1 = r(0,col)*a10 + r(1,col)*a11 + r(2,col)*a12 + r(3,col)*a13;
				t2 = r(0,col)*a20 + r(1,col)*a21 + r(2,col)*a22 + r(3,col)*a23;
				t3 = r(0,col)*a30 + r(1,col)*a31 + r(2,col)*a32 + r(3,col)*a33;
				r(0,col) = t0;
				r(1,col) = t1;
				r(2,col) = t2;
				r(3,col) = t3;
			}
			return;
		}

		r(0,0) = a00*b00 + a01*b10 + a02*b20 + a03*b30;
		r(0,1) = a00*b01 + a01*b11 + a02*b21 + a03*b31;
		r(0,2) = a00*b02 + a01*b12 + a02*b22 + a03*b32;
		r(0,3) = a00*b03 + a01*b13 + a02*b23 + a03*b33;
		r(1,0) = a10*b00 + a11*b10 + a12*b20 + a13*b30;
		r(1,1) = a10*b01 + a11*b11 + a12*b21 + a13*b31;
		r(1,2) = a10*b02 + a11*b12 + a12*b22 + a13*b32;
		r(1,3) = a10*b03 + a11*b13 + a12*b23 + a13*b33;
		r(2,0) = a20*b00 + a21*b10 + a22*b20 + a23*b30;
		r(2,1) = a20*b01 + a21*b11 + a22*b21 + a23*b31;
		r(2,2) = a20*b02 + a21*b12 + a22*b22 + a23*b32;
		r(2,3) = a20*b03 + a21*b13 + a22*b23 + a23*b33;
		r(3,0) = a30*b00 + a31*b10 + a32*b20 + a33*b30;
		r(3,1) = a30*b01 + a31*b11 + a32*b21 + a33*b31;
		r(3,2) = a30*b02 + a31*b12 + a32*b22 + a33*b32;
		r(3,3) = a30*b03 + a31*b13 + a32*b23 + a33*b33;
	}
}

} // namespace rtu

/************************************************************************/
/* Exported 'Secret' Functions                                          */
/************************************************************************/
void _RTU_real4x4_getRotation( const rtu::real4x4<float>& r, rtu::quat<float>& q )
{
	rtu::getRotation( r, q );
}

void _RTU_real4x4_getRotation( const rtu::real4x4<double>& r, rtu::quat<double>& q )
{
	rtu::getRotation( r, q );
}

void _RTU_real4x4_set( rtu::real4x4<float>& r, const rtu::quat<float>& q_in )
{
	rtu::set( r, q_in );
}

void _RTU_real4x4_set( rtu::real4x4<double>& r, const rtu::quat<double>& q_in )
{
	rtu::set( r, q_in );
}

void _RTU_real4x4_makeIdentity( rtu::real4x4<float>& r )
{
	rtu::makeIdentity( r );
}

void _RTU_real4x4_makeIdentity( rtu::real4x4<double>& r )
{
	rtu::makeIdentity( r );
}

void _RTU_real4x4_makeScale( rtu::real4x4<float>& r, const rtu::real3<float>& scale )
{
	rtu::makeScale( r, scale );
}

void _RTU_real4x4_makeScale( rtu::real4x4<double>& r, const rtu::real3<double>& scale )
{
	rtu::makeScale( r, scale );
}

void _RTU_real4x4_makeScale( rtu::real4x4<float>& r, float x, float y, float z )
{
	rtu::makeScale( r, x, y, z );
}

void _RTU_real4x4_makeScale( rtu::real4x4<double>& r, double x, double y, double z )
{
	rtu::makeScale( r, x, y, z );
}

void _RTU_real4x4_makeTranslation( rtu::real4x4<float>& r, const rtu::real3<float>& trans )
{
	rtu::makeTranslation( r, trans );
}

void _RTU_real4x4_makeTranslation( rtu::real4x4<double>& r, const rtu::real3<double>& trans )
{
	rtu::makeTranslation( r, trans );
}

void _RTU_real4x4_makeTranslation( rtu::real4x4<float>& r, float x, float y, float z )
{
	rtu::makeTranslation( r, x, y, z );
}

void _RTU_real4x4_makeTranslation( rtu::real4x4<double>& r, double x, double y, double z )
{
	rtu::makeTranslation( r, x, y, z );
}

void _RTU_real4x4_makeRotation( rtu::real4x4<float>& r, const rtu::real3<float>& from, const rtu::real3<float>& to )
{
	rtu::makeRotation( r, from, to );
}

void _RTU_real4x4_makeRotation( rtu::real4x4<double>& r, const rtu::real3<double>& from, const rtu::real3<double>& to )
{
	rtu::makeRotation( r, from, to );
}

void _RTU_real4x4_makeRotation( rtu::real4x4<float>& r, float radians, const rtu::real3<float>& axis )
{
	rtu::makeRotation( r, radians, axis );
}

void _RTU_real4x4_makeRotation( rtu::real4x4<double>& r, double radians, const rtu::real3<double>& axis )
{
	rtu::makeRotation( r, radians, axis );
}

void _RTU_real4x4_makeRotation( rtu::real4x4<float>& r, float radians, float x, float y, float z )
{
	rtu::makeRotation( r, radians, x, y, z );
}

void _RTU_real4x4_makeRotation( rtu::real4x4<double>& r, double radians, double x, double y, float z )
{
	rtu::makeRotation( r, radians, x, y, z );
}

void _RTU_real4x4_makeRotation( rtu::real4x4<float>& r, float angle1, const rtu::real3<float>& axis1,  
							  float angle2, const rtu::real3<float>& axis2,  
							  float angle3, const rtu::real3<float>& axis3 )
{
	rtu::makeRotation( r, angle1, axis1, angle2, axis2, angle3, axis3 );
}

void _RTU_real4x4_makeRotation( rtu::real4x4<double>& r, double angle1, const rtu::real3<double>& axis1,  
							   double angle2, const rtu::real3<double>& axis2,  
							   double angle3, const rtu::real3<double>& axis3 )
{
	rtu::makeRotation( r, angle1, axis1, angle2, axis2, angle3, axis3 );
}

void _RTU_real4x4_makeOrtho( rtu::real4x4<float>& r, float left, float right, float bottom, float top, float zNear, float zFar )
{
	rtu::makeOrtho( r, left, right, bottom, top, zNear, zFar );
}

void _RTU_real4x4_makeOrtho( rtu::real4x4<double>& r, double left, double right, double bottom, double top, double zNear, double zFar )
{
	rtu::makeOrtho( r, left, right, bottom, top, zNear, zFar );
}

bool _RTU_real4x4_getOrtho( const rtu::real4x4<float>& r, float& left, float& right, float& bottom, float& top, float& zNear, float& zFar )
{
	return rtu::getOrtho( r, left, right, bottom, top, zNear, zFar );
}

bool _RTU_real4x4_getOrtho( const rtu::real4x4<double>& r, double& left, double& right, double& bottom, double& top, double& zNear, double& zFar )
{
	return rtu::getOrtho( r, left, right, bottom, top, zNear, zFar );
}

void _RTU_real4x4_makeOrtho2D( rtu::real4x4<float>& r, float left, float right, float bottom, float top )
{
	rtu::makeOrtho2D( r, left, right, bottom, top );
}

void _RTU_real4x4_makeOrtho2D( rtu::real4x4<double>& r, double left, double right, double bottom, double top )
{
	rtu::makeOrtho2D( r, left, right, bottom, top );
}

void _RTU_real4x4_makeFrustum( rtu::real4x4<float>& r, float left, float right, float bottom, float top, float zNear, float zFar )
{
	rtu::makeFrustum( r, left, right, bottom, top, zNear, zFar );
}

void _RTU_real4x4_makeFrustum( rtu::real4x4<double>& r, double left, double right, double bottom, double top, double zNear, double zFar )
{
	rtu::makeFrustum( r, left, right, bottom, top, zNear, zFar );
}

bool _RTU_real4x4_getFrustum( const rtu::real4x4<float>& r, float& left, float& right, float& bottom, float& top, float& zNear, float& zFar )
{
	return rtu::getFrustum( r, left, right, bottom, top, zNear, zFar );
}

bool _RTU_real4x4_getFrustum( const rtu::real4x4<double>& r, double& left, double& right, double& bottom, double& top, double& zNear, double& zFar )
{
	return rtu::getFrustum( r, left, right, bottom, top, zNear, zFar );
}

void _RTU_real4x4_makePerspective( rtu::real4x4<float>& r, float fovy, float aspectRatio, float zNear, float zFar )
{
	rtu::makePerspective( r, fovy, aspectRatio, zNear, zFar );
}

void _RTU_real4x4_makePerspective( rtu::real4x4<double>& r, double fovy, double aspectRatio, double zNear, double zFar )
{
	rtu::makePerspective( r, fovy, aspectRatio, zNear, zFar );
}

bool _RTU_real4x4_getPerspective( const rtu::real4x4<float>& r, float& fovy, float& aspectRatio, float& zNear, float& zFar )
{
	return rtu::getPerspective( r, fovy, aspectRatio, zNear, zFar );
}

bool _RTU_real4x4_getPerspective( const rtu::real4x4<double>& r, double& fovy, double& aspectRatio, double& zNear, double& zFar )
{
	return rtu::getPerspective( r, fovy, aspectRatio, zNear, zFar );
}

void _RTU_real4x4_makeLookAt( rtu::real4x4<float>& r, const rtu::real3<float>& eye, const rtu::real3<float>& center, const rtu::real3<float>& up )
{
	rtu::makeLookAt( r, eye, center, up );
}

void _RTU_real4x4_makeLookAt( rtu::real4x4<double>& r, const rtu::real3<double>& eye, const rtu::real3<double>& center, const rtu::real3<double>& up )
{
	rtu::makeLookAt( r, eye, center, up );
}

void _RTU_real4x4_getLookAt( const rtu::real4x4<float>& r, rtu::real3<float>& eye, rtu::real3<float>& center, rtu::real3<float>& up, float lookDistance )
{
	rtu::getLookAt( r, eye, center, up );
}

void _RTU_real4x4_getLookAt( const rtu::real4x4<double>& r, rtu::real3<double>& eye, rtu::real3<double>& center, rtu::real3<double>& up, double lookDistance )
{
	rtu::getLookAt( r, eye, center, up );
}

void _RTU_real4x4_transpose( rtu::real4x4<float>& r )
{
	rtu::transpose( r );
}

void _RTU_real4x4_transpose( rtu::real4x4<double>& r )
{
	rtu::transpose( r );
}

void _RTU_real4x4_invert( rtu::real4x4<float>& r )
{
	rtu::invert( r );
}

void _RTU_real4x4_invert( rtu::real4x4<double>& r )
{
	rtu::invert( r );
}

void _RTU_real4x4_product( rtu::real4x4<float>& r, const rtu::real4x4<float>& a, const rtu::real4x4<float>& b )
{
	rtu::product( r, a, b );
}

void _RTU_real4x4_product( rtu::real4x4<double>& r, const rtu::real4x4<double>& a, const rtu::real4x4<double>& b )
{
	rtu::product( r, a, b );
}
