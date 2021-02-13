#pragma once
#ifndef _RTS_IPLUGIN_H_
#define _RTS_IPLUGIN_H_

#include <rt/rts.h>
#include <rtu/refcounting.h>

namespace rts {

class IPlugin : public rtu::RefCounted
{
public:
	// Default implementation: do nothing
	virtual void init();
	virtual void registerSelf();
	virtual void newFrame();
	virtual void receiveParameter( int paramId, void* paramValue );

protected:
	// Can only be deleted via ref_ptr
	virtual ~IPlugin();
};

} // namespace rts

#endif // _RTS_IPLUGIN_H_
