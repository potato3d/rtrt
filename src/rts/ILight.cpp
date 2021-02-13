#include <rts/ILight.h>

namespace rts {

bool ILight::illuminate( rts::RTstate& state )
{
	// avoid warnings
	state;
	return false;
}

} // namespace rts
