#pragma once
#ifndef _RTS_ITEXTURE_H_
#define _RTS_ITEXTURE_H_

#include <rts/IPlugin.h>

namespace rts {

class ITexture : public IPlugin
{
public:
	// Default implementation: do nothing
	virtual void shade( rts::RTstate& state );
	virtual void textureImage2D( unsigned int width, unsigned int height, unsigned char* texels );
};

} // namespace rts

#endif // _RTS_ITEXTURE_H_
