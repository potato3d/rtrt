#include <rt/rt.h>

#include <rtc/Plugins.h>
#include <rtc/Scene.h>

#include <rtc/MatrixStack.h>
#include <rtc/PrimitiveAssembler.h>
#include <rtc/KdTreeBuilder.h>

#include <rtl/PerspectiveCamera.h>
#include <rtl/SingleColorEnvironment.h>
#include <rtl/SingleRenderer.h>

#include <rtu/stl.h>

/************************************************************************/
/* Global objects                                                       */
/************************************************************************/
static rtc::MatrixStack s_matrixStack;

// Receive primitive data
static rtc::PrimitiveAssembler s_primitiveAssembler;
static std::stack<rtc::AttributeBinding> s_bindingStack;
	
// State managing
static unsigned int s_currentGeometry;
static unsigned int s_currentMaterial;
static unsigned int s_currentTexture;
static unsigned int s_currentLight;

// Instance managing
static bool s_instancesDirty = true;

/************************************************************************/
/* Core Programming Interface                                           */
/************************************************************************/

// Initializes a valid ray tracing context
bool rtInit()
{
	// Setup default plug-ins
	rtCameraClass( new rtl::PerspectiveCamera() );
	rtEnvironmentClass( new rtl::SingleColorEnvironment() );
	rtRendererClass( new rtl::SingleRenderer() );

	// Invalid light id == 0
	rtGenLights( 1 );
	rtc::Plugins::lights.at( 0 ) = new rts::ILight();
	rtBindLight( 0 );

	// Invalid material id == 0
	rtGenMaterials( 1 );
	rtc::Plugins::materials.at( 0 ) = new rts::IMaterial();
	rtBindMaterial( 0 );

	// Invalid texture id == 0
	rtGenTextures( 1 );
	rtc::Plugins::textures.at( 0 ) = new rts::ITexture();
	rtBindTexture( 0 );

	// Setup default ray tracing parameters
	rtSetMaxRayRecursionDepth( 3 );
	// TODO: Watch for conflicts with epsilon in raytracer class. Remove there and change here!
	// Currently, we use: ray.tnear = epsilon and hit.position += hit.shadingNormal * epsilon (rtsInitLightState).
	rtSetRayEpsilon( 2e-4f );
	// Approximation for air index
	rtSetMediumRefractionIndex( 1.0f );

	// Default attribute bindings
	rtc::AttributeBinding ab;
	ab.reset();
	s_bindingStack.push( ab );

	// Everything ok
	return true;
}

// Overwrite default renderer
void rtRendererClass( rts::IRenderer* obj )
{
	rtc::Plugins::renderer = obj;
	obj->init();
}

void rtRendererParameter( unsigned int paramId, void* paramValue )
{
	rtc::Plugins::renderer->receiveParameter( paramId, paramValue );
}

// Overwrite default camera
void rtCameraClass( rts::ICamera* obj )
{
	rtc::Plugins::camera = obj;
	obj->init();
}

void rtCameraParameter( unsigned int paramId, void* paramValue )
{
	rtc::Plugins::camera->receiveParameter( paramId, paramValue );
}

// Overwrite default environment
void rtEnvironmentClass( rts::IEnvironment* obj )
{
	rtc::Plugins::environment = obj;
	obj->init();
}

void rtEnvironmentParameter( unsigned int paramId, void* paramValue )
{
	rtc::Plugins::environment->receiveParameter( paramId, paramValue );
}

// Create and setup material shaders
unsigned int rtGenMaterials( unsigned int count )
{
	unsigned int previousSize = rtc::Plugins::materials.size();
	unsigned int newSize = previousSize + count;

	rtc::Plugins::materials.resize( newSize );

	// Populate with default implementations
	for( unsigned int i = previousSize; i < newSize; ++i )
	{
		rtc::Plugins::materials[i] = new rts::IMaterial();
	}

	return previousSize;
}

void rtBindMaterial( unsigned int materialId )
{
	if( materialId >= rtc::Plugins::materials.size() )
		return;

	s_currentMaterial = materialId;
}

void rtMaterialClass( rts::IMaterial* obj )
{
	if( s_currentMaterial == 0 )
		return;

	rtc::Plugins::materials.at( s_currentMaterial ) = obj;
	obj->init();
}

