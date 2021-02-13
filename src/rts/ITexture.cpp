#include <rts/ITexture.h>

namespace rts
{

void ITexture::shade( rts::RTstate& state )
{
	// avoid warnings
	state;
}

void ITexture::textureImage2D( unsigned int width, unsigned int height, unsigned char* texels )
{
	// avoid warnings
	width; height; texels;
}

} // namespace rts
