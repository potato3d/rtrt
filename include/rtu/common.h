#pragma once
#ifndef _RTU_COMMON_H_
#define _RTU_COMMON_H_

#ifndef __cplusplus
	#error The RTU library requires a C++ compiler.
#endif

#include "platform.h"

/*!
	Evaluates a constant expression and, if the result is false, aborts the
	compilation with an error message.

	\param const_expr is a compile-time integral or pointer expression.
	\param id_msg is a C++ identifier that describes the error (it does not
		need to be defined). Something like 'invalid_element_size'.
!*/
#define RTU_STATIC_CHECK( const_expr, id_msg ) \
	{ rtu::CompileTimeError<( (const_expr) != 0 )> ERROR_##id_msg; (void)ERROR_##id_msg; } 

namespace rtu {

// Template trick to yield compile time errors:
template<int> struct CompileTimeError;
template<> struct CompileTimeError<true> {};

} // namespace rtu

#endif