void rtMaterialParameter( unsigned int paramId, void* paramValue )
{
	if( ( s_currentMaterial == 0 ) || !rtc::Plugins::materials.at( s_currentMaterial ).valid() )
		return;

	rtc::Plugins::materials.at( s_currentMaterial )->receiveParameter( paramId, paramValue );
}

// Create and setup texture shaders
unsigned int rtGenTextures( unsigned int count )
{
	unsigned int previousSize = rtc::Plugins::textures.size();
	unsigned int newSize = previousSize + count;

	rtc::Plugins::textures.resize( newSize );

	// Populate with default implementations
	for( unsigned int i = previousSize; i < newSize; ++i )
	{
		rtc::Plugins::textures[i] = new rts::ITexture();
	}

	return previousSize;
}

void rtBindTexture( unsigned int textureId )
{
	if( textureId >= rtc::Plugins::textures.size() )
		return;

	s_currentTexture = textureId;
}

void rtTextureClass( rts::ITexture* obj )
{
	if( s_currentTexture == 0 )
		return;

	rtc::Plugins::textures.at( s_currentTexture ) = obj;
	obj->init();
}

void rtTextureParameter( unsigned int paramId, void* paramValue )
{
	if( ( s_currentTexture == 0 ) || !rtc::Plugins::textures.at( s_currentTexture ).valid() )
		return;

	rtc::Plugins::textures.at( s_currentTexture )->receiveParameter( paramId, paramValue );
}

// Set texture image to currently bound texture shader
// Currently limited to RGB format and FLOAT data type
void rtTextureImage2D(  unsigned int width, unsigned int height, unsigned char* texels )
{
	if( ( s_currentTexture == 0 ) || !rtc::Plugins::textures.at( s_currentTexture ).valid() )
		return;

	rtc::Plugins::textures.at( s_currentTexture )->textureImage2D( width, height, texels );
}

// Create and setup light shaders
unsigned int rtGenLights( unsigned int count )
{
	unsigned int previousSize = rtc::Plugins::lights.size();
	unsigned int newSize = previousSize + count;

	rtc::Plugins::lights.resize( newSize );

	// Populate with default implementations
	for( unsigned int i = previousSize; i < newSize; ++i )
	{
		rtc::Plugins::lights[i] = new rts::ILight();
	}

	return previousSize;
}

void rtBindLight( unsigned int lightId )
{
	if( lightId >= rtc::Plugins::lights.size() )
		return;

	s_currentLight = lightId;
}

void rtLightClass( rts::ILight* obj )
{
	if( s_currentLight == 0 )
		return;

	rtc::Plugins::lights.at( s_currentLight ) = obj;
	obj->init();
}

void rtLightParameter( unsigned int paramId, void* paramValue )
{
	if( ( s_currentLight == 0 ) || !rtc::Plugins::lights.at( s_currentLight ).valid() )
		return;

	rtc::Plugins::lights.at( s_currentLight )->receiveParameter( paramId, paramValue );
}

// Matrix manipulation
void rtPushMatrix()
{
	s_matrixStack.pushMatrix();
}

void rtLoadIdentity()
{
	s_matrixStack.loadIdentity();
}

void rtScalef( float x, float y, float z )
{
	s_matrixStack.scale( x, y, z );
}

void rtTranslatef( float x, float y, float z )
{
	s_matrixStack.translate( x, y, z );
}

void rtRotatef( float degrees, float x, float y, float z )
{
	s_matrixStack.rotate( degrees, x, y, z );
}

void rtLoadMatrixfv( const float* const matrix )
{
	s_matrixStack.loadMatrix( matrix );
}

void rtMultMatrixfv( const float* const matrix )
{
	s_matrixStack.multMatrix( matrix );
}

void rtPopMatrix()
{
	s_matrixStack.popMatrix();
}

// Create geometries
unsigned int rtGenGeometries( unsigned int count )
{
	unsigned int previousSize = rtc::Scene::geometries.size();
	rtc::Scene::geometries.resize( previousSize + count );
	return previousSize;
}

