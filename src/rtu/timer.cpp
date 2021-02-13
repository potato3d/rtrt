//#include "precompiled.h"
#include <rtu/timer.h>

namespace rtu {

const double Timer::SECS_PER_TICK = computeSecsPerTick();

// computeSecsPerTick() platform-specific implementation:
#ifdef _WIN32
	#include <windows.h>
	double Timer::computeSecsPerTick()
	{
		// if QPC is available, use it to determine the TSC rate
		LARGE_INTEGER frequency;
		if( QueryPerformanceFrequency( &frequency ) )
		{
			LARGE_INTEGER qpcStart, qpcStop, qpcStartAfter, qpcStopAfter;
			Stamp tscStart, tscStop;
			double qpcSecsPerClock = ( 1.0 / frequency.QuadPart );
			double elapsed, last, current, bound;

			QueryPerformanceCounter( &qpcStart );
			tscStart = tick();
			QueryPerformanceCounter( &qpcStartAfter );

			current = 0;
			do
			{
				// store the seconds per clock
				last = current;
				// read the clocks
				QueryPerformanceCounter( &qpcStop );
				tscStop = tick();
				QueryPerformanceCounter( &qpcStopAfter );
				// average before and after to approximate reading
				// both clocks at the same time
				elapsed = ( ( ( qpcStop.QuadPart + qpcStopAfter.QuadPart ) - 
							( qpcStart.QuadPart + qpcStartAfter.QuadPart ) ) /
							2.0 * qpcSecsPerClock );
				// TSC seconds per clock
				current = elapsed / ( tscStop - tscStart );
				// calculate a bound to check against
				bound = ( current / 1000000 );
				
			} // break if current-bound<last && current+bound>last
			while( ( current - bound ) > last || ( current + bound ) < last );

			return current;
		}
		else
		{
			Stamp startTime = tick();
			Sleep( 1000 );
			Stamp endTime = tick();
			return ( 1.0 / static_cast<double>( endTime - startTime ) );
		}
	}

#elif defined(unix) || defined(__linux) || defined(__FreeBSD__) || defined(__CYGWIN__) || defined (__APPLE__) || defined (macintosh)

	double Timer::computeSecsPerTick()
	{
		// gettimeofday()'s precision.
		return 1e-6;
	}

#else 

	double Timer::computeSecsPerTick()
	{
		return ( 1.0 / CLOCKS_PER_SEC );
	}

#endif

}
