#pragma once
#ifndef _RTL_JITTEREDRENDERER_H_
#define _RTL_JITTEREDRENDERER_H_

#include <rts/IRenderer.h>

namespace rtl {

class JitteredRenderer : public rts::IRenderer
{
public:
	enum GridResolution
	{
		TWO_BY_TWO,
		FOUR_BY_FOUR,
		// TODO: EIGHT_BY_EIGHT = 2
	};

	virtual void init();
	virtual void render();

	void setGridResolution( GridResolution res );

private:
	GridResolution _gridRes;
};

} // namespace rtl

#endif // _RTL_JITTEREDRENDERER_H_
