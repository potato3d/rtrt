#pragma once
#ifndef _RTL_TILEDRENDERER_H_
#define _RTL_TILEDRENDERER_H_

#include <rts/IRenderer.h>

namespace rtl {

class TiledRenderer : public rts::IRenderer
{
public:
	virtual void render();
};

} // namespace rtl

#endif // _RTL_TILEDRENDERER_H_
