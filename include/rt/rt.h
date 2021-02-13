#pragma once
#ifndef _RT_H_
#define _RT_H_

#include <rt/definitions.h>

/************************************************************************/
/* Core Programming Interface                                           */
/************************************************************************/

// Forward declarations
namespace rts
{
	class IRenderer;
	class ICamera;
	class IEnvironment;
	class IMaterial;
	class ITexture;
	class ILight;
}

// Initializes a valid ray tracing context
bool rtInit();

// Overwrite default renderer
void rtRendererClass( rts::IRenderer* obj );
void rtRendererParameter( unsigned int paramId, void* paramValue );

// Overwrite default camera
void rtCameraClass( rts::ICamera* obj );
void rtCameraParameter( unsigned int paramId, void* paramValue );

// Overwrite default environment
void rtEnvironmentClass( rts::IEnvironment* obj );
void rtEnvironmentParameter( unsigned int paramId, void* paramValue );

// Create and setup material shaders
unsigned int rtGenMaterials( unsigned int count );
void rtBindMaterial( unsigned int materialId );
void rtMaterialClass( rts::IMaterial* obj );
void rtMaterialParameter( unsigned int paramId, void* paramValue );

// Create and setup texture shaders
unsigned int rtGenTextures( unsigned int count );
void rtBindTexture( unsigned int textureId );
void rtTextureClass( rts::ITexture* obj );
void rtTextureParameter( unsigned int paramId, void* paramValue );

// Set texture image to currently bound texture shader
// Currently limited to RGB format and FLOAT data type
void rtTextureImage2D( unsigned int width, unsigned int height, unsigned char* texels );

// Create and setup light shaders
unsigned int rtGenLights( unsigned int count );
void rtBindLight( unsigned int lightId );
void rtLightClass( rts::ILight* obj );
void rtLightParameter( unsigned int paramId, void* paramValue );

// Matrix manipulation
void rtPushMatrix();
void rtLoadIdentity();
void rtScalef( float x, float y, float z );
void rtTranslatef( float x, float y, float z );
void rtRotatef( float degrees, float x, float y, float z );
void rtLoadMatrixfv( const float* const matrix );
void rtMultMatrixfv( const float* const matrix );
void rtPopMatrix();

// Create geometries
unsigned int rtGenGeometries( unsigned int count );
void rtNewGeometry( unsigned int geometryId );

// Vertex attribute binding
// Controls in which scope the specified attributes will be provided
void rtPushAttributeBindings();
void rtSetAttributeBinding( unsigned int attribute, unsigned int binding );
unsigned int rtGetAttributeBinding( unsigned int attribute );
void rtPopAttributeBindings();

// Receive geometric data for current geometry
// Uses currently bound material
void rtBegin( unsigned int primitiveType );

// Geometric data
void rtColor3fv( const float* const color );
void rtColor3f( float r, float g, float b );
void rtTexCoord2fv( const float* const texCoord );
void rtTexCoord2f( float s, float t );
void rtNormal3fv( const float* const normal );
void rtNormal3f( float x, float y, float z );
void rtVertex3fv( const float* const vertex );
void rtVertex3f( float x, float y, float z );

// End receiving data
void rtEnd();

// End new geometry
void rtEndGeometry();

// Instantiate geometries using current matrix
unsigned int rtGenInstances( unsigned int count );
void rtInstantiate( unsigned int instanceId, unsigned int geometryId );

// Do not change transform, or it will cause undefined side-effects
const float* rtGetInstanceTransform( unsigned int instanceId );

// Setup camera parameters
void rtLookAt( float eyeX, float eyeY, float eyeZ, 
			   float centerX, float centerY, float centerZ, 
			   float upX, float upY, float upZ );
void rtPerspective( float fovY, float zNear, float zFar );
void rtViewport( unsigned int width, unsigned int height );

// Set frame buffer
// Currently limited to RGB format and FLOAT data type
// At least width*height*3*sizeof(float) bytes of pixels must fit in buffer
void rtFrameBuffer( float* buffer );

// Set maximum ray recursion depth, used in refraction and reflection computations.
// Default is 3.
void rtSetMaxRayRecursionDepth( unsigned int depth );

// Ray epsilon tolerance, used to avoid self-intersections
// Default is 2e-4f.
void rtSetRayEpsilon( float epsilon );
float rtGetRayEpsilon();

// Refraction index of the medium used in light transmittance calculations
void rtSetMediumRefractionIndex( float index );
float rtGetMediumRefractionIndex();

// Ray trace scene
void rtRenderFrame();

#endif // _RT_H_
