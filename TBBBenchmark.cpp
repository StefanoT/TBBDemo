// Intel TBB Demo
// by Stefano Tommesani (www.tommesani.com) 2013
// this code is release under the Code Project Open License (CPOL) http://www.codeproject.com/info/cpol10.aspx
// The main points subject to the terms of the License are:
// -   Source Code and Executable Files can be used in commercial applications;
// -   Source Code and Executable Files can be redistributed; and
// -   Source Code can be modified to create derivative works.
// -   No claim of suitability, guarantee, or any warranty whatsoever is provided. The software is provided "as-is".
// -   The Article(s) accompanying the Work may not be distributed or republished without the Author's consent

#include "TBBBenchmark.h"
#include <Windows.h>

// benchmark data
LARGE_INTEGER StartTime, StopTime;

void StartBenchmark()
{
	QueryPerformanceCounter(&StartTime);
}

void StopBenchmark(std::string Description)
{
	QueryPerformanceCounter(&StopTime);
	std::cout << " Elapsed time of " << Description;
	std::cout.width(max(0, 20 - Description.length()));
	std::cout << (int)(StopTime.QuadPart - StartTime.QuadPart) << std::endl;
}