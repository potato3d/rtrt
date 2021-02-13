#pragma once
#ifndef _RTU_real4X4_H_
#define _RTU_real4X4_H_

#include <rtu/common.h>
#include <rtu/float3.h>
#include <rtu/quat.h>
#include <memory>

namespace rtu {

template<typename real>
class real4x4
{
public:
	// Construction
	inline real4x4(); // Uninitialized
	inline real4x4( const real4x4<real>& other );
	inline explicit real4x4( real const * const ptr );
	inline real4x4( real a00, real a01, real a02, real a03,
					real a10, real a11, real a12, real a13,
					real a20, real a21, real a22, real a23,
					real a30, real a31, real a32, real a33 );

	// Analysis
	inline bool isNaN() const;
	inline bool isIdentity() const;

	// Assignment
	inline real4x4<real>& operator=( const real4x4<real>& other );

	// Getters
	inline real& operator()( int32 row, int32 col );
	inline real  operator()( int32 row, int32 col ) const;
	inline const real* ptr() const;

	inline void getTranslation( real3<real>& trans ) const;
	inline void getScale( real3<real>& getScale ) const;
	inline void getRotation( quat<real>& q ) const;

	// Setters
	inline void set( const quat<real>& q_in );
	inline void set( const real4x4<real>& other );
	inline void set( real const * const ptr );
	inline void set( real a00, real a01, real a02, real a03,
					 real a10, real a11, real a12, real a13,
					 real a20, real a21, real a22, real a23,
					 real a30, real a31, real a32, real a33 );

	// Only alter specific parts of this
	inline void setRow( int32 row, real scalar0, real scalar1, real scalar2, real scalar3 );
	inline void setTranslation( const real3<real>& trans );
	inline void setTranslation( real x, real y, real z );

	// Makes this a specific kind of matrix
	inline void makeIdentity();

	inline void makeScale( const real3<real>& scale );
	inline void makeScale( real x, real y, real z );

	inline void makeTranslation( const real3<real>& trans );
	inline void makeTranslation( real x, real y, real z );

	inline void makeRotation( const real3<real>& from, const real3<real>& to );
	inline void makeRotation( real radians, const real3<real>& axis );
	inline void makeRotation( real radians, real x, real y, real z );
	inline void makeRotation( real angle1, const real3<real>& axis1, 
							  real angle2, const real3<real>& axis2, 
							  real angle3, const real3<real>& axis3 );

	// OpenGL Matrices

	// Set to an orthographic projection.
	inline void makeOrtho( real left, real right, real bottom, real top, real zNear, real zFar );

	// Get the orthographic settings of the orthographic projection matrix.
	// Note, if matrix is not an orthographic matrix then invalid values will be returned.
	inline bool getOrtho( real& left, real& right, real& bottom, real& top, real& zNear, real& zFar ) const;

	// Set to a 2D orthographic projection.
	inline void makeOrtho2D( real left, real right, real bottom, real top );

	// Set to a generic perspective projection.
	inline void makeFrustum( real left, real right, real bottom, real top, real zNear, real zFar );

	// Get the frustum settings of a perspective projection matrix.
	// Note, if matrix is not a perspective matrix then invalid values will be returned.
	inline bool getFrustum( real& left, real& right, real& bottom, real& top, real& zNear, real& zFar ) const;

	// Set to a symmetrical perspective projection.
	// Aspect ratio is defined as width/height.
	inline void makePerspective( real fovy, real aspectRatio, real zNear, real zFar );

	// Get the frustum settings of a symmetric perspective projection matrix.
	// Return false if matrix is not a perspective matrix, where parameter values are undefined. 
	// Note, if matrix is not a symmetric perspective matrix then the shear will be lost.
	// Asymmetric matrices occur when stereo, power walls, caves and reality center display are used.
	// In these configuration one should use the frustum methods instead.
	inline bool getPerspective( real& fovy, real& aspectRatio, real& zNear, real& zFar ) const;

	// Set the position and orientation to be a view matrix, using the same convention as gluLookAt.
	inline void makeLookAt( const real3<real>& eye, const real3<real>& center, const real3<real>& up );

	// Get to the position and orientation of a modelview matrix, using the same convention as gluLookAt.
	inline void getLookAt( real3<real>& eye, real3<real>& center, real3<real>& up, real lookDistance = 1.0f ) const;

	// Arithmetic operations
	inline real4x4<real> operator*( real scalar ) const;
	inline real4x4<real> operator-() const;

