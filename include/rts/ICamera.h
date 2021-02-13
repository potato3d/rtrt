#pragma once
#ifndef _RTS_ICAMERA_H_
#define _RTS_ICAMERA_H_

#include <rts/IPlugin.h>

namespace rts {

class ICamera : public IPlugin
{
public:
	virtual void lookAt( float eyeX, float eyeY, float eyeZ, 
		                   float centerX, float centerY, float centerZ, 
						   float upX, float upY, float upZ ) = 0;
	virtual void setPerspective( float fovY, float zNear, float zFar ) = 0;
	virtual void setViewport( unsigned int width, unsigned int height ) = 0;
	virtual void getViewport( unsigned int& width, unsigned int& height ) = 0;
	virtual void getRay( rts::RTstate& state, float x, float y );
	virtual void getRayPacket( rts::RTstate& state, float* rayXYCoords );
};

} // namespace rts

#endif // _RTS_ICAMERA_H_
