#include <rtl/SingleColorEnvironment.h>

namespace rtl {

void SingleColorEnvironment::init()
{
	_background.set( 0, 0, 0 );
}

void SingleColorEnvironment::shade( rts::RTstate& state )
{
	rtsResultColor( state ) = _background;
}

} // namespace rtl
