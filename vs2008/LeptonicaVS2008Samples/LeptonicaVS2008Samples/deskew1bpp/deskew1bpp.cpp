/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -  This software is distributed in the hope that it will be
 -  useful, but with NO WARRANTY OF ANY KIND.
 -  No author or distributor accepts responsibility to anyone for the
 -  consequences of using this software, or for whether it serves any
 -  particular purpose or works at all, unless he or she says so in
 -  writing.  Everyone is granted permission to copy, modify and
 -  redistribute this source code, for commercial or non-commercial
 -  purposes, with the following restrictions: (1) the origin of this
 -  source code must not be misrepresented; (2) modified versions must
 -  be plainly marked as such; and (3) this notice may not be removed
 -  or altered from any source or modified source distribution.
 *====================================================================*/

/*
 * deskew1bpp.cpp
 * 
 * Directly deskewing 1 bpp images using any rotation technique
 * gives artifacts (sudden 1 bit horizontal shifts in the middle
 * of lines). This program compares direct rotation vs:
 *
 *   converting to gray,
 *   blurring using block convolution,
 *   rotating using area map,
 *   and then finally thresholding back to b&w
 *    (higher thresholds lead to fatter characters).
 * 
 * based on
 *
 * skewtest.c
 *
 *     Tests various skew finding methods, optionally deskewing
 *     the input (binary) image.  The best version does a linear
 *     sweep followed by a binary (angle-splitting) search.
 *     The basic method is to find the vertical shear angle such
 *     that the differential variance of ON pixels between each
 *     line and it's neighbor, when summed over all lines, is
 *     maximized.
 */

#include "stdafx.h"

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

bool binImageDiff(const char *oFilenameP, const char *nFilenameP, const char *diffFilenameP, char *tagP);

// deskew1bpp.cpp : Defines the entry point for the console application.
//

