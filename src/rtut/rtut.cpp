#include <rtut/rtut.h>

#include <rt/rt.h>

#include <rtut/Teapot.h>
#include <rtut/Cube.h>
#include <rtut/OsgGeometryLoader.h>

#include <rtl/HeadlightColor.h>

#include <fstream>
#include <istream>

void rtutLogo()
{
	rtBegin( RT_TRIANGLES );

	rtNormal3f( 0.0f, 0.0f, 1.0f );

	// Triangle 1
	rtColor3f( 0.0f, 0.0f, 1.0f );
	rtVertex3f( 0.0f, 0.0f, 0.0f );

	rtColor3f( 1.0f, 0.0f, 0.0f );
	rtVertex3f( 1.0f, 0.0f, 0.0f );

	rtColor3f( 0.0f, 1.0f, 0.0f );
	rtVertex3f( 0.0f, 1.0f, 0.0f );

	// Triangle 2
	rtColor3f( 1.0f, 0.0f, 0.0f );
	rtVertex3f( 2.0f, 0.0f, 0.0f );

	rtColor3f( 0.0f, 1.0f, 0.0f );
	rtVertex3f( 3.0f, 0.0f, 0.0f );

	rtColor3f( 0.0f, 0.0f, 1.0f );
	rtVertex3f( 3.0f, 1.0f, 0.0f );

	// Triangle 3
	rtColor3f( 1.0f, 0.0f, 0.0f );
	rtVertex3f( 1.5f, 0.0f, 0.0f );

	rtColor3f( 0.0f, 1.0f, 0.0f );
	rtVertex3f( 2.5f, 1.0f, 0.0f );

	rtColor3f( 0.0f, 0.0f, 1.0f );
	rtVertex3f( 0.5f, 1.0f, 0.0f );

	rtEnd();
}

// Basic shapes

/*
*  Vertices are computed as follows:
*     7+------+6
*     /|     /|      y
*    / |    / |      |
*   / 3+---/--+2     |
* 4+------+5 /       *---x
*  | /    | /       /
*  |/     |/       z
* 0+------+1      
*/
void rtutCube()
{
	rtBegin( RT_TRIANGLES );
		// Top
		rtNormal3f( 0.0f, 1.0f, 0.0f );
		rtVertex3fv( rtut::CUBE_VERTICES + 4*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 5*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 6*3 );

		rtVertex3fv( rtut::CUBE_VERTICES + 4*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 6*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 7*3 );

		// Bottom
		rtNormal3f( 0.0f, -1.0f, 0.0f );
		rtVertex3fv( rtut::CUBE_VERTICES + 0*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 3*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 2*3 );

		rtVertex3fv( rtut::CUBE_VERTICES + 0*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 2*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 1*3 );

		// Left
		rtNormal3f( -1.0f, 0.0f, 0.0f );
		rtVertex3fv( rtut::CUBE_VERTICES + 4*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 3*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 0*3 );

		rtVertex3fv( rtut::CUBE_VERTICES + 4*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 7*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 3*3 );

		// Right
		rtNormal3f( 1.0f, 0.0f, 0.0f );
		rtVertex3fv( rtut::CUBE_VERTICES + 5*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 2*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 6*3 );

		rtVertex3fv( rtut::CUBE_VERTICES + 5*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 1*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 2*3 );

		// Front
		rtNormal3f( 0.0f, 0.0f, 1.0f );
		rtVertex3fv( rtut::CUBE_VERTICES + 0*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 1*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 5*3 );

		rtVertex3fv( rtut::CUBE_VERTICES + 0*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 5*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 4*3 );

		// Back
		rtNormal3f( 0.0f, 0.0f, -1.0f );
		rtVertex3fv( rtut::CUBE_VERTICES + 3*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 7*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 6*3 );

		rtVertex3fv( rtut::CUBE_VERTICES + 3*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 6*3 );
		rtVertex3fv( rtut::CUBE_VERTICES + 2*3 );
	rtEnd();
}

