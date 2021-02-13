#pragma once
#ifndef _RTS_ILIGHT_H_
#define _RTS_ILIGHT_H_

#include <rts/IPlugin.h>

namespace rts {

class ILight : public IPlugin
{
public:
	// Default implementation: do nothing
	virtual bool illuminate( rts::RTstate& state );
};

} // namespace rts

#endif // _RTS_ILIGHT_H_
