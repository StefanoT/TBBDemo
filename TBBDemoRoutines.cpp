// Intel TBB Demo
// by Stefano Tommesani (www.tommesani.com) 2013
// this code is release under the Code Project Open License (CPOL) http://www.codeproject.com/info/cpol10.aspx
// The main points subject to the terms of the License are:
// -   Source Code and Executable Files can be used in commercial applications;
// -   Source Code and Executable Files can be redistributed; and
// -   Source Code can be modified to create derivative works.
// -   No claim of suitability, guarantee, or any warranty whatsoever is provided. The software is provided "as-is".
// -   The Article(s) accompanying the Work may not be distributed or republished without the Author's consent

#include <Windows.h>

// Intel TBB library
#include <task_scheduler_init.h>
using namespace tbb;
#include <parallel_for.h>
#include <blocked_range.h>
#include <blocked_range2d.h>

// SIMD intrinsics
#include <emmintrin.h>

// constants for RGB to Y conversion
// Ey = 0.299*Er + 0.587*Eg + 0.114*Eb
const int SCALING_LOG = 15;
const int SCALING_FACTOR = (1 << SCALING_LOG);
const int Y_RED_SCALE = (int)(0.299 * SCALING_FACTOR);
const int Y_GREEN_SCALE = (int)(0.587 * SCALING_FACTOR);
const int Y_BLUE_SCALE = (int)(0.114 * SCALING_FACTOR);

// ProcessRGBSerial
// reference serial code that converts a given RGB image to a luma-only image
// both input and output image are unidimensional arrays of ImageWidth * ImageHeight pixels
// PixelOffset is the distance in bytes between two pixels in the input RGB image (RGB24 -> PixelOffset = 3, RGBA32 -> PixelOffset = 4)

void ProcessRGBSerial(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset)
{
	unsigned char *SourceImagePtr = (unsigned char *)SourceImage;
	unsigned char *YImagePtr = (unsigned char *)YImage;

    for (int i = (ImageWidth * ImageHeight); i > 0; i--)
	{
		int YValue = (SourceImagePtr[0] * Y_RED_SCALE  ) +
					 (SourceImagePtr[1] * Y_GREEN_SCALE) +
					 (SourceImagePtr[2] * Y_BLUE_SCALE );
		SourceImagePtr += PixelOffset;
		YValue += 1 << (SCALING_LOG - 1);
		YValue >>= SCALING_LOG;
		if (YValue > 255)
			YValue = 255;
		*YImagePtr = (unsigned char)YValue;
		YImagePtr++;
	}
}

// ProcessRGBTBB1
// first version of multi-threaded code using Intel TBB
// it is necessary to define a struct (RGBToGrayConverter) with a () operator that hosts the kernel of the computation
// all the parameters necessary to the computation are copied inside the struct

struct RGBToGrayConverter {
	unsigned char *SourceImagePtr;
	unsigned char *YImagePtr;
	int PixelOffset;
	void operator( )( const blocked_range<int>& range ) const {
		unsigned char *LocalSourceImagePtr = SourceImagePtr;
		unsigned char *LocalYImagePtr = YImagePtr;
		LocalSourceImagePtr += range.begin() * PixelOffset;
		LocalYImagePtr += range.begin();
		for( int i=range.begin(); i!=range.end( ); ++i )
		{
			int YValue = (LocalSourceImagePtr[0] * Y_RED_SCALE  ) +
						 (LocalSourceImagePtr[1] * Y_GREEN_SCALE) +
						 (LocalSourceImagePtr[2] * Y_BLUE_SCALE );
			LocalSourceImagePtr += PixelOffset;
			YValue += 1 << (SCALING_LOG - 1);
			YValue >>= SCALING_LOG;
			if (YValue > 255)
				YValue = 255;
			*LocalYImagePtr = (unsigned char)YValue;
			LocalYImagePtr++;
		}
	}
};

void ProcessRGBTBB1(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset)
{
	RGBToGrayConverter RGBToGrayConverterPtr;
	RGBToGrayConverterPtr.PixelOffset = PixelOffset;
	RGBToGrayConverterPtr.SourceImagePtr = (unsigned char *)SourceImage;
	RGBToGrayConverterPtr.YImagePtr = (unsigned char *)YImage;
	
	parallel_for( blocked_range<int>( 0, (ImageWidth * ImageHeight)), RGBToGrayConverterPtr);
}

// ProcessRGBTBB2
// by using a lambda that copies the required information automatically [=], the kernel of the computation is contained
// inside the invocation to parallel_for