void rtutSphere( unsigned int slices, unsigned int stacks )
{
	unsigned int i,j;
	double t1,t2,t3;
	rtu::float3 e,p;

	float phi1 = 0;
	float phi2 = rtu::mathf::TWO_PI;   

	float r  = 1.0f;

	float theta1 = 0;
	float theta2 = rtu::mathf::PI;

	rtu::float3 c( 0.0f, 0.0f, 0.0f );

	for( j = 0; j < stacks/2; j++ )
	{
		t1 = phi1 + j * (phi2 - phi1) / (slices/2);
		t2 = phi1 + (j + 1) * (phi2 - phi1) / (slices/2);

		rtBegin(RT_TRIANGLE_STRIP);

		for (i=0;i <= slices;i++)
		{
			t3 = theta1 + i * (theta2 - theta1) / slices;

			e.x = cos(t1) * cos(t3);
			e.y = sin(t1);
			e.z = cos(t1) * sin(t3);
			p.x = c.x + r * e.x;
			p.y = c.y + r * e.y;
			p.z = c.z + r * e.z;
			rtNormal3f(e.x,e.y,e.z);
			rtTexCoord2f(i/(double)slices,2*j/(double)slices);
			rtVertex3f(p.x,p.y,p.z);

			e.x = cos(t2) * cos(t3);
			e.y = sin(t2);
			e.z = cos(t2) * sin(t3);
			p.x = c.x + r * e.x;
			p.y = c.y + r * e.y;
			p.z = c.z + r * e.z;
			rtNormal3f(e.x,e.y,e.z);
			rtTexCoord2f(i/(double)slices,2*(j+1)/(double)slices);
			rtVertex3f(p.x,p.y,p.z);

		}
		rtEnd();
	}
}

void rtutTeapot()
{
	unsigned int i = 0;
	while( i < rtut::NUM_TEAPOT_INDEXES )
	{
		rtBegin( RT_TRIANGLE_STRIP );
		while( rtut::TEAPOT_INDEXES[i] != rtut::STRIP_END )
		{
			int index = rtut::TEAPOT_INDEXES[i] * 3;
			rtNormal3fv( rtut::TEAPOT_NORMALS + index );
			rtVertex3fv( rtut::TEAPOT_VERTICES + index );
			i++;
		}
		rtEnd();
		i++; // skip strip end flag
	}
}

// Geometry loading

// Simple triangle scene for tests
bool rtutLoadRa2( char* filename, unsigned int& geometryId )
{
	std::ifstream geometry( filename, std::ios::binary | std::ios::in );
	if( !geometry )
		return false;

	rtPushAttributeBindings();

	geometryId = rtGenGeometries( 1 );
	rtNewGeometry( geometryId );

	rtSetAttributeBinding( RT_NORMAL, RT_BIND_PER_MATERIAL );
	rtSetAttributeBinding( RT_COLOR, RT_BIND_PER_MATERIAL );

	unsigned int matId = rtGenMaterials( 1 );
	rtBindMaterial( matId );
	rtMaterialClass( new rtl::HeadlightColor );

	rtu::float3 vertex;

	// Load geometry data
	rtBegin( RT_TRIANGLES );
	while( !geometry.eof() )
	{
		geometry.read( (char*)vertex.ptr(), 3*sizeof(float) );
		rtVertex3fv( vertex.ptr() );
		geometry.read( (char*)vertex.ptr(), 3*sizeof(float) );
		rtVertex3fv( vertex.ptr() );
		geometry.read( (char*)vertex.ptr(), 3*sizeof(float) );
		rtVertex3fv( vertex.ptr() );
	}
	rtEnd();

	rtEndGeometry();
	rtBindMaterial( 0 );
	rtPopAttributeBindings();

	return true;
}

// Accepts any file format supported by OpenSceneGraph
bool rtutLoadOpenSceneGraph( char* filename, unsigned int& geometryId )
{
	rtut::OsgGeometryLoader loader;
	return loader.loadFile( filename, geometryId );
}
