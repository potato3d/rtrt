#include <rtc/Plugins.h>

namespace rtc {

rtu::ref_ptr<rts::IRenderer> Plugins::renderer;
rtu::ref_ptr<rts::ICamera> Plugins::camera;
rtu::ref_ptr<rts::IEnvironment> Plugins::environment;
std::vector< rtu::ref_ptr<rts::IMaterial> > Plugins::materials;
std::vector< rtu::ref_ptr<rts::ILight> > Plugins::lights;
std::vector< rtu::ref_ptr<rts::ITexture> > Plugins::textures;

} // namespace rtc