void ProcessRGBTBB2(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset)
{
	parallel_for( blocked_range<int>( 0, (ImageWidth * ImageHeight)),
	    [=](const blocked_range<int>& r) {
			unsigned char *LocalSourceImagePtr = (unsigned char *)SourceImage;
			unsigned char *LocalYImagePtr = (unsigned char *)YImage;
			LocalSourceImagePtr += r.begin() * PixelOffset;
			LocalYImagePtr += r.begin();

			for(size_t i = r.begin(); i != r.end(); ++i)
			{
				int YValue = (LocalSourceImagePtr[0] * Y_RED_SCALE  ) +
							 (LocalSourceImagePtr[1] * Y_GREEN_SCALE) +
							 (LocalSourceImagePtr[2] * Y_BLUE_SCALE );
				LocalSourceImagePtr += PixelOffset;
				YValue += 1 << (SCALING_LOG - 1);
				YValue >>= SCALING_LOG;
				if (YValue > 255)
					YValue = 255;
				*LocalYImagePtr = (unsigned char)YValue;
				LocalYImagePtr++;
			}
	      }
	    );
}

// ProcessRGBTBB3
// even if working with images expressed as unidimensional array does not require it, a 2D range instead of a 1D one
// works on a tile of the given images at a time
// the location and dimensions of the tile are contained in the rows and cols of the blocked_range2d

void ProcessRGBTBB3(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset)
{
	parallel_for( blocked_range2d<int,int>( 0, ImageHeight, 0, ImageWidth),
	    [=](const blocked_range2d<int,int>& r) {
			int StartX = r.cols().begin();
			int StopX = r.cols().end();
			int StartY = r.rows().begin();
			int StopY = r.rows().end();

			for (int y = StartY; y != StopY; y++)
			{
				unsigned char *LocalSourceImagePtr = (unsigned char *)SourceImage + (StartX + y * ImageWidth) * PixelOffset;
				unsigned char *LocalYImagePtr = (unsigned char *)YImage + (StartX + y * ImageWidth);
				for (int x = StartX; x != StopX; x++)
				{
					int YValue = (LocalSourceImagePtr[0] * Y_RED_SCALE ) +
							 (LocalSourceImagePtr[1] * Y_GREEN_SCALE) +
							 (LocalSourceImagePtr[2] * Y_BLUE_SCALE );
					LocalSourceImagePtr += PixelOffset;
					YValue += 1 << (SCALING_LOG - 1);
					YValue >>= SCALING_LOG;
					if (YValue > 255)
						YValue = 255;
					*LocalYImagePtr = (unsigned char)YValue;
					LocalYImagePtr++;
				}
			}
	      }
	    );
}

// ProcessRGBSIMD
// SSE2 version of Serial code
// both input and output image are unidimensional arrays of ImageWidth * ImageHeight pixels
// assumes that PixelOffset is 4 (RGBA image) and that ImageWidth * ImageHeight * PixelOffset is a multiple of 16
// does not assume that the input image is aligned on 16 bytes

void ProcessRGBSIMD(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset)
{
	__m128i RGBScale = _mm_set_epi16(0, Y_BLUE_SCALE, Y_GREEN_SCALE, Y_RED_SCALE, 0, Y_BLUE_SCALE, Y_GREEN_SCALE, Y_RED_SCALE);
	__m128i ShiftScalingAdjust = _mm_set1_epi32(1 << (SCALING_LOG - 1));
	__m128i *SourceImagePtr = (__m128i *)SourceImage;
	int *YImagePtr = (int *)YImage;

    for (int i = (ImageWidth * ImageHeight); i > 0; i -= 4)  // 4 pixels per loop
	{
		__m128i RGBValue = _mm_loadu_si128(SourceImagePtr);
		SourceImagePtr++;
		__m128i LowRGBValue = _mm_unpacklo_epi8(RGBValue, _mm_setzero_si128());
		__m128i HighRGBValue = _mm_unpackhi_epi8(RGBValue, _mm_setzero_si128());
		// int YValue = (SourceImagePtr[0] * Y_RED_SCALE  ) +
		//			 (SourceImagePtr[1] * Y_GREEN_SCALE) +
		//			 (SourceImagePtr[2] * Y_BLUE_SCALE );
		__m128i LowYValue = _mm_madd_epi16(LowRGBValue, RGBScale);
		__m128i HighYValue = _mm_madd_epi16(HighRGBValue, RGBScale);
		LowYValue = _mm_add_epi32(LowYValue, _mm_slli_epi64(LowYValue, 32));
		HighYValue = _mm_add_epi32(HighYValue, _mm_slli_epi64(HighYValue, 32));
		// YValue += 1 << (SCALING_LOG - 1);
		LowYValue = _mm_add_epi32(LowYValue, ShiftScalingAdjust);
		HighYValue = _mm_add_epi32(HighYValue, ShiftScalingAdjust);
		// YValue >>= SCALING_LOG;
		LowYValue = _mm_srli_epi64(LowYValue, 32 + SCALING_LOG);
		HighYValue = _mm_srli_epi64(HighYValue, 32 + SCALING_LOG);
		__m128i YValue =  _mm_packs_epi32(LowYValue, HighYValue);
		YValue =  _mm_packs_epi32(YValue, _mm_setzero_si128());
		// if (YValue > 255)
		//	YValue = 255;
		YValue =  _mm_packus_epi16(YValue, YValue);
		// *YImagePtr = (unsigned char)YValue;
		*YImagePtr = _mm_cvtsi128_si32(YValue);
		YImagePtr++;
	}
}

