#pragma once
#ifndef _RTC_PRIMITIVEASSEMBLER_H_
#define _RTC_PRIMITIVEASSEMBLER_H_

#include <rtu/common.h>
#include <rtc/Transform.h>

namespace rtc {

struct AttributeBinding 
{
	void reset();

	unsigned int normalBinding;
	unsigned int colorBinding;
	unsigned int textureBinding;
};

struct PrimitiveAssembler
{
	PrimitiveAssembler();

	void beginPrimitiveData( unsigned int geometryId, unsigned int primitiveType, unsigned int materialId );
	void setMatrixTransform( const rtu::float4x4& matrix );

	void setBindings( AttributeBinding& bindings );

	void setColor( const rtu::float3& color );
	void setNormal( const rtu::float3& normal );
	void setTexCoord( const rtu::float3& texCoord );
	void addVertex( const rtu::float3& vertex );

	void endPrimitiveData();

private:
	void updateTriangles();
	void updateTriangleStrip();

	unsigned int _geometryId;
	unsigned int _primitiveType;
	unsigned int _materialId;
	unsigned int _startVertex;
	Transform _transform;
	AttributeBinding _bindings;

	rtu::float3 _currentColor;
	rtu::float3 _currentNormal;
	rtu::float3 _currentTexCoord;
};

} // namespace rtc

#endif // _RTC_PRIMITIVEASSEMBLER_H_
