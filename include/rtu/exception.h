#pragma once
#ifndef _RTU_EXCEPTION_H_
#define _RTU_EXCEPTION_H_

#include <rtu/common.h>
#include <string>
#include <exception>

namespace rtu {

/**
 *	General Exception class for the TecVR library.
 *	Base of all exception classes in the TecVR group.
 */
class  Exception : public std::exception
{
public:
	//! Constructs an Exception with the specified message.
	Exception( const std::string &message ) : m_message( message )
	{;}

	//! Constructs a copy of an Exception.
	Exception( const Exception &other )
	{
		*this = other;
	}

	//! Destructs the Exception.
	virtual ~Exception();

	//! Returns the exception message.
	const std::string & message() const
	{
		return m_message;
	}

	//! Updates the exception message.
	void setMessage( const std::string &message )
	{
		m_message = message;
	}

	//! Returns the exception message.
	virtual const char * what() const;

	//! Copy operator.
	Exception & operator=( const Exception &other )
	{
		if( &other != this )
			m_message = other.m_message;
		return *this;
	}

private:
	std::string m_message;
};

} // namespace rtu

#endif