void rtNewGeometry( unsigned int geometryId )
{
	if( geometryId >= rtc::Scene::geometries.size() )
		return;

	s_currentGeometry = geometryId;

	rtc::Geometry& geometry = rtc::Scene::geometries.at( geometryId );
	geometry.kdTree.erase();
	rtu::vectorFreeMemory( geometry.triAccel );
	rtu::vectorFreeMemory( geometry.triDesc );
	rtu::vectorFreeMemory( geometry.vertices );
	rtu::vectorFreeMemory( geometry.normals );
	rtu::vectorFreeMemory( geometry.colors );
	rtu::vectorFreeMemory( geometry.texCoords );
}

// Vertex attribute binding
// Controls in which scope the specified attributes will be provided
void rtPushAttributeBindings()
{
	s_bindingStack.push( s_bindingStack.top() );
}

void rtSetAttributeBinding( unsigned int attribute, unsigned int binding )
{
	rtc::AttributeBinding& currBindings = s_bindingStack.top();

	switch( attribute )
	{
	case RT_NORMAL:
		currBindings.normalBinding = binding;
		break;
	case RT_COLOR:
		currBindings.colorBinding = binding;
		break;
	case RT_TEXTURE_COORD:
		currBindings.textureBinding = binding;
		break;
	default:
		// TODO: warning message
	    break;
	}
}

unsigned int rtGetAttributeBinding( unsigned int attribute )
{
	rtc::AttributeBinding& currBindings = s_bindingStack.top();

	switch( attribute )
	{
	case RT_NORMAL:
		return currBindings.normalBinding;
	case RT_COLOR:
		return currBindings.colorBinding;
	case RT_TEXTURE_COORD:
		return currBindings.textureBinding;
	default:
		return 0;
	}
}

void rtPopAttributeBindings()
{
	if( s_bindingStack.size() > 1 )
		s_bindingStack.pop();
}

// Receive geometric data for current geometry
// Uses currently bound material
void rtBegin( unsigned int primitiveType )
{
	s_primitiveAssembler.beginPrimitiveData( s_currentGeometry, primitiveType, s_currentMaterial );
	s_primitiveAssembler.setMatrixTransform( s_matrixStack.top() );
	s_primitiveAssembler.setBindings( s_bindingStack.top() );
}

// Geometric data
void rtColor3fv( const float* const color )
{
	s_primitiveAssembler.setColor( rtu::float3( color ) );
}

void rtColor3f( float r, float g, float b )
{
	s_primitiveAssembler.setColor( rtu::float3( r, g, b ) );
}

void rtTexCoord2fv( const float* const texCoord )
{
	s_primitiveAssembler.setTexCoord( rtu::float3( texCoord[0], texCoord[1], 0.0f ) );
}

void rtTexCoord2f( float s, float t )
{
	s_primitiveAssembler.setTexCoord( rtu::float3( s, t, 0.0f ) );
}

void rtNormal3fv( const float* const normal )
{
	s_primitiveAssembler.setNormal( rtu::float3( normal ) );
}

void rtNormal3f( float x, float y, float z )
{
	s_primitiveAssembler.setNormal( rtu::float3( x, y, z ) );
}

void rtVertex3fv( const float* const vertex )
{
	s_primitiveAssembler.addVertex( rtu::float3( vertex ) );
}

void rtVertex3f( float x, float y, float z )
{
	s_primitiveAssembler.addVertex( rtu::float3( x, y, z ) );
}

// End receiving data
void rtEnd()
{
	// Process vertex data and create triangle indexes based on primitive type
	s_primitiveAssembler.endPrimitiveData();
}

// End new geometry
void rtEndGeometry()
{
	// Build and store the optimized kdtree
	rtc::KdTreeBuilder::buildTree( &rtc::Scene::geometries.at( s_currentGeometry ) );
}

// Instantiate geometries using current matrix
unsigned int rtGenInstances( unsigned int count )
{
	int previousSize = rtc::Scene::instances.size();
	rtc::Scene::instances.resize( previousSize + count );
	return previousSize;
}

