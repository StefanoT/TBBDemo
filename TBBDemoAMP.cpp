// Intel TBB Demo
// by Stefano Tommesani (www.tommesani.com) 2013
// this code is release under the Code Project Open License (CPOL) http://www.codeproject.com/info/cpol10.aspx
// The main points subject to the terms of the License are:
// -   Source Code and Executable Files can be used in commercial applications;
// -   Source Code and Executable Files can be redistributed; and
// -   Source Code can be modified to create derivative works.
// -   No claim of suitability, guarantee, or any warranty whatsoever is provided. The software is provided "as-is".
// -   The Article(s) accompanying the Work may not be distributed or republished without the Author's consent

#include <iostream>
using namespace std;
#include <amp.h>
using namespace concurrency;

#include "TBBBenchmark.h"
#include "TBBDemoRoutines.h"

// constants for RGB to Y conversion
// Ey = 0.299*Er + 0.587*Eg + 0.114*Eb
const int SCALING_LOG = 15;
const int SCALING_FACTOR = (1 << SCALING_LOG);
const int Y_RED_SCALE = (int)(0.299 * SCALING_FACTOR);
const int Y_GREEN_SCALE = (int)(0.587 * SCALING_FACTOR);
const int Y_BLUE_SCALE = (int)(0.114 * SCALING_FACTOR);

void inspect_accelerators()
{
    auto accelerators = accelerator::get_all();
    for_each(begin(accelerators), end(accelerators),[=](accelerator acc){ 
        wcout << "New accelerator: " << acc.description << endl;
        wcout << "is_debug = " << acc.is_debug << endl;
        wcout << "is_emulated = " << acc.is_emulated <<endl;
        wcout << "dedicated_memory = " << acc.dedicated_memory << endl;
        wcout << "device_path = " << acc.device_path << endl;
        wcout << "has_display = " << acc.has_display << endl;                
        wcout << "version = " << (acc.version >> 16) << '.' << (acc.version & 0xFFFF) << endl;
    });
}

// ProcessRGBAMP
// C++ AMP GPGPU version of Serial code
// both input and output image are unidimensional arrays of ImageWidth * ImageHeight pixels
// assumes that PixelOffset is 4 (RGBA image) and that ImageWidth * ImageHeight * PixelOffset is a multiple of 16

void ProcessRGBAMP(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset)
{
	array_view<const unsigned int, 1> SourceImageView(ImageWidth * ImageHeight, (unsigned int *)SourceImage);
	array_view<unsigned int, 1> YImageView((ImageWidth * ImageHeight) >> 2, (unsigned int *)YImage);
	YImageView.discard_data();

	parallel_for_each(YImageView.extent, [=] (index<1> idx) restrict(amp) {
		unsigned int YValue = 0;
		for (int index = 0; index < 4; index++)
		{
			unsigned int RGBPixel = SourceImageView[idx * 4 + index];
			unsigned int PartialYValue = ((RGBPixel & 0xFF) * Y_RED_SCALE) + (((RGBPixel >> 8) & 0xFF) * Y_GREEN_SCALE) + (((RGBPixel >> 16) & 0xFF) * Y_BLUE_SCALE);
			PartialYValue += 1 << (SCALING_LOG - 1);
			PartialYValue >>= SCALING_LOG;
			//if (PartialYValue > 255)
			//	PartialYValue = 255;
			PartialYValue = concurrency::direct3d::clamp(PartialYValue, 0, 255);
			YValue |= PartialYValue << (index * 8);
		}
		YImageView[idx] = YValue;
	});

	YImageView.synchronize();
}