#pragma once
#ifndef _RTC_MATRIXSTACK_H_
#define _RTC_MATRIXSTACK_H_

#include <rtu/common.h>
#include <rtu/float4x4.h>
#include <stack>

namespace rtc {

class MatrixStack
{
public:
	MatrixStack();

	const rtu::float4x4& top();

	void pushMatrix();
	void loadIdentity();
	void scale( float x, float y, float z );
	void translate( float x, float y, float z );
	void rotate( float degrees, float x, float y, float z );
	void loadMatrix( const float* const matrix );
	void multMatrix( const float* const matrix );
	void popMatrix();

private:
	std::stack<rtu::float4x4> _stack;
};

} // namespace rtc

#endif // _RTC_MATRIXSTACK_H_
