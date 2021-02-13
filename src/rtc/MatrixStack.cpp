#include <rtc/MatrixStack.h>

namespace rtc {

MatrixStack::MatrixStack()
{
	_stack.push( rtu::float4x4() );
	loadIdentity();
}

const rtu::float4x4& MatrixStack::top()
{
	return _stack.top();
}

void MatrixStack::pushMatrix()
{
	_stack.push( _stack.top() );
}

void MatrixStack::loadIdentity()
{
	_stack.top().makeIdentity();
}

void MatrixStack::scale( float x, float y, float z )
{
	rtu::float4x4 m;
	m.makeScale( x, y, z );
	_stack.top().product( _stack.top(), m );
}

void MatrixStack::translate( float x, float y, float z )
{
	rtu::float4x4 m;
	m.makeTranslation( x, y, z );
	_stack.top().product( _stack.top(), m );
}

void MatrixStack::rotate( float degrees, float x, float y, float z )
{
	rtu::float4x4 m;
	m.makeRotation( rtu::mathf::toRadians( degrees ), x, y, z );
	_stack.top().product( _stack.top(), m );
}

void MatrixStack::loadMatrix( const float* const matrix )
{
	_stack.top().set( matrix );
}

void MatrixStack::multMatrix( const float* const matrix )
{
	_stack.top().product( _stack.top(), rtu::float4x4( matrix ) );
}

void MatrixStack::popMatrix()
{
	if( _stack.size() <= 1 )
		return;

	_stack.pop();
}

} // namespace rtc
