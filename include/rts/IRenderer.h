#pragma once
#ifndef _RTS_IRENDERER_H_
#define _RTS_IRENDERER_H_

#include <rts/IPlugin.h>

namespace rts {

class IRenderer : public IPlugin
{
public:
	virtual void render() = 0;
};

} // namespace rts

#endif // _RTS_IRENDERER_H_
