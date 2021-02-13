#pragma once
#ifndef _RTL_PERSPECTIVECAMERA_H_
#define _RTL_PERSPECTIVECAMERA_H_

#include <rts/ICamera.h>
#include <rtu/float3.h>
#include <rtu/quat.h>
#include <rtu/sse.h>

namespace rtl {

class PerspectiveCamera : public rts::ICamera
{
public:
	virtual void init();
	virtual void newFrame();
	virtual void receiveParameter( int paramId, void* paramValue );

	virtual void lookAt( float eyeX, float eyeY, float eyeZ, 
		                 float centerX, float centerY, float centerZ, 
		                 float upX, float upY, float upZ );
	virtual void setPerspective( float fovY, float zNear, float zFar );
	virtual void setViewport( unsigned int width, unsigned int height );
	virtual void getViewport( unsigned int& width, unsigned int& height );
	virtual void getRay( rts::RTstate& state, float x, float y );
	virtual void getRayPacket( rts::RTstate& state, float* rayXYCoords );

private:
	// Has changed since the last update?
	bool _dirty;

	// Input parameters
	rtu::float3 _position;
	rtu::quatf _orientation;

	float _fovy;
	float _zNear;
	float _zFar;

	unsigned int _screenWidth;
	unsigned int _screenHeight;

	// Derived values
	rtu::float3 _axisX;		// camera axes
	rtu::float3 _axisY;
	rtu::float3 _axisZ;

	rtu::float3 _nearOrigin;	// near plane origin
	rtu::float3 _nearU;		// near plane U vector
	rtu::float3 _nearV;		// near plane V vector

	// Pre-computation
	float _invWidth;
	float _invHeight;
	rtu::float3 _baseDir;
};

} // namespace rts

#endif // _RTL_PERSPECTIVECAMERA_H_
