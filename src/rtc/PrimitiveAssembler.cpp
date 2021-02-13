#include <rtc/PrimitiveAssembler.h>
#include <rtc/Scene.h>
#include <rt/definitions.h>

namespace rtc {

void AttributeBinding::reset()
{
	normalBinding = RT_BIND_PER_VERTEX;
	colorBinding = RT_BIND_PER_VERTEX;
	textureBinding = RT_BIND_PER_VERTEX;
}

PrimitiveAssembler::PrimitiveAssembler()
{
	_geometryId = 0;
	_primitiveType = RT_TRIANGLES;
	_materialId = 0;
	_startVertex = 0;
	_bindings.reset();

	rtu::float4x4 m;
	m.makeIdentity();
	_transform.setMatrix( m );

	_currentColor.set( 1.0f, 1.0f, 1.0f );
	_currentNormal.set( 0.0f, 0.0f, 1.0f );
}

void PrimitiveAssembler::beginPrimitiveData( unsigned int geometryId, unsigned int primitiveType, unsigned int materialId )
{
	_geometryId		= geometryId;
	_primitiveType	= primitiveType;
	_materialId		= materialId;

	_startVertex	= Scene::geometries.at( _geometryId ).vertices.size();
}

void PrimitiveAssembler::setMatrixTransform( const rtu::float4x4& matrix )
{
	_transform.setMatrix( matrix );
}

void PrimitiveAssembler::setBindings( AttributeBinding& bindings )
{
	_bindings = bindings;
}

void PrimitiveAssembler::setColor( const rtu::float3& color )
{
	_currentColor = color;
}

void PrimitiveAssembler::setNormal( const rtu::float3& normal )
{
	_currentNormal = normal;
}

void PrimitiveAssembler::setTexCoord( const rtu::float3& texCoord )
{
	_currentTexCoord = texCoord;
}

void PrimitiveAssembler::addVertex( const rtu::float3& vertex )
{
	Geometry& geometry = Scene::geometries.at( _geometryId );

	rtu::float3 transformedVertex( vertex );
	_transform.transformVertex( transformedVertex );
	geometry.vertices.push_back( transformedVertex );

	if( _bindings.normalBinding == RT_BIND_PER_VERTEX )
	{
		rtu::float3 transformedNormal( _currentNormal );
		_transform.transformNormal( transformedNormal );
		transformedNormal.normalize();
		geometry.normals.push_back( transformedNormal );
	}

	if( _bindings.colorBinding == RT_BIND_PER_VERTEX )
	{
		geometry.colors.push_back( _currentColor );
	}

	if( _bindings.textureBinding == RT_BIND_PER_VERTEX )
	{
		geometry.texCoords.push_back( _currentTexCoord );
	}
}

void PrimitiveAssembler::endPrimitiveData()
{
	switch( _primitiveType )
	{
	case RT_TRIANGLES:
		updateTriangles();
		break;
	case RT_TRIANGLE_STRIP:
		updateTriangleStrip();
		break;
	default:
		// TODO: warning message
		break;
	}
}

void PrimitiveAssembler::updateTriangles()
{
	Geometry& geometry = Scene::geometries.at( _geometryId );
	unsigned int idx = _startVertex;
	unsigned int limit = geometry.vertices.size();
	TriDesc triangle;

	while( idx < limit )
	{
		// Setup TriDesc
		triangle.v0 = idx;
		++idx;
		triangle.v1 = idx;
		++idx;
		triangle.v2 = idx;
		++idx;
		triangle.materialId = _materialId;

		// Setup TriAccel
		TriAccel accel;
		accel.buildFrom( geometry.vertices[triangle.v0], geometry.vertices[triangle.v1], geometry.vertices[triangle.v2] );

		// Only store valid triangles
		if( accel.valid() )
		{
			accel.triangleId = geometry.triDesc.size();
			geometry.triDesc.push_back( triangle );
			geometry.triAccel.push_back( accel );
		}
	}
}

void PrimitiveAssembler::updateTriangleStrip()
{
	Geometry& geometry = Scene::geometries.at( _geometryId );
	int idx = (int)_startVertex;
	int limit = geometry.vertices.size() - 2;
	TriDesc triangle;
	bool inverted = false;

	// Vertices v1 and v2 are shared between current triangle and next one
	while( idx < limit )
	{
		// Setup TriDesc
		if( inverted )
		{
			triangle.v0 = idx + 2;
			triangle.v1 = idx + 1;
			triangle.v2 = idx;
		}
		else
		{
			triangle.v0 = idx;
			triangle.v1 = idx + 1;
			triangle.v2 = idx + 2;
		}
		triangle.materialId = _materialId;

		// Setup TriAccel
		TriAccel accel;
		accel.buildFrom( geometry.vertices[triangle.v0], geometry.vertices[triangle.v1], geometry.vertices[triangle.v2] );

		// Only store valid triangles
		if( accel.valid() )
		{
			accel.triangleId = geometry.triDesc.size();
			geometry.triDesc.push_back( triangle );
			geometry.triAccel.push_back( accel );
		}

		// To next triangle
		++idx;
		inverted ^= true;
	}
}

} // namespace rtc
