#pragma once
#ifndef _RTU_STL_H_
#define _RTU_STL_H_

#include <rtu/common.h>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

namespace rtu {

/************************************************************************/
/* Vector                                                               */
/************************************************************************/

// Sets new capacity equal to new size, do not grow vector by half
// Used when exact number of elements is previously known, optimizing memory allocation
template<typename T>
inline static void vectorExactResize( std::vector<T>& v, unsigned int newSize )
{
	v.reserve( newSize );
	v.resize( newSize );
}

// Sets new capacity equal to new size, do not grow vector by half
// Used when exact number of elements is previously known, optimizing memory allocation
// The value 'pad' is used for new elements added if the new size is larger that the original size
template<typename T>
inline static void vectorExactResize( std::vector<T>& v, unsigned int newSize, T& pad )
{
	v.reserve( newSize );
	v.resize( newSize, pad );
}

//////////////////////////////////////////////////////////////////////////
// The following need to duplicate vector data.
// They are not recommended for very large vectors.
//////////////////////////////////////////////////////////////////////////

// Sets vector to interval [first, last], erases all other data
template<typename T>
inline static void vectorClip( std::vector<T>& v, unsigned int first, unsigned int last )
{
	std::vector<T>( v.begin() + first, v.begin() + last ).swap( v );
}

// Sets capacity equal to current size. Free wasted memory.
template<typename T>
inline static void vectorTrim( std::vector<T>& v )
{
	std::vector<T>( v ).swap( v );
}

// Sets size and capacity to zero. Free all memory.
template<typename T>
inline static void vectorFreeMemory( std::vector<T>& v )
{
	std::vector<T>().swap( v );
}

/************************************************************************/
/* String                                                               */
/************************************************************************/
static const char STL_SPACES[] = " \t\r\n";

// Coverts all characters of the string to uppercase
inline static void stringToUppercase( std::string& s )
{
	std::transform( s.begin(), s.end(), s.begin(), toupper );
}

// Coverts all characters of the string to lowercase
inline static void stringToLowercase( std::string& s )
{
	std::transform( s.begin(), s.end(), s.begin(), tolower );
}

// Removes characters equal to 't' from the right of the string
inline static void stringTrimRight( std::string& s, const char* t = STL_SPACES )
{ 
	std::string::size_type i( s.find_last_not_of( t ) );
	if( i == std::string::npos )
		s.clear();
	else
		s = s.erase( i + 1 );
}

// Removes characters equal to 't' from the left of the string
inline static void stringTrimLeft( std::string& s, const char* t = STL_SPACES )
{ 
	s = s.erase( 0, s.find_first_not_of( t ) );
}

// Removes characters equal to 't' from the left and right of the string
inline static void stringTrim( std::string& s, const char* t = STL_SPACES )
{ 
	stringTrimRight( s, t );
	stringTrimLeft( s, t );
}

// Transformation function for stringToCapitals that has a "state" so it can capitalize a sequence
class ToCapitals : public std::unary_function<char,char>
{
	bool bUpper;
public:
	// First letter in string will be in capitals
	ToCapitals () : bUpper( true ) {}

	char operator()( const char& c )
	{ 
		char c1;
		// Capitalize depending on previous letter
		if( bUpper )
			c1 = toupper( c );
		else
			c1 = tolower( c );

		// Work out whether next letter should be capitals
		bUpper = isalnum(c) == 0;
		return c1; 
	}
};

// Capitalizes the given string 
inline static void stringToCapitals( std::string& s )
{
	std::transform( s.begin(), s.end(), s.begin(), ToCapitals() );
}

// Split a line into the first word, and rest-of-the-line
inline static void stringExtractWord( std::string& word, std::string& s, 
											 const std::string& delim = " ", 
											 const bool trimSpaces = true )
{
	// find delimiter  
	std::string::size_type i( s.find( delim ) );

	// split into before and after delimiter
	word = s.substr( 0, i );

	// if no delimiter, remainder is empty
	if( i == std::string::npos )
		s.erase();
	else
		// Erase up to the delimiter
		s.erase( 0, i + delim.size() );

	// trim spaces if required
	if( trimSpaces )
	{
		stringTrim( word );
		stringTrim( s );
	}
}

// Get the file extension (without the dot) from filename
inline static void stringGetExtension( std::string& extension, const std::string& filename )
{
	std::string::size_type i( filename.find_last_of( '.' ) );
	extension = filename.substr( i + 1, filename.length() - i - 1 );
}

} // namespace rtu

#endif // _RTU_STL_H_
