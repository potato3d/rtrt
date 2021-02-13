#pragma once
#ifndef _RTL_PACKETTILEDRENDERER_H_
#define _RTL_PACKETTILEDRENDERER_H_

#include <rts/IRenderer.h>

namespace rtl {

class PacketTiledRenderer : public rts::IRenderer
{
public:
	virtual void render();
};

} // namespace rtl

#endif // _RTL_PACKETTILEDRENDERER_H_