void rtInstantiate( unsigned int instanceId, unsigned int geometryId )
{
	if( ( instanceId >= rtc::Scene::instances.size() ) || ( geometryId >= rtc::Scene::geometries.size() ) )
		return;

	rtc::Instance& instance = rtc::Scene::instances.at( instanceId );
	instance.transform.setMatrix( s_matrixStack.top() );
	instance.geometryId = geometryId;

	// Update instance bounding box according to current matrix and geometry's original bounding box
	rtu::float3 boxVertices[8];
	rtc::Scene::geometries.at( geometryId ).kdTree.bbox.computeVertices( boxVertices );

	for( unsigned int v = 0; v < 8; ++v )
	{
		instance.transform.transformVertex( boxVertices[v] );
	}

	instance.bbox.buildFrom( boxVertices, 8 );

	// Avoid precision problems: we extend the box a little
	// The other option would be to add a similar tolerance to the clipRay method in AABB.
	// But since the clipRay method is called several times per ray, we opted to put that tolerance here.
	rtu::float3& minv = instance.bbox.minv;
	minv.x *= ( minv.x < 0.0f )? 1.111f : 0.999f;
	minv.y *= ( minv.y < 0.0f )? 1.111f : 0.999f;
	minv.z *= ( minv.z < 0.0f )? 1.111f : 0.999f;
	rtu::float3& maxv = instance.bbox.maxv;
	maxv.x *= ( maxv.x < 0.0f )? 0.999f : 1.111f;
	maxv.y *= ( maxv.y < 0.0f )? 0.999f : 1.111f;
	maxv.z *= ( maxv.z < 0.0f )? 0.999f : 1.111f;

	s_instancesDirty = true;
}

// Do not change transform, or it will cause undefined side-effects
const float* rtGetInstanceTransform( unsigned int instanceId )
{
	if( instanceId >= rtc::Scene::instances.size() )
		return NULL;

	return rtc::Scene::instances.at( instanceId ).transform.matrix().ptr();
}

// Setup camera parameters
void rtLookAt( float eyeX, float eyeY, float eyeZ, 
			   float centerX, float centerY, float centerZ, 
			   float upX, float upY, float upZ )
{
	rtc::Plugins::camera->lookAt( eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ );
}

void rtPerspective( float fovY, float zNear, float zFar )
{
	rtc::Plugins::camera->setPerspective( fovY, zNear, zFar );
}

void rtViewport( unsigned int width, unsigned int height )
{
	rtc::Plugins::camera->setViewport( width, height );
}

// Set frame buffer
// Currently limited to RGB format and FLOAT data type
// At least width*height*3*sizeof(float) bytes of pixels must fit in buffer
void rtFrameBuffer( float* buffer )
{
	rtc::Scene::frameBuffer = buffer;
}

// Set maximum ray recursion depth, used in refraction and reflection computations.
// Default is 3.
void rtSetMaxRayRecursionDepth( unsigned int depth )
{
	rtc::Scene::maxRayRecursionDepth = depth;
}

// Ray epsilon tolerance, used to avoid self-intersections
// Default is 2e-4f.
void rtSetRayEpsilon( float epsilon )
{
	rtc::Scene::rayEpsilon = epsilon;
}

float rtGetRayEpsilon()
{
	return rtc::Scene::rayEpsilon;
}

// Refraction index of the medium used in light transmittance calculations
void rtSetMediumRefractionIndex( float index )
{
	rtc::Scene::mediumRefractionIndex = index;
}

float rtGetMediumRefractionIndex()
{
	return rtc::Scene::mediumRefractionIndex;
}

// Ray trace scene
void rtRenderFrame()
{
	// Call newFrame for everyone
	// Skip id == 0
	for( unsigned int i = 1, size = rtc::Plugins::lights.size(); i < size; ++i )
	{
		rtc::Plugins::lights[i]->newFrame();
	}
	for( unsigned int i = 1, size = rtc::Plugins::textures.size(); i < size; ++i )
	{
		rtc::Plugins::textures[i]->newFrame();
	}
	for( unsigned int i = 1, size = rtc::Plugins::materials.size(); i < size; ++i )
	{
		rtc::Plugins::materials[i]->newFrame();
	}
	rtc::Plugins::environment->newFrame();
	rtc::Plugins::camera->newFrame();
	rtc::Plugins::renderer->newFrame();

	// Update instances, if needed
	if( s_instancesDirty )
	{
		rtc::KdTreeBuilder::buildTree( rtc::Scene::instanceTree, rtc::Scene::instances );
		s_instancesDirty = false;
	}

	// Render current frame
	rtc::Plugins::renderer->render();
}
