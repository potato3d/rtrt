#pragma once
#ifndef _RTC_STACK_H_
#define _RTC_STACK_H_

#include <rtu/common.h>

namespace rtc {

// Unsafe implementation!!!
// Do not push beyond stack size!
// Only pop if stack isn't empty!
template<typename T>
class Stack
{
public:
	inline Stack( unsigned int size );
	inline ~Stack();

    inline void clear();

	inline void push();
	inline void pop();

	inline T& top();

	inline bool empty() const;

private:
	T* _data;
	int _topIdx;
};

template<typename T>
Stack<T>::Stack( unsigned int size )
: _data( new T[size] ), _topIdx( -1 )
{
}

template<typename T>
Stack<T>::~Stack()
{
	if( _data != NULL )
		delete [] _data;
}

template<typename T>
void Stack<T>::clear()
{
    _topIdx = -1;
}

template<typename T>
void Stack<T>::push()
{
	++_topIdx;
}

template<typename T>
void Stack<T>::pop()
{
	--_topIdx;
}

template<typename T>
T& Stack<T>::top()
{
	return _data[_topIdx];
}

template<typename T>
bool Stack<T>::empty() const
{
	return ( _topIdx < 0 );
}

// Static stack
// Needs to be cleared before use
template<typename T, unsigned int SIZE>
class StaticStack
{
public:
    inline void clear();

    inline void push();
    inline void pop();

    inline T& top();

    inline bool empty() const;

//private:
    T data[SIZE];
    int topIdx;
};

template<typename T, unsigned int SIZE>
void StaticStack<T, SIZE>::clear()
{
    topIdx = -1;
}

template<typename T, unsigned int SIZE>
void StaticStack<T, SIZE>::push()
{
    ++topIdx;
}

template<typename T, unsigned int SIZE>
void StaticStack<T, SIZE>::pop()
{
    --topIdx;
}

template<typename T, unsigned int SIZE>
T& StaticStack<T, SIZE>::top()
{
    return data[topIdx];
}

template<typename T, unsigned int SIZE>
bool StaticStack<T, SIZE>::empty() const
{
    return ( topIdx < 0 );
}

} // namespace rtc

#endif // _RTC_STACK_H_
