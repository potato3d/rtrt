#include <rtc/Transform.h>

namespace rtc {

void Transform::setMatrix( const rtu::float4x4& matrix )
{
	_matrix = matrix;
	_inverseMatrix = matrix;
	_inverseMatrix.invert();
	_inverseTransposedMatrix = matrix;
	_inverseTransposedMatrix.transpose();
	_inverseTransposedMatrix.invert();
}

void Transform::transformVertex( rtu::float3& vertex ) const
{
	_matrix.transform( vertex );
}

void Transform::transformNormal( rtu::float3& normal ) const
{
	// (M^T)^-1
	// Inverse of transposed matrix, first transpose then invert!
	_inverseTransposedMatrix.transform( normal );
}

} // namespace rtc