// ProcessRGBSIMD2
// SSSE3 version of Serial code
// both input and output image are unidimensional arrays of ImageWidth * ImageHeight pixels
// assumes that PixelOffset is 4 (RGBA image) and that ImageWidth * ImageHeight * PixelOffset is a multiple of 16
// does not assume that the input image is aligned on 16 bytes

void ProcessRGBSIMD2(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset)
{
	__m128i RGBScale = _mm_set_epi16(0, Y_BLUE_SCALE, Y_GREEN_SCALE, Y_RED_SCALE, 0, Y_BLUE_SCALE, Y_GREEN_SCALE, Y_RED_SCALE);
	__m128i ShiftScalingAdjust = _mm_set1_epi32(1 << (SCALING_LOG - 1));
	__m128i *SourceImagePtr = (__m128i *)SourceImage;
	__m128i ZeroConst = _mm_setzero_si128();
	int *YImagePtr = (int *)YImage;

    for (int i = (ImageWidth * ImageHeight); i > 0; i -= 4)  // 4 pixels per loop
	{
		__m128i RGBValue = _mm_loadu_si128(SourceImagePtr);
		SourceImagePtr++;
		__m128i LowRGBValue = _mm_unpacklo_epi8(RGBValue, ZeroConst);
		__m128i HighRGBValue = _mm_unpackhi_epi8(RGBValue, ZeroConst);
		// int YValue = (SourceImagePtr[0] * Y_RED_SCALE  ) +
		//			 (SourceImagePtr[1] * Y_GREEN_SCALE) +
		//			 (SourceImagePtr[2] * Y_BLUE_SCALE );
		__m128i LowYValue = _mm_madd_epi16(LowRGBValue, RGBScale);
		__m128i HighYValue = _mm_madd_epi16(HighRGBValue, RGBScale);
		__m128i YValue = _mm_hadd_epi32(LowYValue, HighYValue);
		// YValue += 1 << (SCALING_LOG - 1);
		YValue = _mm_add_epi32(YValue, ShiftScalingAdjust);
		// YValue >>= SCALING_LOG;
		YValue = _mm_srli_epi32(YValue, SCALING_LOG);
		YValue =  _mm_packs_epi32(YValue, YValue);
		// if (YValue > 255)
		//	YValue = 255;
		YValue =  _mm_packus_epi16(YValue, YValue);
		// *YImagePtr = (unsigned char)YValue;
		*YImagePtr = _mm_cvtsi128_si32(YValue);
		YImagePtr++;
	}
}

// ProcessRGBSIMD3
// reduced precision SSSE3 version of Serial code that does NOT produce results that match serial code's ones.
// both input and output image are unidimensional arrays of ImageWidth * ImageHeight pixels
// assumes that PixelOffset is 4 (RGBA image) and that ImageWidth * ImageHeight * PixelOffset is a multiple of 16
// does not assume that the input image is aligned on 16 bytes

