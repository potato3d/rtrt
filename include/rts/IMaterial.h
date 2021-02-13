#pragma once
#ifndef _RTS_IMATERIAL_H_
#define _RTS_IMATERIAL_H_

#include <rts/IPlugin.h>

namespace rts {

class IMaterial : public IPlugin
{
public:
	// Default implementation: do nothing
	virtual void shade( rts::RTstate& state );
};

} // namespace rts

#endif // _RTS_IMATERIAL_H_
