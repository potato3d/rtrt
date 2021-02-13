#include <rts/ICamera.h>

namespace rts {

void ICamera::getRay( rts::RTstate& state, float x, float y )
{
	// avoid warnings
	state; x; y;
}

void ICamera::getRayPacket( rts::RTstate& state, float* rayXYCoords )
{
	// avoid warnings
	state; rayXYCoords;
}

} // namespace rts
