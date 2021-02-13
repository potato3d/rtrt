#pragma once
#ifndef _RTL_MULTITHREADRENDERER_H_
#define _RTL_MULTITHREADRENDERER_H_

#include <rts/IRenderer.h>

namespace rtl {

class MultiThreadRenderer : public rts::IRenderer
{
public:
	virtual void render();
};

} // namespace rtl

#endif // _RTL_MULTITHREADRENDERER_H_
