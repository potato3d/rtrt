#pragma once
#ifndef _RTC_PLUGINS_H_
#define _RTC_PLUGINS_H_

#include <rtu/common.h>
#include <rts/IRenderer.h>
#include <rts/ICamera.h>
#include <rts/IEnvironment.h>
#include <rts/IMaterial.h>
#include <rts/ITexture.h>
#include <rts/ILight.h>
#include <vector>

namespace rtc {

struct Plugins
{
	static rtu::ref_ptr<rts::IRenderer> renderer;
	static rtu::ref_ptr<rts::ICamera> camera;
	static rtu::ref_ptr<rts::IEnvironment> environment;
	static std::vector< rtu::ref_ptr<rts::ITexture> > textures;
	static std::vector< rtu::ref_ptr<rts::IMaterial> > materials;
	static std::vector< rtu::ref_ptr<rts::ILight> > lights;	
};

} // namespace rtc

#endif // _RTC_PLUGINS_H_