void ProcessRGBSIMD3(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset)
{
	const int SCALING_LOG = 7;
	const int SCALING_FACTOR = (1 << SCALING_LOG);
	const int Y_RED_SCALE = (int)(0.299 * SCALING_FACTOR);
	const int Y_GREEN_SCALE = (int)(0.587 * SCALING_FACTOR);
	const int Y_BLUE_SCALE = (int)(0.114 * SCALING_FACTOR);

	__m128i RGBScale = _mm_set_epi8(0, Y_BLUE_SCALE, Y_GREEN_SCALE, Y_RED_SCALE, 0, Y_BLUE_SCALE, Y_GREEN_SCALE, Y_RED_SCALE, 0, Y_BLUE_SCALE, Y_GREEN_SCALE, Y_RED_SCALE, 0, Y_BLUE_SCALE, Y_GREEN_SCALE, Y_RED_SCALE);
	__m128i ShiftScalingAdjust = _mm_set1_epi16(1 << (SCALING_LOG - 1));
	__m128i *SourceImagePtr = (__m128i *)SourceImage;
	__m128i ZeroConst = _mm_setzero_si128();
	int *YImagePtr = (int *)YImage;

    for (int i = (ImageWidth * ImageHeight); i > 0; i -= 4)  // 4 pixels per loop
	{
		__m128i RGBValue = _mm_loadu_si128(SourceImagePtr);
		SourceImagePtr++;
		__m128i MultYValue = _mm_maddubs_epi16(RGBValue, RGBScale);
		// int YValue = (SourceImagePtr[0] * Y_RED_SCALE  ) +
		//			 (SourceImagePtr[1] * Y_GREEN_SCALE) +
		//			 (SourceImagePtr[2] * Y_BLUE_SCALE );
		__m128i YValue = _mm_hadd_epi16(MultYValue, MultYValue);
		YValue = _mm_add_epi16(YValue, ShiftScalingAdjust);
		YValue = _mm_srli_epi16(YValue, SCALING_LOG);
		// if (YValue > 255)
		//	YValue = 255;
		YValue =  _mm_packus_epi16(YValue, YValue);
		*YImagePtr = _mm_cvtsi128_si32(YValue);
		YImagePtr++;
	}
}

// ProcessRGBTBBSIMD
// by using a lambda that copies the required information automatically [=], the kernel of the computation is contained
// inside the invocation to parallel_for

void ProcessRGBTBBSIMD(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset)
{
	__m128i RGBScale = _mm_set_epi16(0, Y_BLUE_SCALE, Y_GREEN_SCALE, Y_RED_SCALE, 0, Y_BLUE_SCALE, Y_GREEN_SCALE, Y_RED_SCALE);
	__m128i ShiftScalingAdjust = _mm_set1_epi32(1 << (SCALING_LOG - 1));
	__m128i *SourceImagePtr = (__m128i *)SourceImage;
	__m128i ZeroConst = _mm_setzero_si128();
	int *YImagePtr = (int *)YImage;
	
	parallel_for( blocked_range<int>( 0, (ImageWidth * ImageHeight)),
	    [=](const blocked_range<int>& r) {
			unsigned char *LocalSourceImagePtr = (unsigned char *)SourceImage;
			unsigned char *LocalYImagePtr = (unsigned char *)YImage;
			LocalSourceImagePtr += r.begin() * PixelOffset;
			LocalYImagePtr += r.begin();
			__m128i *SourceImagePtr = (__m128i *)LocalSourceImagePtr;
			int *YImagePtr = (int *)LocalYImagePtr;

			for (int i = r.begin(); i < r.end(); i+=4)
			{
				__m128i RGBValue = _mm_loadu_si128(SourceImagePtr);
				SourceImagePtr++;
				__m128i LowRGBValue = _mm_unpacklo_epi8(RGBValue, ZeroConst);
				__m128i HighRGBValue = _mm_unpackhi_epi8(RGBValue, ZeroConst);
				// int YValue = (SourceImagePtr[0] * Y_RED_SCALE  ) +
				//			 (SourceImagePtr[1] * Y_GREEN_SCALE) +
				//			 (SourceImagePtr[2] * Y_BLUE_SCALE );
				__m128i LowYValue = _mm_madd_epi16(LowRGBValue, RGBScale);
				__m128i HighYValue = _mm_madd_epi16(HighRGBValue, RGBScale);
				__m128i YValue = _mm_hadd_epi32(LowYValue, HighYValue);
				YValue = _mm_add_epi32(YValue, ShiftScalingAdjust);
				YValue = _mm_srli_epi32(YValue, SCALING_LOG);
				YValue =  _mm_packs_epi32(YValue, ZeroConst);
				// if (YValue > 255)
				//	YValue = 255;
				YValue =  _mm_packus_epi16(YValue, YValue);
				*YImagePtr = _mm_cvtsi128_si32(YValue);
				YImagePtr++;
			}
	      }
	    );
}