int _tmain(int argc, _TCHAR* argv[])
{
	l_float64    deg2rad;
	l_float32    angle, conf;
	PIX         *pixs, *pixd, *pixRot;
	char		 msgBuffer[1000];
	l_float32	 radians;
	l_int32		 w, h, d;
	char		*basenameP, *extensionP;
	L_KERNEL    *kel1;

	static char  mainName[] = "deskew1bpp";
	static const l_int32  THRESHOLD = 130;

	if (argc != 3)
		return ERROR_INT(" Syntax:  deskew1bpp filein fileout", mainName, 1);

	CT2A filein (argv[1]);
	CT2A fileout (argv[2]);

	splitPathAtExtension(filein, &basenameP, &extensionP);
	std::string baseFilename = std::string(basenameP);

	deg2rad = 3.1415926535 / 180.;

	if ((pixs = pixRead(filein)) == NULL)
		return ERROR_INT("pixs not made", mainName, 1);

	if (pixFindSkewSweepAndSearch(pixs, &angle, &conf, SWEEP_REDUCTION2,
		SEARCH_REDUCTION, SWEEP_RANGE2, SWEEP_DELTA2,
		SEARCH_MIN_DELTA)) {
			L_WARNING("skew angle not valid", mainName);
			return 1;
	}

	if (L_ABS(angle) < 0.1)
	{
		sprintf(msgBuffer, "Skew angle only %f5.3, image not deskewed", angle);
		L_WARNING(msgBuffer, mainName);
		return 1;
	}
	if (conf < 3.0)
	{
		sprintf(msgBuffer, "Skew angle confidence only %f3.1, image not deskewed", conf);
		L_WARNING(msgBuffer, mainName);
		return 1;
	}
	fprintf(stderr, "Skew Angle (confidence): %f5.3 (%f3.1)", angle, conf);

	radians = deg2rad * angle;
	pixGetDimensions (pixs, &w, &h, &d);

	pixd = pixRotate (pixs, radians, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, w, h);
	std::string originalFilenameStr = baseFilename + "-leptonica-am-bw.tif";
	pixWrite(originalFilenameStr.c_str(), pixd, IFF_TIFF_G4);
	pixDestroy(&pixd);

	pixd = pixConvertTo8(pixs, FALSE);
	pixDestroy(&pixs);
	if (pixd == NULL)
	{
		L_WARNING("Couldn't convert to grayscale", mainName);
		return 1;
	}
	std::string outFilenameStr = baseFilename + "-leptonica-gray.tif";
	pixWrite(outFilenameStr.c_str(), pixd, IFF_TIFF_LZW);

#ifdef commentout
	pixs = pixScaleGray2xLI(pixd);
	pixDestroy(&pixd);

	outFilenameStr = baseFilename + "-leptonica-gray2x.tif";
	pixWrite(outFilenameStr.c_str(), pixs, IFF_TIFF_LZW);
#endif

#ifdef commentout
	kel1 = makeGaussianKernel(1, 1, 0.5, 1.0);
	pixs = pixConvolve(pixd, kel1, 8, 1);
	kernelDestroy(&kel1);
	outFilenameStr = baseFilename + "-leptonica-gauss-0.50.tif";
	pixWrite(outFilenameStr.c_str(), pixs, IFF_TIFF_LZW);
	pixDestroy(&pixs);

	kel1 = makeGaussianKernel(1, 1, 0.75, 1.0);
	pixs = pixConvolve(pixd, kel1, 8, 1);
	kernelDestroy(&kel1);
	outFilenameStr = baseFilename + "-leptonica-gauss-0.75.tif";
	pixWrite(outFilenameStr.c_str(), pixs, IFF_TIFF_LZW);
	pixDestroy(&pixs);

	kel1 = makeGaussianKernel(1, 1, 1.00, 1.0);
	pixs = pixConvolve(pixd, kel1, 8, 1);
	kernelDestroy(&kel1);
	outFilenameStr = baseFilename + "-leptonica-gauss-1.00.tif";
	pixWrite(outFilenameStr.c_str(), pixs, IFF_TIFF_LZW);
	pixDestroy(&pixs);

	kel1 = makeGaussianKernel(1, 1, 1.25, 1.0);
	pixs = pixConvolve(pixd, kel1, 8, 1);
	kernelDestroy(&kel1);
	outFilenameStr = baseFilename + "-leptonica-gauss-1.25.tif";
	pixWrite(outFilenameStr.c_str(), pixs, IFF_TIFF_LZW);
	pixDestroy(&pixs);

	kel1 = makeGaussianKernel(1, 1, 1.50, 1.0);
	pixs = pixConvolve(pixd, kel1, 8, 1);
	kernelDestroy(&kel1);
	outFilenameStr = baseFilename + "-leptonica-gauss-1.50.tif";
	pixWrite(outFilenameStr.c_str(), pixs, IFF_TIFF_LZW);
	pixDestroy(&pixd);
#endif

	pixs = pixBlockconv(pixd, 1, 1);
	pixDestroy(&pixd);
	outFilenameStr = baseFilename + "-leptonica-blockconv11.tif";
	pixWrite(outFilenameStr.c_str(), pixs, IFF_TIFF_LZW);

	// use area map rotation
	pixd = pixRotate (pixs, radians, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, w, h);
	pixDestroy(&pixs);
	if (pixd == NULL)
	{
		L_WARNING("Couldn't rotate gray by area map", mainName);
		return 1;
	}
	outFilenameStr = baseFilename + "-leptonica-am.tif";
	pixWrite(outFilenameStr.c_str(), pixd, IFF_TIFF_LZW);

#ifdef commentout
	pixRot = pixScaleAreaMap2(pixd);
	pixDestroy(&pixd);

	outFilenameStr = baseFilename + "-leptonica-am-reduce2.tif";
	pixWrite(outFilenameStr.c_str(), pixRot, IFF_TIFF_LZW);
#endif

	pixRot = pixCopy(NULL, pixd);
	pixDestroy(&pixd);

	pixd = pixConvertTo1(pixRot, THRESHOLD);
	outFilenameStr = baseFilename + "-leptonica-am-bw130.tif";
	pixWrite(outFilenameStr.c_str(), pixd, IFF_TIFF_G4);
	pixDestroy(&pixd);

	std::string diffFilenameStr = baseFilename + "-leptonica-am-bw130-diff.tif";
	binImageDiff(originalFilenameStr.c_str(), outFilenameStr.c_str(), diffFilenameStr.c_str(), "Threshold 130 ");

	pixd = pixConvertTo1(pixRot, 150);
	outFilenameStr = baseFilename + "-leptonica-am-bw150.tif";
	pixWrite(outFilenameStr.c_str(), pixd, IFF_TIFF_G4);
	pixDestroy(&pixd);

	diffFilenameStr = baseFilename + "-leptonica-am-bw150-diff.tif";
	binImageDiff(originalFilenameStr.c_str(), outFilenameStr.c_str(), diffFilenameStr.c_str(), "Threshold 150 ");

	pixd = pixConvertTo1(pixRot, 170);
	outFilenameStr = baseFilename + "-leptonica-am-bw170.tif";
	pixWrite(outFilenameStr.c_str(), pixd, IFF_TIFF_G4);
	pixDestroy(&pixd);

	diffFilenameStr = baseFilename + "-leptonica-am-bw170-diff.tif";
	binImageDiff(originalFilenameStr.c_str(), outFilenameStr.c_str(), diffFilenameStr.c_str(), "Threshold 170 ");

	pixd = pixConvertTo1(pixRot, 200);
	outFilenameStr = baseFilename + "-leptonica-am-bw200.tif";
	pixWrite(outFilenameStr.c_str(), pixd, IFF_TIFF_G4);
	pixDestroy(&pixd);

	diffFilenameStr = baseFilename + "-leptonica-am-bw200-diff.tif";
	binImageDiff(originalFilenameStr.c_str(), outFilenameStr.c_str(), diffFilenameStr.c_str(), "Threshold 200 ");

	pixDestroy(&pixRot);
	return 0;
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
