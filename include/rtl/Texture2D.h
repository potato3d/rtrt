#pragma once
#ifndef _RTL_TEXTURE2D_H_
#define _RTL_TEXTURE2D_H_

#include <rts/ITexture.h>

namespace rtl {

class Texture2D : public rts::ITexture
{
public:
	Texture2D();

	virtual void receiveParameter( int paramId, void* paramValue );
	virtual void textureImage2D( unsigned int width, unsigned int height, unsigned char* texels );
	virtual void shade( rts::RTstate& state );

protected:
	unsigned int   _filter;
	unsigned int   _wrapS;
	unsigned int   _wrapT;
	unsigned int   _envMode;
	unsigned int   _width;
	unsigned int   _height;
	unsigned char* _texels;
};

} // namespace rtl

#endif // _RTL_TEXTURE2D_H_
