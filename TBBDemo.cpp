// Intel TBB Demo
// by Stefano Tommesani (www.tommesani.com) 2013
// this code is release under the Code Project Open License (CPOL) http://www.codeproject.com/info/cpol10.aspx
// The main points subject to the terms of the License are:
// -   Source Code and Executable Files can be used in commercial applications;
// -   Source Code and Executable Files can be redistributed; and
// -   Source Code can be modified to create derivative works.
// -   No claim of suitability, guarantee, or any warranty whatsoever is provided. The software is provided "as-is".
// -   The Article(s) accompanying the Work may not be distributed or republished without the Author's consent

// TBBDemo.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"

#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
#include "TBBBenchmark.h"
#include "TBBDemoRoutines.h"
#include "TBBDemoAMP.h"

typedef void (*TBBDemoFunction)(void *, void *, int, int, int);
typedef std::pair<TBBDemoFunction, std::string> TBBDemoImplementation;

int _tmain(int argc, _TCHAR* argv[])
{
	const int DEFAULT_IMAGE_WIDTH = 8 * 1024;
	const int DEFAULT_IMAGE_HEIGHT = 8 * 1024;
	const int DEFAULT_IMAGE_SIZE = (DEFAULT_IMAGE_WIDTH * DEFAULT_IMAGE_HEIGHT);
	const int RGBA_PIXEL_SIZE = 4;

	cout << "Intel TBB Demo by Stefano Tommesani (www.tommesani.com)" << endl;

	// create input image
	unsigned char *RGBAImage = new unsigned char[DEFAULT_IMAGE_SIZE * RGBA_PIXEL_SIZE];
	// fill RGBA image with random data
	srand(0x5555);
	for (int j = 0; j < (DEFAULT_IMAGE_SIZE * RGBA_PIXEL_SIZE); j++)
		RGBAImage[j] = rand() % 256;
	// create output image
	unsigned char *GrayImage = new unsigned char[DEFAULT_IMAGE_SIZE];

	// checks for correctness have been moved to unit testing project

	// check performance
	std::vector<TBBDemoImplementation> Implementations;
	Implementations.push_back(TBBDemoImplementation(&ProcessRGBSerial, "Serial"));
	Implementations.push_back(TBBDemoImplementation(&ProcessRGBTBB1, "TBB1"));
	Implementations.push_back(TBBDemoImplementation(&ProcessRGBTBB2, "TBB2"));
	Implementations.push_back(TBBDemoImplementation(&ProcessRGBTBB3, "TBB3"));
	Implementations.push_back(TBBDemoImplementation(&ProcessRGBSIMD, "SIMD1"));
	Implementations.push_back(TBBDemoImplementation(&ProcessRGBSIMD2, "SIMD2"));
	Implementations.push_back(TBBDemoImplementation(&ProcessRGBSIMD3, "SIMD3"));
	Implementations.push_back(TBBDemoImplementation(&ProcessRGBTBBSIMD, "TBB SIMD"));
	Implementations.push_back(TBBDemoImplementation(&ProcessRGBAMP, "AMP GPGPU"));

	const int PERFORMANCE_LOOPS = 5;  //< multiple loops to minimize benchmarking errors
	for (int i = 1; i <= PERFORMANCE_LOOPS; i++)
	{
		cout << "---- Iteration " << i << " ---- " << endl;
		for (auto ImplementationsPtr = Implementations.begin(); ImplementationsPtr != Implementations.end(); ImplementationsPtr++)
		{
			StartBenchmark();
			ImplementationsPtr->first(RGBAImage, GrayImage, DEFAULT_IMAGE_WIDTH, DEFAULT_IMAGE_HEIGHT, RGBA_PIXEL_SIZE);
			StopBenchmark(ImplementationsPtr->second);
		}
	}

	Implementations.clear();
	// free images
	delete[] RGBAImage;
	delete[] GrayImage;
	// wait for user to enter a key before closing
	char WaitForUser;
	cin >> WaitForUser;
	return 0;
}
