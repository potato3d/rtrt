#pragma once
#ifndef _RTL_SINGLERENDERER_H_
#define _RTL_SINGLERENDERER_H_

#include <rts/IRenderer.h>

namespace rtl {

class SingleRenderer : public rts::IRenderer
{
public:
	virtual void render();
};

} // namespace rtl

#endif // _RTL_SINGLERENDERER_H_
