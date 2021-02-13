/* 
	License of the Mersenne Twister PRNG implementation:

	Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
	All rights reserved.                          

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.

	3. The names of its contributors may not be used to endorse or promote 
	products derived from this software without specific prior written 
	permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//#include "precompiled.h"
#include <rtu/random.h>
#include <rtu/math.h>
#include <Windows.h>

namespace rtu {

void Random::seed( uint32 s )
{
	mt[0] = s;
	for( mti = 1; mti < N; ++mti )
	{
		mt[mti] = ( 1812433253 * ( mt[mti - 1] ^ ( mt[mti - 1] >> 30 ) ) + mti );
	}
}

void Random::seed( uint32 *array, uint32 size )
{
	seed( 19650218 );

	uint32 i = 1, j = 0;

	for( uint32 k = rtu::mathf::max( static_cast<uint32>( N ), size ); k; k-- )
	{
		mt[i] = ( mt[i] ^ ( ( mt[i - 1] ^ ( mt[i - 1] >> 30 ) ) * 1664525 ) ) + array[j] + j;
		++i; ++j;
		if( i >= N )
		{
			mt[0] = mt[N - 1];
			i = 1;
		}
		if( j >= size )
			j = 0;
	}

	for( uint32 k = N - 1; k; k-- )
	{
		mt[i] = ( mt[i] ^ ( ( mt[i - 1] ^ ( mt[i - 1] >> 30 ) ) * 1566083941 ) ) - i;
		i++;
		if( i >= N )
		{
			mt[0] = mt[N - 1];
			i = 1;
		}
	}

	// MSB is 1; assuring non-zero initial array
	mt[0] = 0x80000000;
}

bool Random::autoSeed()
{
	static bool sIsAutoSeeded = false;

	if( sIsAutoSeeded )
		return false;

	uint8 buffer[512];
	sIsAutoSeeded = true;

#ifdef _WIN32

	HCRYPTPROV cryptProvider;
	CryptAcquireContext( &cryptProvider, NULL, NULL, PROV_RSA_FULL, 0 );
	CryptGenRandom( cryptProvider, sizeof(buffer), buffer );
	CryptReleaseContext( cryptProvider, 0 );

#else // UNIX

	FILE *devrandom = fopen( "/dev/urandom", "r" );
	for( int32 i = 0; i < sizeof(buffer); ++i )
	{
		buffer[i] ^= static_cast<uint8>( fgetc( devrandom ) );
	}
	fclose( devrandom );

#endif

	seed( reinterpret_cast<uint32 *>( buffer ), sizeof(buffer)/sizeof(uint32) );
	return true;
}

uint32 Random::integer32()
{
	static const uint32 mag01[2] = { 0x0, MATRIX_A };

	uint32 y;

	// are there words left? if not, generate N words at one time...
	if( mti >= N )
	{
		// if seed() has not been called, a default initial seed is used
		if( mti == ( N + 1 ) )
			seed( 5489 );

		int32 kk;
		for( kk = 0; kk < ( N - M ); ++kk )
		{
			y = ( ( mt[kk] & UPPER_MASK ) | ( mt[kk + 1] & LOWER_MASK ) );
			mt[kk] = ( mt[kk + M] ^ ( y >> 1 ) ^ mag01[y & 0x00000001] );
		}

		for( ; kk < ( N - 1 ); ++kk )
		{
			y = ( ( mt[kk] & UPPER_MASK ) | ( mt[kk + 1] & LOWER_MASK ) );
			mt[kk] = ( mt[kk + ( M - N )] ^ ( y >> 1 ) ^ mag01[y & 0x00000001] );
		}

		y = ( ( mt[N - 1] & UPPER_MASK ) | ( mt[0] & LOWER_MASK ) );
		mt[N - 1] = ( mt[M - 1] ^ ( y >> 1 ) ^ mag01[y & 0x00000001] );

		mti = 0;
	}

	y = mt[mti++];

	// tempering
	y ^= ( y >> 11 );
	y ^= ( ( y << 7 ) & 0x9D2C5680 );
	y ^= ( ( y << 15 ) & 0xEFC60000 );
	y ^= ( y >> 18 );

	return y;
}

uint32 Random::mt[N];
int32 Random::mti( N + 1 );

}
