#include <rtl/PerspectiveCamera.h>
#include <algorithm>

// TODO: testing
#include <rtc/RayState.h>

namespace rtl {

// TODO: temporary since I haven't found a way to store these inside class (access violation due to unaligned data)
// TODO: perhaps need aligned_malloc? but what about ref_ptr?
// SSE storage
static __m128 s_posx4;
static __m128 s_posy4;
static __m128 s_posz4;

static __m128 s_dirx4;
static __m128 s_diry4;
static __m128 s_dirz4;

static __m128 s_nearUx4;
static __m128 s_nearUy4;
static __m128 s_nearUz4;

static __m128 s_nearVx4;
static __m128 s_nearVy4;
static __m128 s_nearVz4;

static __m128 s_invW4;
static __m128 s_invH4;

void PerspectiveCamera::init()
{
	_dirty = true;
	_position.set( 0, 0, 0 );
	_fovy = 60.0f;
	_zNear = 1.0f;
	_zFar = 1000.0f;
	_screenWidth = 400;
	_screenHeight = 300;
	_axisX.set( 1, 0, 0 );
	_axisY.set( 0, 1, 0 );
	_axisZ.set( 0, 0, -1 );
	_orientation.makeRotation( 0, 1, 0, 0 );
}

void PerspectiveCamera::newFrame()
{
	if( !_dirty )
		return;

	_dirty = false;

	_axisX = rtu::float3::UNIT_X();
	_axisY = rtu::float3::UNIT_Y();
	_axisZ = rtu::float3::UNIT_Z();

	_orientation.transform( _axisX );
	_orientation.transform( _axisY );
	_orientation.transform( _axisZ );

	// Compute the near plane origin
	float sz = _zNear;
	float sy = sz * tan( rtu::mathf::toRadians( _fovy * 0.5f ) );
	float sx = ( sy * _screenWidth ) / _screenHeight;
	_nearOrigin = _position + _axisX * -sx + _axisY * -sy + _axisZ * -sz;

	// Compute the near plane axis
	_nearU = _axisX * ( 2.0f * sx );
	_nearV = _axisY * ( 2.0f * sy );

	// Pre-compute data needed for ray generation
	_invWidth = 1.0f / _screenWidth;
	_invHeight = 1.0f / _screenHeight;
	_baseDir = _nearOrigin - _position;

	// SSE storage
	s_posx4 = _mm_set_ps1( _position.x );
	s_posy4 = _mm_set_ps1( _position.y );
	s_posz4 = _mm_set_ps1( _position.z );

	s_dirx4 = _mm_set_ps1( _baseDir.x );
	s_diry4 = _mm_set_ps1( _baseDir.y );
	s_dirz4 = _mm_set_ps1( _baseDir.z );

	s_nearUx4 = _mm_set_ps1( _nearU.x );
	s_nearUy4 = _mm_set_ps1( _nearU.y );
	s_nearUz4 = _mm_set_ps1( _nearU.z );

	s_nearVx4 = _mm_set_ps1( _nearV.x );
	s_nearVy4 = _mm_set_ps1( _nearV.y );
	s_nearVz4 = _mm_set_ps1( _nearV.z );

	s_invW4 = _mm_set_ps1( _invWidth );
	s_invH4 = _mm_set_ps1( _invHeight );
}

void PerspectiveCamera::receiveParameter( int paramId, void* paramValue )
{
	switch( paramId )
	{
	case RT_TRANSLATE:
		{
			rtu::float3 translation( reinterpret_cast<float*>( paramValue ) );
			_orientation.transform( translation );
			_position += translation;
			break;
		}

	case RT_ROTATE_X:
		{
			float angle = *reinterpret_cast<float*>( paramValue );
			_orientation *= rtu::quatf( angle, _axisX );
			break;
		}

	case RT_ROTATE_Y:
		{
			float angle = *reinterpret_cast<float*>( paramValue );
			_orientation *= rtu::quatf( angle, _axisY );
			break;
		}

	case RT_ROTATE_Z:
		{
			float angle = *reinterpret_cast<float*>( paramValue );
			_orientation *= rtu::quatf( angle, _axisZ );
			break;
		}

	default:
		return;
	}

	_dirty = true;
}

void PerspectiveCamera::lookAt( float eyeX, float eyeY, float eyeZ, 
					            float centerX, float centerY, float centerZ, 
					            float upX, float upY, float upZ )
{
	_position.set( eyeX, eyeY, eyeZ );
	rtu::float3 center( centerX, centerY, centerZ );
	rtu::float3 up( upX, upY, upZ );

	_axisZ = _position - center;
	_axisZ.normalize();

	_axisX = up.cross( _axisZ );
	_axisX.normalize();

	_axisY = _axisZ.cross( _axisX );

	rtu::float4x4 lookAtMatrix;
	lookAtMatrix.makeLookAt( _position, center, up );
	lookAtMatrix.getRotation( _orientation );
	_dirty = true;
}

void PerspectiveCamera::setPerspective( float fovY, float zNear, float zFar )
{
	_fovy  = fovY;
	_zNear = zNear;
	_zFar  = zFar;
	_dirty = true;
}

void PerspectiveCamera::setViewport( unsigned int width, unsigned int height )
{
	_screenWidth = width;
	_screenHeight = height;
	_dirty = true;
}

void PerspectiveCamera::getViewport( unsigned int& width, unsigned int& height )
{
	width  = _screenWidth;
	height = _screenHeight;
}

void PerspectiveCamera::getRay( rts::RTstate& state, float x, float y )
{
	const float uStep = x * _invWidth;
	const float vStep = y * _invHeight;

	// No need to normalize ray direction
	rtsRayOrigin( state ) = _position;
	rtsRayDirection( state ).set( _baseDir.x + _nearU.x*uStep + _nearV.x*vStep,	    // x
								  _baseDir.y + _nearU.y*uStep + _nearV.y*vStep,	    // y
								  _baseDir.z + _nearU.z*uStep + _nearV.z*vStep );	// z
}

void PerspectiveCamera::getRayPacket( rts::RTstate& state, float* rayXYCoords )
{
	const __m128 rc4[RT_PACKET_SIMD_SIZE*2] = { 
		_mm_set_ps( rayXYCoords[6], rayXYCoords[4], rayXYCoords[2], rayXYCoords[0] ), 
		_mm_set_ps( rayXYCoords[7], rayXYCoords[5], rayXYCoords[3], rayXYCoords[1] ), 
		_mm_set_ps( rayXYCoords[14], rayXYCoords[12], rayXYCoords[10], rayXYCoords[8] ), 
		_mm_set_ps( rayXYCoords[15], rayXYCoords[13], rayXYCoords[11], rayXYCoords[9] ),
		_mm_set_ps( rayXYCoords[22], rayXYCoords[20], rayXYCoords[18], rayXYCoords[16] ),
		_mm_set_ps( rayXYCoords[23], rayXYCoords[21], rayXYCoords[19], rayXYCoords[17] ),
		_mm_set_ps( rayXYCoords[30], rayXYCoords[28], rayXYCoords[26], rayXYCoords[24] ),
		_mm_set_ps( rayXYCoords[31], rayXYCoords[29], rayXYCoords[27], rayXYCoords[25] ) };

	// TODO: temporary hack while there is no rts api to do this efficiently
	// TODO: when done, remove ../../src from project include path!
	rtc::RayPacketState& rps = *reinterpret_cast<rtc::RayPacketState*>( &state );

	for( int p = 0, r = 0; p < RT_PACKET_SIMD_SIZE; ++p, r+=2 )
	{
		const __m128 uStep4 = _mm_mul_ps( rc4[r], s_invW4 );
		const __m128 vStep4 = _mm_mul_ps( rc4[r+1], s_invH4 );

		rps.packet.ox4[p] = s_posx4;
		rps.packet.oy4[p] = s_posy4;
		rps.packet.oz4[p] = s_posz4;

		rps.packet.dx4[p] = _mm_add_ps( s_dirx4, _mm_add_ps( _mm_mul_ps( s_nearUx4, uStep4 ), _mm_mul_ps( s_nearVx4, vStep4 ) ) );
		rps.packet.dy4[p] = _mm_add_ps( s_diry4, _mm_add_ps( _mm_mul_ps( s_nearUy4, uStep4 ), _mm_mul_ps( s_nearVy4, vStep4 ) ) );
		rps.packet.dz4[p] = _mm_add_ps( s_dirz4, _mm_add_ps( _mm_mul_ps( s_nearUz4, uStep4 ), _mm_mul_ps( s_nearVz4, vStep4 ) ) );
	}
}

} // namespace rtl
