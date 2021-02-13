#pragma once
#ifndef _RTS_IENVIRONMENT_H_
#define _RTS_IENVIRONMENT_H_

#include <rts/IPlugin.h>

namespace rts {

class IEnvironment : public IPlugin
{
public:
	virtual void shade( rts::RTstate& state ) = 0;
};

} // namespace rts

#endif // _RTS_IENVIRONMENT_H_
