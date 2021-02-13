//#include "precompiled.h"
#include <rtu/exception.h>

namespace rtu {

Exception::~Exception()
{
	// empty
}

const char * Exception::what() const
{
	return m_message.c_str();
}

}