	// Arithmetic updates
	inline real4x4<real>& operator*=( real scalar );

	// Matrix operations
	inline void transpose();
	inline void invert();

	// Assumes this is a rigid-body transformation.
	// Avoids expensive matrix inversion by transposing 3x3 sub-matrix and inverting the translation vector.
	inline void invertRBT();

	// Assumes this is a rotation matrix.
	// Ortho-normalize the 3x3 rotation and scale matrix.
	inline void orthoNormalize();

	// Apply this transformation to v
	inline void transform( real3<real>& v, real w = 1 ) const;
	inline void transposedTransform( real3<real>& v, real w = 1 ) const;

	// Apply only the 3x3 transformation to v
	inline void transform3x3( real3<real>& v ) const;
	inline void transposedTransform3x3( real3<real>& v ) const;

	// Sets this as the result of the product of a and b <this = a * b>
	inline void product( const real4x4<real>& a, const real4x4<real>& b );
	
private:
	real _m[4][4];
};

// Construction
template<typename real>
real4x4<real>::real4x4()
{
	// Do not initialize for performance!
}

template<typename real>
real4x4<real>::real4x4( const real4x4<real>& other )
{
	set( other.ptr() );
}

template<typename real>
real4x4<real>::real4x4( real const * const ptr )
{
	set( ptr );
}

template<typename real>
real4x4<real>::real4x4( real a00, real a01, real a02, real a03,  
						  real a10, real a11, real a12, real a13,  
						  real a20, real a21, real a22, real a23,  
						  real a30, real a31, real a32, real a33 )
{
	setRow( 0, a00, a01, a02, a03 );
	setRow( 1, a10, a11, a12, a13 );
	setRow( 2, a20, a21, a22, a23 );
	setRow( 3, a30, a31, a32, a33 );
}

// Analysis
template<typename real>
bool real4x4<real>::isNaN() const
{
	return math<real>::isNaN( _m[0][0] ) ||
		   math<real>::isNaN( _m[0][1] ) ||
		   math<real>::isNaN( _m[0][2] ) ||
		   math<real>::isNaN( _m[0][3] ) ||
		   math<real>::isNaN( _m[1][0] ) ||
		   math<real>::isNaN( _m[1][1] ) ||
		   math<real>::isNaN( _m[1][2] ) ||
		   math<real>::isNaN( _m[1][3] ) ||
		   math<real>::isNaN( _m[2][0] ) ||
		   math<real>::isNaN( _m[2][1] ) ||
		   math<real>::isNaN( _m[2][2] ) ||
		   math<real>::isNaN( _m[2][3] ) ||
		   math<real>::isNaN( _m[3][0] ) ||
		   math<real>::isNaN( _m[3][1] ) ||
		   math<real>::isNaN( _m[3][2] ) ||
		   math<real>::isNaN( _m[3][3] );
}

template<typename real>
bool real4x4<real>::isIdentity() const
{
	return ( _m[0][0] == 1 ) && ( _m[0][1] == 0 ) && ( _m[0][2] == 0 ) && ( _m[0][3] == 0 ) &&
		   ( _m[1][0] == 0 ) && ( _m[1][1] == 1 ) && ( _m[1][2] == 0 ) && ( _m[1][3] == 0 ) &&
		   ( _m[2][0] == 0 ) && ( _m[2][1] == 0 ) && ( _m[2][2] == 1 ) && ( _m[2][3] == 0 ) &&
		   ( _m[3][0] == 0 ) && ( _m[3][1] == 0 ) && ( _m[3][2] == 0 ) && ( _m[3][3] == 1 );
}

// Assignment
template<typename real>
real4x4<real>& real4x4<real>::operator=( const real4x4<real>& other )
{
	if( &other == this )
		return *this;
	set( other.ptr() );
	return *this;
}

// Getters
template<typename real>
real& real4x4<real>::operator()( int32 row, int32 col )
{
	return _m[row][col];
}

template<typename real>
real real4x4<real>::operator()( int32 row, int32 col ) const
{
	return _m[row][col];
}

template<typename real>
const real* real4x4<real>::ptr() const
{
	return reinterpret_cast<const real*>(_m);
}

template<typename real>
void real4x4<real>::getTranslation( real3<real>& trans ) const
{
	trans.set( _m[3] );
}

template<typename real>
void real4x4<real>::getScale( real3<real>& scale ) const
{
	scale.set( sqrt( _m[0][0]*_m[0][0] + _m[1][0]*_m[1][0] + _m[2][0]*_m[2][0] ),
			   sqrt( _m[0][1]*_m[0][1] + _m[1][1]*_m[1][1] + _m[2][1]*_m[2][1] ),
			   sqrt( _m[0][2]*_m[0][2] + _m[1][2]*_m[1][2] + _m[2][2]*_m[2][2] ) );
}

template<typename real>
void real4x4<real>::getRotation( quat<real>& q ) const
{
	void _RTU_real4x4_getRotation( const real4x4<real>& r, quat<real>& q );
	_RTU_real4x4_getRotation( *this, q );
}

// Setters
template<typename real>
void real4x4<real>::set( const quat<real>& q_in )
{
	void _RTU_real4x4_set( real4x4<real>& r, const quat<real>& q_in );
	_RTU_real4x4_set( *this, q_in );
}

template<typename real>
void real4x4<real>::set( const real4x4<real>& other )
{
	set( other.ptr() );
}

template<typename real>
void real4x4<real>::set( real const * const ptr )
{
	memcpy( reinterpret_cast<real*>(_m), ptr, 16*sizeof(real) );
}

template<typename real>
void real4x4<real>::set( real a00, real a01, real a02, real a03,  
						     real a10, real a11, real a12, real a13,  
							 real a20, real a21, real a22, real a23,  
							 real a30, real a31, real a32, real a33 )
{
	setRow( 0, a00, a01, a02, a03 );
	setRow( 1, a10, a11, a12, a13 );
	setRow( 2, a20, a21, a22, a23 );
	setRow( 3, a30, a31, a32, a33 );
}

// Only alter specific parts of this
template<typename real>
void real4x4<real>::setRow( int32 row, real scalar0, real scalar1, real scalar2, real scalar3 )
{
	_m[row][0] = scalar0;
	_m[row][1] = scalar1;
	_m[row][2] = scalar2;
	_m[row][3] = scalar3;
}

template<typename real>
void real4x4<real>::setTranslation( const real3<real>& trans )
{
	setTranslation( trans.x, trans.y, trans.z );
}

template<typename real>
void real4x4<real>::setTranslation( real x, real y, real z )
{
	_m[3][0] = x;
	_m[3][1] = y;
	_m[3][2] = z;
}

// Makes this a specific kind of matrix
template<typename real>
void real4x4<real>::makeIdentity()
{
	void _RTU_real4x4_makeIdentity( rtu::real4x4<real>& r );
	_RTU_real4x4_makeIdentity( *this );
}

template<typename real>
void real4x4<real>::makeScale( const real3<real>& scale )
{
	void _RTU_real4x4_makeScale( rtu::real4x4<real>& r, const rtu::real3<real>& scale );
	_RTU_real4x4_makeScale( *this, scale );
}

template<typename real>
void real4x4<real>::makeScale( real x, real y, real z )
{
	void _RTU_real4x4_makeScale( rtu::real4x4<real>& r, real x, real y, real z );
	_RTU_real4x4_makeScale( *this, x, y, z );
}

template<typename real>
void real4x4<real>::makeTranslation( const real3<real>& trans )
{
	void _RTU_real4x4_makeTranslation( rtu::real4x4<real>& r, const rtu::real3<real>& trans );
	_RTU_real4x4_makeTranslation( *this, trans );
}

template<typename real>
void real4x4<real>::makeTranslation( real x, real y, real z )
{
	void _RTU_real4x4_makeTranslation( rtu::real4x4<real>& r, real x, real y, real z );
	_RTU_real4x4_makeTranslation( *this, x, y, z );
}

template<typename real>
void real4x4<real>::makeRotation( const real3<real>& from, const real3<real>& to )
{
	void _RTU_real4x4_makeRotation( rtu::real4x4<real>& r, const rtu::real3<real>& from, const rtu::real3<real>& to );
	_RTU_real4x4_makeRotation( *this, from, to );
}

template<typename real>
void real4x4<real>::makeRotation( real radians, const real3<real>& axis )
{
	void _RTU_real4x4_makeRotation( rtu::real4x4<real>& r, real radians, const rtu::real3<real>& axis );
	_RTU_real4x4_makeRotation( *this, radians, axis );
}

template<typename real>
void real4x4<real>::makeRotation( real radians, real x, real y, real z )
{
	makeRotation( radians, real3<real>( x, y, z ) );
}

template<typename real>
void real4x4<real>::makeRotation( real angle1, const real3<real>& axis1,  
									 real angle2, const real3<real>& axis2,  
									 real angle3, const real3<real>& axis3 )
{
	void _RTU_real4x4_makeRotation( rtu::real4x4<real>& r, real angle1, const rtu::real3<real>& axis1,  
								   real angle2, const rtu::real3<real>& axis2,  
								   real angle3, const rtu::real3<real>& axis3 );
	_RTU_real4x4_makeRotation( *this, angle1, axis1, angle2, axis2, angle3, axis3 );
}

// OpenGL Matrices
template<typename real>
void real4x4<real>::makeOrtho( real left, real right, real bottom, real top, real zNear, real zFar )
{
	void _RTU_real4x4_makeOrtho( rtu::real4x4<real>& r, real left, real right, real bottom, real top, real zNear, real zFar );
	_RTU_real4x4_makeOrtho( *this, left, right, bottom, top, zNear, zFar );
}

template<typename real>
bool real4x4<real>::getOrtho( real& left, real& right, real& bottom, real& top, real& zNear, real& zFar ) const
{
	bool _RTU_real4x4_getOrtho( const rtu::real4x4<real>& r, real& left, real& right, real& bottom, real& top, real& zNear, real& zFar );
	return _RTU_real4x4_getOrtho( *this, left, right, bottom, top, zNear, zFar );
}

template<typename real>
void real4x4<real>::makeOrtho2D( real left, real right, real bottom, real top )
{
	void _RTU_real4x4_makeOrtho2D( rtu::real4x4<real>& r, real left, real right, real bottom, real top );
	_RTU_real4x4_makeOrtho2D( *this, left, right, bottom, top );
}

template<typename real>
void real4x4<real>::makeFrustum( real left, real right, real bottom, real top, real zNear, real zFar )
{
	void _RTU_real4x4_makeFrustum( rtu::real4x4<real>& r, real left, real right, real bottom, real top, real zNear, real zFar );
	_RTU_real4x4_makeFrustum( *this, left, right, bottom, top, zNear, zFar );
}

template<typename real>
bool real4x4<real>::getFrustum( real& left, real& right, real& bottom, real& top, real& zNear, real& zFar ) const
{
	bool _RTU_real4x4_getFrustum( const rtu::real4x4<real>& r, real& left, real& right, real& bottom, real& top, real& zNear, real& zFar );
	return _RTU_real4x4_getFrustum( *this, left, right, bottom, top, zNear, zFar );
}

template<typename real>
void real4x4<real>::makePerspective( real fovy, real aspectRatio, real zNear, real zFar )
{
	void _RTU_real4x4_makePerspective( rtu::real4x4<real>& r, real fovy, real aspectRatio, real zNear, real zFar );
	_RTU_real4x4_makePerspective( *this, fovy, aspectRatio, zNear, zFar );
}

template<typename real>
bool real4x4<real>::getPerspective( real& fovy, real& aspectRatio, real& zNear, real& zFar ) const
{
	bool _RTU_real4x4_getPerspective( const rtu::real4x4<real>& r, real& fovy, real& aspectRatio, real& zNear, real& zFar );
	return _RTU_real4x4_getPerspective( *this, fovy, aspectRatio, zNear, zFar );
}

template<typename real>
void real4x4<real>::makeLookAt( const real3<real>& eye, const real3<real>& center, const real3<real>& up )
{
	void _RTU_real4x4_makeLookAt( rtu::real4x4<real>& r, const rtu::real3<real>& eye, const rtu::real3<real>& center, const rtu::real3<real>& up );
	_RTU_real4x4_makeLookAt( *this, eye, center, up );
}

template<typename real>
void real4x4<real>::getLookAt( real3<real>& eye, real3<real>& center, real3<real>& up, real lookDistance ) const
{
	void _RTU_real4x4_getLookAt( const rtu::real4x4<real>& r, rtu::real3<real>& eye, rtu::real3<real>& center, rtu::real3<real>& up, real lookDistance );
	_RTU_real4x4_getLookAt( *this, eye, center, up, lookDistance );
}

// Matrix operations
template<typename real>
void real4x4<real>::transpose()
{
	void _RTU_real4x4_transpose( rtu::real4x4<real>& r );
	_RTU_real4x4_transpose( *this );
}

template<typename real>
void real4x4<real>::invert()
{
	void _RTU_real4x4_invert( rtu::real4x4<real>& r );
	_RTU_real4x4_invert( *this );
}

template<typename real>
void real4x4<real>::product( const real4x4<real>& a, const real4x4<real>& b )
{
	void _RTU_real4x4_product( rtu::real4x4<real>& r, const rtu::real4x4<real>& a, const rtu::real4x4<real>& b );
	_RTU_real4x4_product( *this, a, b );
}

// Arithmetic operations
template<typename real>
real4x4<real> real4x4<real>::operator*( real scalar ) const
{
	return real4x4<real>( _m[0][0]*scalar, _m[0][1]*scalar, _m[0][2]*scalar, _m[0][3]*scalar,
						  _m[1][0]*scalar, _m[1][1]*scalar, _m[1][2]*scalar, _m[1][3]*scalar,
						  _m[2][0]*scalar, _m[2][1]*scalar, _m[2][2]*scalar, _m[2][3]*scalar,
						  _m[3][0]*scalar, _m[3][1]*scalar, _m[3][2]*scalar, _m[3][3]*scalar );
}

template<typename real>
real4x4<real> real4x4<real>::operator-() const
{
	return real4x4<real>( -_m[0][0], -_m[0][1], -_m[0][2], -_m[0][3],
						  -_m[1][0], -_m[1][1], -_m[1][2], -_m[1][3],
						  -_m[2][0], -_m[2][1], -_m[2][2], -_m[2][3],
						  -_m[3][0], -_m[3][1], -_m[3][2], -_m[3][3] );
}

template<typename real>
real4x4<real>& real4x4<real>::operator*=( real scalar )
{
	_m[0][0] *= scalar;
	_m[0][1] *= scalar;
	_m[0][2] *= scalar;
	_m[0][3] *= scalar;
	_m[1][0] *= scalar;
	_m[1][1] *= scalar;
	_m[1][2] *= scalar;
	_m[1][3] *= scalar;
	_m[2][0] *= scalar;
	_m[2][1] *= scalar;
	_m[2][2] *= scalar;
	_m[2][3] *= scalar;
	_m[3][0] *= scalar;
	_m[3][1] *= scalar;
	_m[3][2] *= scalar;
	_m[3][3] *= scalar;
	return *this;
}

// Matrix operations
template<typename real>
void real4x4<real>::invertRBT()
{
	// negate the translation: 1) transform it by the 3x3 sub-matrix; 2) negate it;
	real tx = _m[3][0];
	real ty = _m[3][1];
	real tz = _m[3][2];
	_m[3][0] = -( _m[0][0]*tx + _m[0][1]*ty + _m[0][2]*tz );
	_m[3][1] = -( _m[1][0]*tx + _m[1][1]*ty + _m[1][2]*tz );
	_m[3][2] = -( _m[2][0]*tx + _m[2][1]*ty + _m[2][2]*tz );

	// now, simply transpose the 3x3 sub-matrix
	real temp = _m[1][0];
	_m[1][0] = _m[0][1];
	_m[0][1] = temp;

	temp = _m[2][0];
	_m[2][0] = _m[0][2];
	_m[0][2] = temp;

	temp = _m[2][1];
	_m[2][1] = _m[1][2];
	_m[1][2] = temp;
}

template<typename real>
void real4x4<real>::orthoNormalize()
{
	real x_colMag = _m[0][0]*_m[0][0] + _m[1][0]*_m[1][0] + _m[2][0]*_m[2][0];
	real y_colMag = _m[0][1]*_m[0][1] + _m[1][1]*_m[1][1] + _m[2][1]*_m[2][1];
	real z_colMag = _m[0][2]*_m[0][2] + _m[1][2]*_m[1][2] + _m[2][2]*_m[2][2];

	if( !math<real>::isEqual( x_colMag, 1.0 ) && !math<real>::isEqual( x_colMag, 0.0 ) )
	{
		x_colMag = static_cast<real>(1.0) / sqrt( x_colMag );
		_m[0][0] *= x_colMag;
		_m[1][0] *= x_colMag;
		_m[2][0] *= x_colMag;
	}

	if( !math<real>::isEqual( y_colMag, 1.0 ) && !math<real>::isEqual( y_colMag, 0.0 ) )
	{
		y_colMag = static_cast<real>(1.0) / sqrt( y_colMag );
		_m[0][1] *= y_colMag;
		_m[1][1] *= y_colMag;
		_m[2][1] *= y_colMag;
	}

	if( !math<real>::isEqual( z_colMag, 1.0 ) && !math<real>::isEqual( z_colMag, 0.0 ) )
	{
		z_colMag = static_cast<real>(1.0) / sqrt( z_colMag );
		_m[0][2] *= z_colMag;
		_m[1][2] *= z_colMag;
		_m[2][2] *= z_colMag;
	}
}

template<typename real>
void real4x4<real>::transform( real3<real>& v, real w = 1 ) const
{
	const real vx = v.x;
	const real vy = v.y;
	const real vz = v.z;
	const real d = 1.0f / ( _m[0][3]*vx + _m[1][3]*vy + _m[2][3]*vz + _m[3][3]*w ) ;

	v.x = ( _m[0][0]*vx + _m[1][0]*vy + _m[2][0]*vz + _m[3][0]*w ) * d;
	v.y = ( _m[0][1]*vx + _m[1][1]*vy + _m[2][1]*vz + _m[3][1]*w ) * d;
	v.z = ( _m[0][2]*vx + _m[1][2]*vy + _m[2][2]*vz + _m[3][2]*w ) * d;
}

template<typename real>
void real4x4<real>::transposedTransform( real3<real>& v, real w = 1 ) const
{
	const real vx = v.x;
	const real vy = v.y;
	const real vz = v.z;
	const real d = 1.0f / ( _m[3][0]*vx + _m[3][1]*vy + _m[3][2]*vz + _m[3][3]*w ) ;

	v.x = ( _m[0][0]*v.x + _m[0][1]*v.y + _m[0][2]*v.z + _m[0][3]*w ) * d;
	v.y = ( _m[1][0]*v.x + _m[1][1]*v.y + _m[1][2]*v.z + _m[1][3]*w ) * d;
	v.z = ( _m[2][0]*v.x + _m[2][1]*v.y + _m[2][2]*v.z + _m[2][3]*w ) * d;
}

template<typename real>
void real4x4<real>::transform3x3( real3<real>& v ) const
{
	const real vx = v.x;
	const real vy = v.y;
	const real vz = v.z;
	v.x = _m[0][0]*vx + _m[1][0]*vy + _m[2][0]*vz;
	v.y = _m[0][1]*vx + _m[1][1]*vy + _m[2][1]*vz;
	v.z = _m[0][2]*vx + _m[1][2]*vy + _m[2][2]*vz;
}

template<typename real>
void real4x4<real>::transposedTransform3x3( real3<real>& v ) const
{
	const real vx = v.x;
	const real vy = v.y;
	const real vz = v.z;
	v.x = _m[0][0]*vx + _m[0][1]*vy +_m[0][2]*vz;
	v.y = _m[1][0]*vx + _m[1][1]*vy +_m[1][2]*vz;
	v.z = _m[2][0]*vx + _m[2][1]*vy +_m[2][2]*vz;
}

/************************************************************************/
/* real4x4<real> Instantiations                                        */
/************************************************************************/

/*!
	real4x4 class with a single-precision implementation.
*/
class  float4x4 : public real4x4<float>
{
public:
	inline float4x4() : real4x4<float>() {;} // Uninitialized
	inline float4x4( const real4x4<float>& other ) : real4x4<float>( other ) {;}
	inline explicit float4x4( float const * const ptr ) : real4x4<float>( ptr ) {;}
	inline float4x4( float a00, float a01, float a02, float a03,
					 float a10, float a11, float a12, float a13,
					 float a20, float a21, float a22, float a23,
					 float a30, float a31, float a32, float a33 ) 
		: real4x4<float>( a00, a01, a02, a03,
						  a10, a11, a12, a13,
						  a20, a21, a22, a23,
						  a30, a31, a32, a33 ) {;}
};

/*!
	real4x4 class with a real-precision implementation.
*/
class  double4x4 : public real4x4<double>
{
public:
	inline double4x4() : real4x4<double>() {;} // Uninitialized
	inline double4x4( const real4x4<double>& other ) : real4x4<double>( other ) {;}
	inline explicit double4x4( double const * const ptr ) : real4x4<double>( ptr ) {;}
	inline double4x4( double a00, double a01, double a02, double a03,
					  double a10, double a11, double a12, double a13,
					  double a20, double a21, double a22, double a23,
					  double a30, double a31, double a32, double a33 ) 
		: real4x4<double>( a00, a01, a02, a03,
						   a10, a11, a12, a13,
						   a20, a21, a22, a23,
						   a30, a31, a32, a33 ) {;}
};

} // namespace rtu

#endif
