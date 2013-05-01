// Intel TBB Demo
// by Stefano Tommesani (www.tommesani.com) 2013
// this code is release under the Code Project Open License (CPOL) http://www.codeproject.com/info/cpol10.aspx
// The main points subject to the terms of the License are:
// -   Source Code and Executable Files can be used in commercial applications;
// -   Source Code and Executable Files can be redistributed; and
// -   Source Code can be modified to create derivative works.
// -   No claim of suitability, guarantee, or any warranty whatsoever is provided. The software is provided "as-is".
// -   The Article(s) accompanying the Work may not be distributed or republished without the Author's consent

void ProcessRGBSerial(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset);
void ProcessRGBTBB1(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset);
void ProcessRGBTBB2(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset);
void ProcessRGBTBB3(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset);
void ProcessRGBSIMD(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset);
void ProcessRGBSIMD2(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset);
void ProcessRGBSIMD3(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset);
void ProcessRGBTBBSIMD(void *SourceImage, void *YImage, int ImageWidth, int ImageHeight, int PixelOffset);
