#pragma once
#ifndef _RTL_ADAPTIVERENDERER_H_
#define _RTL_ADAPTIVERENDERER_H_

#include <rts/IRenderer.h>

namespace rtl {

class AdaptiveRenderer : public rts::IRenderer
{
public:
	virtual void init();
	virtual void render();

private:
	void adaptiveSupersample( float x, float y, rtu::float3& resultColor, unsigned int recursionDepth );

	unsigned int _maxRecursionDepth;
	float _epsilon;
};

} // namespace rtl

#endif // _RTL_ADAPTIVERENDERER_H_
