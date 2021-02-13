//#include "precompiled.h"
#include <rtu/refCounting.h>
#include <rtu/exception.h>
#include <strstream>

namespace rtu {

RefCounted::~RefCounted()
{
	if( _refCount > 0 )
	{
		std::strstream message;
		message << "Warning: deleting still referenced object " << this
				<< ". The final reference count was " << _refCount
				<< ", memory corruption possible.";
		throw Exception( message.str() );
	}
}

}
