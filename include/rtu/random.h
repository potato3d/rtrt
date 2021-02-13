#pragma once
#ifndef _RTU_RANDOM_H_
#define _RTU_RANDOM_H_

#include <rtu/common.h>

namespace rtu {

/**
 *	A Pseudo-Random Number Generator.
 */
class  Random
{
public:
	//! Initializes the PRNG using a DWORD.
	static void seed( uint32 s );

	//! Initializes the PRNG using an array of DWORDs. 
	static void seed( uint32 *array, uint32 size );

	/**
	 *	If the PRNG hasn't been initialized with the auto-seeder yet, (re-)initializes it
	 *	automatically using high-entropy data sources.
	 *
	 *	@return true if the auto-seeder was initialized for the first time.
	 *			false if it had been initialized before (in this session).
	 */
	static bool autoSeed();

	//! Generates a random uint32 on the [0,0xFFFFFFFF] interval.
	static uint32 integer32();

	//! Generates a random int32 on the [0,0x7FFFFFFF] interval (always positive).
	inline static int32 integer31()
	{
		return static_cast<int32>( integer32() >> 1 );
	}

	//! Generates a random real number on the [0,1] interval (32 bits resolution).
	inline static double realInIn()
	{
		return ( integer32() * ( 1.0 / 4294967295.0 ) ); 
	}

	//! Generates a random real number on the [0,1) interval (32 bits resolution).
	inline static double realInOut()
	{
		return ( integer32() * ( 1.0 / 4294967296.0 ) ); 
	}

	//! Generates a random real number on the (0,1) interval (32 bits resolution).
	inline static double realOutOut()
	{
		return ( ( static_cast<double>( integer32() ) + 0.5 ) * ( 1.0 / 4294967296.0 ) ); 
	}

	//! Generates a random real number on the [0,1) interval (53 bits resolution).
	inline static double realHiRes()
	{
		uint32 a = ( integer32() >> 5 );
		uint32 b = ( integer32() >> 6 );
		return ( ( a * 67108864.0 + b ) * ( 1.0 / 9007199254740992.0 ) );
	}

	inline static double real( double min, double max )
	{
		return ( min + ( max - min ) * realInIn() );
	}

private:
	enum // period parameters
	{
		N			= 624,
		M			= 397,
		MATRIX_A	= 0x9908B0DF,
		UPPER_MASK	= 0x80000000,
		LOWER_MASK	= 0x7FFFFFFF
	};

	// internal RNG state
	static uint32 mt[N];
	static int32 mti;
};

} // namespace rtu

#endif
