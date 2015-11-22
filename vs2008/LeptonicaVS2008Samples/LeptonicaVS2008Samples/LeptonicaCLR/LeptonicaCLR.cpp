// LeptonicaCLR.cpp : main project file.

#include "stdafx.h"

using namespace System;

int getBitDepth(const char *sFilenameP)
{
	PIX *pixs = pixRead(sFilenameP);
	if (!pixs)
		return -1;

	int depth = pixGetDepth(pixs);
	pixDestroy(&pixs);

	return depth;
}

/* deskew */
#define   DESKEW_REDUCTION      2      /* 1, 2 or 4 */

/* sweep only */
#define   SWEEP_RANGE           5.     /* degrees */
#define   SWEEP_DELTA           0.2    /* degrees */
#define   SWEEP_REDUCTION       2      /* 1, 2, 4 or 8 */

/* sweep and search */
#define   SWEEP_RANGE2          5.     /* degrees */
#define   SWEEP_DELTA2          1.     /* degrees */
#define   SWEEP_REDUCTION2      2      /* 1, 2, 4 or 8 */
#define   SEARCH_REDUCTION      2      /* 1, 2, 4 or 8 */
#define   SEARCH_MIN_DELTA      0.01   /* degrees */

bool deskewBinaryImage(const char *oFilenameP, const char *nFilenameP, int threshold)
{
	l_float32    angle, conf;
	PIX         *pixs, *pixd, *pixRot;
	char		 msgBuffer[1000];
	l_float32	 radians;
	l_int32		 w, h, d;
	l_float64    deg2rad = 3.1415926535 / 180.;
	static char  mainName[] = "deskewBinaryImage";

	if ((pixs = pixRead(oFilenameP)) == NULL)
		return false;
	if (pixGetDepth(pixs) != 1)
		return false;

	if (pixFindSkewSweepAndSearch(pixs, &angle, &conf, SWEEP_REDUCTION2,
		SEARCH_REDUCTION, SWEEP_RANGE2, SWEEP_DELTA2, SEARCH_MIN_DELTA))
	{
		L_WARNING("skew angle not valid", mainName);
		return false;
	}

	if (L_ABS(angle) < 0.1)
	{
		sprintf_s(msgBuffer, "Skew angle only %f5.3, image not deskewed", angle);
		L_WARNING(msgBuffer, mainName);
		return false;
	}
	if (conf < 3.0)
	{
		sprintf_s(msgBuffer, "Skew angle confidence only %f3.1, image not deskewed", conf);
		L_WARNING(msgBuffer, mainName);
		return false;
	}
	//fprintf(stderr, "Skew Angle (confidence): %f5.3 (%f3.1)", angle, conf);

	radians = deg2rad * angle;
	pixGetDimensions (pixs, &w, &h, &d);

	pixd = pixConvertTo8(pixs, FALSE);
	pixDestroy(&pixs);
	if (pixd == NULL)
	{
		L_WARNING("Couldn't convert to grayscale", mainName);
		return false;
	}

	pixs = pixBlockconv(pixd, 1, 1);
	pixDestroy(&pixd);

	// use area map rotation
	pixRot = pixRotate (pixs, radians, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, w, h);
	pixDestroy(&pixs);
	if (pixRot == NULL)
	{
		L_WARNING("Couldn't rotate gray by area map", mainName);
		return false;
	}

	pixd = pixConvertTo1(pixRot, threshold);
	pixDestroy(&pixRot);

	pixWrite(nFilenameP, pixd, IFF_TIFF_G4);
	pixDestroy(&pixd);

	return true;
}

