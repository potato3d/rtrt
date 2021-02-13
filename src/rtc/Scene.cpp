#include <rtc/Scene.h>

namespace rtc {

KdTree Scene::instanceTree;
std::vector<Instance> Scene::instances;
std::vector<Geometry> Scene::geometries;
float* Scene::frameBuffer;
float Scene::rayEpsilon;
unsigned int Scene::maxRayRecursionDepth;
float Scene::mediumRefractionIndex;

} // namespace rtc
