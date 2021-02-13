#include <rts/IPlugin.h>

namespace rts {

void IPlugin::init()
{
	// empty
}

void IPlugin::registerSelf()
{
	// empty
}

void IPlugin::newFrame()
{
	// empty
}

void IPlugin::receiveParameter( int paramId, void* paramValue )
{
	// avoid warnings
	paramId; paramValue;
}

IPlugin::~IPlugin()
{
	// empty
}

} // namespace rts