int renderCC
	(
	PIX *pixs,			//original image
	PIX *pixmask,		//1bpp indicator mask to render onto pixs
	int margin,			//margin of box drawn around cc in pixmask
	l_uint8 rval,		//red value
	l_uint8 gval,		//green value
	l_uint8 bval,		//blue value
	l_float32 fract		//transparency
	)
{
	BOXA *boxa, *boxad;
	BOX  *boxs, *boxd;
	int n, i, j, lineWidth=4, halfWidth;
	l_int32 result;

	halfWidth = lineWidth/2;
	boxa = pixConnComp(pixmask, NULL, 8);
	n = boxaGetCount(boxa);
	if (n > 1000)
	{
		boxaDestroy(&boxa);
		return n;
	}

	if ((boxad = boxaCreate(n)) == NULL)
	{
		boxaDestroy(&boxa);
		return 0;
	}

	for (i = 0; i < n; i++) 
	{
		l_int32 x, y, w, h;

		if ((boxs = boxaGetBox(boxa, i, L_CLONE)) == NULL)
		{
			boxaDestroy(&boxa);
			boxaDestroy(&boxad);
			return 0;
		}

		boxGetGeometry(boxs, &x, &y, &w, &h);
		//fprintf(stderr, "box %d: %d, %d (%d x %d)\n", i, x, y, w, h);

		x -= margin;
		y -= margin;
		w += margin * 2;
		h += margin * 2;

		boxd = boxCreate(x, y, w, h);
		boxDestroy(&boxs);
		boxaAddBox(boxad, boxd, L_INSERT);

		// Draw single pixel markers?
		for (j=halfWidth-1; j<w-lineWidth-1; j += 2)
		{
			int xb = x + halfWidth + j;
			int yb = y + halfWidth;

			pixRenderLineBlend (pixs, xb, yb, xb, yb, 1, rval, gval, bval, fract);

			yb = y + h - lineWidth;
			//pixRenderLineBlend (pixs, xb, yb, xb, yb, 1, rval, gval, bval, fract);
		}

		for (j=halfWidth-1; j<h-lineWidth-1; j += 2)
		{			
			int xb = x + halfWidth;
			int yb = y + halfWidth + j;
			pixRenderLineBlend (pixs, xb, yb, xb, yb, 1, rval, gval, bval, fract);

			xb = x + w - lineWidth;
			//pixRenderLineBlend (pixs, xb, yb, xb, yb, 1, rval, gval, bval, fract);
		}
	}

	//composeRGBPixel(rval, gval, bval, &val);
	//pixd = pixDrawBoxa(pixs, boxad, 2, val);

	result = pixRenderBoxaBlend (pixs, boxad, lineWidth, rval, gval, bval, fract, 1);

	boxaDestroy(&boxa);
	boxaDestroy(&boxad);

	return n;
}

