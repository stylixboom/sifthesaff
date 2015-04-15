/*
 * ins_offline.h
 *
 *  Created on: January 25, 2013
 *      Author: Siriwat Kasamwattanarote
 */
#pragma once

#include "version_lib.h"

#include "opencv2/core/core.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#define D 128
#define HEADSIZE 5

using namespace std;
using namespace cv;

class SIFThesaff
{
    bool check_file_exist;

    int colorspace;
    bool normalize;
	bool RootSIFT;
	int g_numberOfPoints;
	int g_numberOfAffinePoints;

	// Color space
    const static int RGB_SPACE = 10;
    const static int IRGB_SPACE = 11;
    const static int LAB_SPACE = 12;

    // Memory management flag
    bool has_kp;
    bool has_desc;

public:
	SIFThesaff(int Colorspace = RGB_SPACE, bool isNormalize = false, bool isRootSIFT = true, bool isCheckFile = true); // colorspace = 0 is rgb
	~SIFThesaff(void);
	void init(int Colorspace = RGB_SPACE, bool isNormalize = false, bool isRootSIFT = true, bool isCheckFile = true);
	static string version() { return siftlib_AutoVersion::siftlib_FULLVERSION_STRING; }; // Version
	static int GetSIFTD() { return D; };
	static int GetSIFTHeadSize() { return HEADSIZE; };

	vector<float*> kp; // x y a b c
	vector<float*> desc; // x-descriptors
	int num_kp;
	int width, height;
	struct HessianAffineParams;
	void exportKeypoints(const string& out, bool isBinary = true);
	bool importKeypoints(const string& in, bool isBinary = true);
	int checkNumKp(const string& in, bool isBinary = true);
	int extractPerdochSIFT(const string& imgPath);
	int extractPerdochSIFT(const Mat& img);
	void unlink_kp();
	void unlink_desc();
	void Reset(void);
	void rgb2lab(const uchar R, const uchar G, const uchar B, uchar& Lv, uchar& av, uchar& bv);
};
