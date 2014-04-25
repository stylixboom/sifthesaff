/*
 * ins_offline.h
 *
 *  Created on: January 25, 2013
 *      Author: Siriwat Kasamwattanarote
 */
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <sys/stat.h>   // file-directory existing
#include <sys/types.h>  // file-directory
#include <dirent.h>     // file-directory

#include "AffineHessianDetector.cpp"
#include "pyramid.h"
#include "helpers.h"
#include "affine.h"
#include "siftdesc.h"

#include "opencv2/core/core.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "../alphautils/alphautils.h"

#include "version_lib.h"

#define D 128
#define HEADSIZE 5

using namespace std;
using namespace cv;
using namespace alphautils;

class SIFThesaff
{
    int colorspace;
    bool normalize;
	bool RootSIFT;
	int g_numberOfPoints;
	int g_numberOfAffinePoints;

	// Color space
    const static int RGB_SPACE = 0;
    const static int IRGB_SPACE = 1;
    const static int LAB_SPACE = 2;

public:
	SIFThesaff(int Colorspace = RGB_SPACE, bool isNormalize = false, bool isRootSIFT = true); // colorspace = 0 is rgb
	~SIFThesaff(void);
	static string version() { return siftlib_AutoVersion::siftlib_FULLVERSION_STRING; }; // Version
	vector< vector<float> > kp; // x y a b c
	vector< vector<float> > desc; // x-descriptors
	int num_kp;
	int width, height;
	struct HessianAffineParams;
	void exportKeypoints(const string& out, bool isBinary = true);
	void importKeypoints(const string& in, bool isBinary = true);
	int checkNumKp(const string& in, bool isBinary = true);
	void extractPerdochSIFT(const string& imgPath);
	void extractPerdochSIFT(const Mat& img);
	void Reset(void);
	int GetSIFTD(void);
	int GetSIFTHeadSize(void);
	void rgb2lab(const uchar R, const uchar G, const uchar B, uchar& Lv, uchar& av, uchar& bv);
};