bool binImageDiff(const char *oFilenameP, const char *nFilenameP, const char *diffFilenameP, char *tagP)
{
	PIX			*pixOff, *pixOn, *pixDiff, *pixColor;
	l_float32	 fractOff, fractOn;
	l_int32      nOff, nOn;
	int			 nOffBlots, nOnBlots;
	char		 descriptionBuf[2048];
	NUMA        *naflags;
	SARRAY      *savals, *satypes;

	PIX *pixo = pixRead(oFilenameP);
	if (pixo == NULL)
		return false;

	PIX *pixn = pixRead(nFilenameP);
	if (pixn == NULL)
	{
		pixDestroy(&pixo);
		return false;
	}

	pixCompareBinary(pixo, pixn, L_COMPARE_SUBTRACT, &fractOff, &pixOff);
	pixCountPixels(pixOff, &nOff, NULL);

	pixCompareBinary(pixn, pixo, L_COMPARE_SUBTRACT, &fractOn, &pixOn);
	pixCountPixels(pixOn, &nOn, NULL);

	// show changed pixel as colors on original image
	pixDiff = pixConvert1To2Cmap(pixo);
	pixSetMaskedCmap(pixDiff, pixOff, 0, 0, 0, 192, 0);		//removed
	pixSetMaskedCmap(pixDiff, pixOn, 0, 0, 255, 0, 0);		//added

	pixColor = pixConvertTo32(pixDiff);
	pixDestroy(&pixDiff);
	nOffBlots = renderCC(pixColor, pixOff, 15, 0, 192, 0, (l_float32) 0.75);
	nOnBlots = renderCC(pixColor, pixOn, 15, 255, 0, 0, (l_float32) 0.75);

	sprintf_s(descriptionBuf,
		"%s(%d, %d) changed marks. (erased=green, added=red)", tagP, nOffBlots, nOnBlots);
	naflags = numaCreate(1);
	savals = sarrayCreate(1);
	satypes = sarrayCreate(1);
	numaAddNumber(naflags, 270);  /* IMAGEDESCRIPTION */
	sarrayAddString(savals, descriptionBuf, 1);
	sarrayAddString(satypes, "char*", 1);
	pixWriteTiffCustom (diffFilenameP, pixColor, IFF_TIFF_LZW, "w", naflags, savals, satypes, NULL);

	sarrayRemoveString(savals, 0);
	sarrayAddString(savals, "New image", 1);
	pixWriteTiffCustom (diffFilenameP, pixn, IFF_TIFF_G4, "a", naflags, savals, satypes, NULL);

	sarrayRemoveString(savals, 0);
	sarrayAddString(savals, "Original image", 1);
	pixWriteTiffCustom (diffFilenameP, pixo, IFF_TIFF_G4, "a", naflags, savals, satypes, NULL);

	sprintf_s(descriptionBuf, "Marks added: %d, Pixels %d (%10.6f).", nOnBlots, nOn, fractOn);
	sarrayRemoveString(savals, 0);
	sarrayAddString(savals, descriptionBuf, 1);
	pixWriteTiffCustom (diffFilenameP, pixOn, IFF_TIFF_G4, "a", naflags, savals, satypes, NULL);

	sprintf_s(descriptionBuf, "Marks erased: %d, Pixels %d (%10.6f).", nOffBlots, nOff, fractOff);
	sarrayRemoveString(savals, 0);
	sarrayAddString(savals, descriptionBuf, 1);
	pixWriteTiffCustom (diffFilenameP, pixOff, IFF_TIFF_G4, "a", naflags, savals, satypes, NULL);

	pixDestroy(&pixo);
	pixDestroy(&pixn);
	pixDestroy(&pixOff);
	pixDestroy(&pixOn);
	pixDestroy(&pixColor);

	numaDestroy(&naflags);
	sarrayDestroy(&savals);
	sarrayDestroy(&satypes);

	return true;
}

namespace LeptonicaCLR
{
	public ref class Utils
	{
	public:
		static int GetBitDepth(String^ oFilename)
		{
			IntPtr ipOFilename = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(oFilename);
			char *oFilenameP = static_cast<char *>(ipOFilename.ToPointer());

			int result = getBitDepth (oFilenameP);

			System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr(ipOFilename));

			return result;
		}
		
		static bool DeskewBinaryImage (String^ oFilename, String^ nFilename, int threshold)
		{
			IntPtr ipOFilename = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(oFilename);
			char *oFilenameP = static_cast<char *>(ipOFilename.ToPointer());
			IntPtr ipNFilename = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(nFilename);
			char *nFilenameP = static_cast<char *>(ipNFilename.ToPointer());

			bool result = deskewBinaryImage (oFilenameP, nFilenameP, threshold);

			System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr(ipOFilename));
			System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr(ipNFilename));

			return result;
		}

		static bool CreateBinaryImageDiff (String^ oFilename, String^ nFilename, String^ diffFilename,
			String^ tag)
		{
			IntPtr ipOFilename = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(oFilename);
			char *oFilenameP = static_cast<char *>(ipOFilename.ToPointer());
			IntPtr ipNFilename = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(nFilename);
			char *nFilenameP = static_cast<char *>(ipNFilename.ToPointer());
			IntPtr ipDiffFilename = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(diffFilename);
			char *diffFilenameP = static_cast<char *>(ipDiffFilename.ToPointer());
			IntPtr ipTag = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(tag);
			char *tagP = static_cast<char *>(ipTag.ToPointer());

			bool result = binImageDiff (oFilenameP, nFilenameP, diffFilenameP, tagP);

			System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr(ipOFilename));
			System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr(ipNFilename));
			System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr(ipDiffFilename));
			System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr(ipTag));

			return result;
		}
	};
}
