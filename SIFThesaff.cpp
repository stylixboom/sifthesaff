/*
 * ins_offline.h
 *
 *  Created on: January 25, 2013
 *      Author: Siriwat Kasamwattanarote
 */
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <sys/stat.h>   // file-directory existing
#include <sys/types.h>  // file-directory
#include <dirent.h>     // file-directory

#include "opencv2/core/core.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"

// Siriwat's header
#include "../alphautils/alphautils.h"

#include "AffineHessianDetector.cpp"
#include "pyramid.h"
#include "helpers.h"
#include "affine.h"
#include "siftdesc.h"

#include "SIFThesaff.h"

#include "version_lib.h"

using namespace std;
using namespace cv;
using namespace alphautils;

SIFThesaff::SIFThesaff(int Colorspace, bool isNormalize, bool isRootSIFT, bool isCheckFile)
{
    init(Colorspace, isNormalize, isRootSIFT, isCheckFile);
}

SIFThesaff::~SIFThesaff(void)
{
	Reset();
}

struct SIFThesaff::HessianAffineParams
{
	float threshold;
	int   max_iter;
	float desc_factor;
	int   patch_size;
	bool  verbose;
	HessianAffineParams()
	{
	    //threshold = 16.0f/3.0f;           // original
		threshold = 16.0f/4.6875f;        // 0.04f * 256 / 3 from pyramid.h;  // Siriwat's baseline
		//threshold = 16.0f/6.0f;             // affect detector, higher t for lower number of feature, lower t for higher number of feature (high sensitive)
		max_iter = 16;                      // affect detector, higher it for higher number of feature, lower it for lower number of feature (low sensitive)
		desc_factor = 3.0f*sqrt(3.0f);      // affect descriptor
		patch_size = 41;                    // affect descriptor
		verbose = false;
	}
};

void SIFThesaff::init(int Colorspace, bool isNormalize, bool isRootSIFT, bool isCheckFile)
{
    colorspace = Colorspace;
    normalize = isNormalize;
	RootSIFT = isRootSIFT;
	check_file_exist = isCheckFile;
	g_numberOfPoints = 0;
	g_numberOfAffinePoints = 0;
	num_kp = 0;
	has_kp = false;
	has_desc = false;
}

void SIFThesaff::exportKeypoints(const string& out, bool isBinary)
{
    // Check directory
    string dir_name = get_directory(out);
    make_dir_available(dir_name);

    // Oxford format
	if(isBinary)
	{
		ofstream OutFile (out.c_str(), ios::binary);
		if (OutFile.is_open())
		{
		    /// Prepare buffer
		    size_t buffer_size = sizeof(int) * 4 +
                                 num_kp * (HEADSIZE * sizeof(float) + D * sizeof(float));
                                 // sizeof(width) + sizeof(height) + sizeof(len_sift) + sizeof(num_kp) +
                                 // num_kp * (HEADSIZE * sizeof(kp) + D * sizeof(desc))
            char* buffer = new char[buffer_size];
            char* buffer_ptr = buffer;

		    /// Put buffer
            // Put width
            *((int*)buffer_ptr) = width;
            buffer_ptr += sizeof(int);

            // Put height
            *((int*)buffer_ptr) = height;
            buffer_ptr += sizeof(int);

			// Put len_sift
            *((int*)buffer_ptr) = D;
            buffer_ptr += sizeof(int);

			// Put num_kp
            *((int*)buffer_ptr) = num_kp;
            buffer_ptr += sizeof(int);

			// Put sift "x y a b c" and "all dimensions"
			for(int kp_idx = 0; kp_idx < num_kp; kp_idx++)
			{
				// Put sift head x y a b c
                *((float*)buffer_ptr) = kp[kp_idx][0];
                buffer_ptr += sizeof(float);
                *((float*)buffer_ptr) = kp[kp_idx][1];
                buffer_ptr += sizeof(float);
                *((float*)buffer_ptr) = kp[kp_idx][2];
                buffer_ptr += sizeof(float);
                *((float*)buffer_ptr) = kp[kp_idx][3];
                buffer_ptr += sizeof(float);
                *((float*)buffer_ptr) = kp[kp_idx][4];
                buffer_ptr += sizeof(float);

				// Put sift data
				for (int desc_idx = 0; desc_idx < D; desc_idx++)
                {
                    *((float*)buffer_ptr) = desc[kp_idx][desc_idx];
                    buffer_ptr += sizeof(float);
                }
			}

			// Write file from buffer
			OutFile.write(buffer, buffer_size);

			// Close file
			OutFile.close();

			// Release memory
			delete[] buffer;
		}
	}
	else
	{
        ofstream OutFile (out.c_str());
        if (OutFile.is_open())
        {
            OutFile << width << endl;
            OutFile << height << endl;
            OutFile << D << endl;
            OutFile << num_kp << endl;

            for(int kp_idx = 0; kp_idx < num_kp; kp_idx++)
            {
                OutFile << kp[kp_idx][0] << " " << kp[kp_idx][1] << " " << kp[kp_idx][2] << " " << kp[kp_idx][3] << " " << kp[kp_idx][4] << " ";
                for(size_t desc_pos = 0; desc_pos < D; desc_pos++)
                    OutFile << desc[kp_idx][desc_pos] << " ";
                OutFile << endl;
            }

            OutFile.close();
        }
	}
}

bool SIFThesaff::importKeypoints(const string& in, bool isBinary)
{
    Reset();

    if(check_file_exist && !is_path_exist(in)) // no exist
    {
        cout << "File \"" << in << "\" not found" << endl;

        char opt;
        cout << "Wanna try to extract?\nPlease do it manually 'y' or skip 'n' [y|n]:"; cout.flush();
        cin >> opt;
        if (opt == 'n')
            return false;   // interpret as no sift keypoint
    }

    // Oxford format
	if(isBinary)
	{
	    // Prepare buffer
		ifstream InFile (in.c_str(), ios::binary);
		if (InFile)
		{
		    /// Create buffer
		    InFile.seekg(0, InFile.end);
            size_t buffer_size = InFile.tellg();
		    InFile.seekg(0, InFile.beg);
            char* buffer = new char[buffer_size];
            char* buffer_ptr = buffer;

		    // Read whole file into buffer
		    InFile.read(buffer, buffer_size);

			// Close file
			InFile.close();

			/// Interpret from buffer
			// Read width
            width = *((int*)buffer_ptr);
            buffer_ptr += sizeof(width);

            // Read height
            height = *((int*)buffer_ptr);
            buffer_ptr += sizeof(height);

			// Read len_sift
			// Same as D
            int len_sift;
            len_sift = *((int*)buffer_ptr);
            buffer_ptr += sizeof(len_sift);

			// Read num_sift
            num_kp = *((int*)buffer_ptr);
            buffer_ptr += sizeof(num_kp);

            size_t actual_filesize = sizeof(width) + sizeof(height) + sizeof(len_sift) + sizeof(num_kp) + (num_kp * (HEADSIZE * sizeof(float) + len_sift * sizeof(float)));
            if (actual_filesize != buffer_size)
            {
                cout << "SIFT file is corrupt: " << in << endl;

                char opt;
                cout << "Wanna try to extract?\nPlease do it manually 'y' or skip 'n' [y|n]:"; cout.flush();
                cin.clear();
                cin >> opt;
                if (opt == 'n')
                    exit(EXIT_FAILURE);
            }

			// Read sift "x y a b c" and "all dimensions"
			for(int kp_idx = 0; kp_idx < num_kp; kp_idx++)
			{
				// Read sift head "x y a b c"
                float* read_kp = new float[HEADSIZE];
				for(int head_pos = 0; head_pos < HEADSIZE; head_pos++)
                {
                    read_kp[head_pos] = *((float*)buffer_ptr);
                    buffer_ptr += sizeof(read_kp[head_pos]);
                }
                kp.push_back(read_kp);

				// Read sift data "128D"
                float* read_desc = new float[len_sift];
				for(int desc_pos = 0; desc_pos < len_sift; desc_pos++)
                {
                    read_desc[desc_pos] = *((float*)buffer_ptr);
                    buffer_ptr += sizeof(read_desc[desc_pos]);
                }
                desc.push_back(read_desc);
			}

			// Release buffer
			delete[] buffer;
		}
	}
	else
	{
        cout << "No implementation for reading sift text file" << endl;
        exit(EXIT_FAILURE);
        /*
        ofstream InFile (in.c_str());
        if (InFile)
        {
            InFile << D << endl;
            InFile << num_kp << endl;

            for(size_t kpIdx = 0; kpIdx < kp.size(); kpIdx++)
            {
                InFile << kp[kpIdx][0] << " " << kp[kpIdx][1] << " " << kp[kpIdx][2] << " " << kp[kpIdx][3] << " " << kp[kpIdx][4] << " ";
                for(size_t descIdx = 0; descIdx < desc[kpIdx].size(); descIdx++)
                    InFile << desc[kpIdx][descIdx] << " ";
                InFile << endl;
            }

            InFile.close();
        }
        */
	}

	// Flag memory allocated
    has_kp = true;
    has_desc = true;

    return true;
}

int SIFThesaff::checkNumKp(const string& in, bool isBinary)
{
    int ret_num_sift = 0;

    if(check_file_exist && !is_path_exist(in)) // no exist
    {
        cout << "File \"" << in << "\" not found" << endl;

        char opt;
        cout << "Wanna try to extract?\nPlease do it manually 'y' or skip 'n' [y|n]:"; cout.flush();
        cin >> opt;
        if (opt == 'n')
            return 0;   // interpret as no sift keypoint
    }

    // Oxford format
	if(isBinary)
	{
		ifstream InFile (in.c_str(), ios::binary);
		if (InFile)
		{
            int len_sift;
		    /// Create buffer
            size_t buffer_size = sizeof(width) + sizeof(height) + sizeof(len_sift) + sizeof(ret_num_sift);
            char* buffer = new char[buffer_size];
            char* buffer_ptr = buffer;

		    // Read whole file into buffer
		    InFile.read(buffer, buffer_size);

            InFile.seekg(0, InFile.end);
            size_t file_size = InFile.tellg();

			// Close file
			InFile.close();

			/// Interpret from buffer
			// Read width
            width = *((int*)buffer_ptr);
            buffer_ptr += sizeof(width);

            // Read height
            height = *((int*)buffer_ptr);
            buffer_ptr += sizeof(height);

			// Read len_sift
            len_sift = *((int*)buffer_ptr);
            buffer_ptr += sizeof(len_sift);

			// Read ret_num_sift
            ret_num_sift = *((int*)buffer_ptr);
            buffer_ptr += sizeof(ret_num_sift);

            size_t actual_filesize = sizeof(width) + sizeof(height) + sizeof(len_sift) + sizeof(ret_num_sift) + (ret_num_sift * (HEADSIZE * sizeof(float) + len_sift * sizeof(float)));
            if (actual_filesize != file_size)
            {
                cout << "SIFT corrupt: " << actual_filesize << " - " << file_size << " " << in << endl;
            }
            else
            // Stop reading the rest

			// Release buffer
			delete[] buffer;
		}
	}
	else
	{
        cout << "No implementation for reading sift text file" << endl;
        exit(EXIT_FAILURE);
        /*
        ofstream InFile (in.c_str());
        if (InFile)
        {
            InFile << D << endl;
            InFile << num_kp << endl;

            for(size_t kpIdx = 0; kpIdx < kp.size(); kpIdx++)
            {
                InFile << kp[kpIdx][0] << " " << kp[kpIdx][1] << " " << kp[kpIdx][2] << " " << kp[kpIdx][3] << " " << kp[kpIdx][4] << " ";
                for(size_t descIdx = 0; descIdx < desc[kpIdx].size(); descIdx++)
                    InFile << desc[kpIdx][descIdx] << " ";
                InFile << endl;
            }

            InFile.close();
        }
        */
	}

    // Keep num_kp
    num_kp = ret_num_sift;
	return ret_num_sift; // num_kp
}

int SIFThesaff::extractPerdochSIFT(const string& imgPath)
{
	Mat tmp = imread(imgPath);
	extractPerdochSIFT(tmp);
	tmp.release();

	return num_kp;
}

int SIFThesaff::extractPerdochSIFT(const Mat& tmp)
{
	Reset();

	//Mat tmp = imread(image_filename);
	Mat image(tmp.rows, tmp.cols, CV_32FC1, Scalar(0)); // float 1 channel

	float *out = image.ptr<float>(0);

    // Make gray scale by (b+g+r)/3
    if (colorspace == RGB_SPACE)
    {
        const uchar *in = tmp.ptr<uchar>(0);
        for (size_t i = tmp.rows * tmp.cols; i > 0; i--)
        {
            *out = (float(in[0]) + in[1] + in[2]) / 3.0f;
            out++;
            in += 3;
        }
    }
    // Make gray scale by (0.2989 * R) + (0.5870 * G) + (0.1140 * B)
    // http://docs.opencv.org/modules/imgproc/doc/miscellaneous_transformations.html
    else if (colorspace == IRGB_SPACE)
    {
        //Mat tmp_gray(tmp.rows, tmp.cols, CV_8UC1, Scalar(0));
        //cvtColor(tmp, tmp_gray, CV_BGR2GRAY);
        const uchar *in = tmp.ptr<uchar>(0);
        //const uchar *in = tmp_gray.ptr<uchar>(0);
        for (size_t i = tmp.rows * tmp.cols; i > 0; i--)
        {
            *out = 0.2989f * in[2] + 0.5870f * in[1] + 0.1140f * in[0];
            //*out = float(in[0]);
            out++;
            in += 3;
            //in++;
        }
    }
    // Make gray from L channel from OpenCV Lab
    else
    {
        //Convert BGR to LAB
        Mat tmp_lab(tmp.rows, tmp.cols, CV_8UC3, Scalar(0, 0, 0));
        cvtColor(tmp, tmp_lab, CV_BGR2Lab);
        const uchar *in = tmp_lab.ptr<uchar>(0);
        for (size_t i = tmp.rows * tmp.cols; i > 0; i--)
        {
            *out = float(in[0]);
            //uchar Lv, av, bv;
            //rgb2lab(in[0], in[1], in[2], Lv, av, bv);
            //*out = float(Lv);
            out++;
            in += 3;
        }
    }

	HessianAffineParams par;
	//double t1 = 0;
	// copy params
	PyramidParams p;
	p.threshold = par.threshold;

	AffineShapeParams ap;
	ap.maxIterations = par.max_iter;
	ap.patchSize = par.patch_size;
	ap.mrSize = par.desc_factor;

	SIFTDescriptorParams sp;
	sp.patchSize = par.patch_size;

	AffineHessianDetector detector(image, p, ap, sp, RootSIFT);
	//t1 = getTime();
	detector.detectPyramidKeypoints(image);
	g_numberOfPoints = detector.g_numberOfPoints;
	g_numberOfAffinePoints = detector.g_numberOfAffinePoints;

	//cout << "Detected " << g_numberOfPoints << " keypoints and " << g_numberOfAffinePoints << " affine shapes in " << getTime()-t1 << " sec." << endl;

    // Get width and height
    width = tmp.cols;
    height = tmp.rows;

    // Got result here, in kp and desc
	num_kp = detector.extractKeypoints(kp, desc);

	// Point normalization to 0-1 scale
	if (normalize)
	{
        for (int kp_idx = 0; kp_idx < num_kp; kp_idx++)
        {
            kp[kp_idx][0] /= width;
            kp[kp_idx][1] /= height;
        }
    }

    // Flag memory allocated
    has_kp = true;
    has_desc = true;

    return num_kp;
}

void SIFThesaff::unlink_kp()
{
    has_kp = false;
}

void SIFThesaff::unlink_desc()
{
    has_desc = false;
}

void SIFThesaff::Reset(void)
{
	// Release Mem
	if(num_kp)
	{
	    size_t actual_kp = kp.size();
		for(size_t kp_idx = 0; kp_idx < actual_kp; kp_idx++)
		{
			if (has_kp) delete[] kp[kp_idx];
			if (has_desc) delete[] desc[kp_idx];
		}
		vector<float*>().swap(kp);
		vector<float*>().swap(desc);
		has_kp = has_desc = false;

		g_numberOfPoints = 0;
		g_numberOfAffinePoints = 0;
		num_kp = 0;
	}
}

void SIFThesaff::rgb2lab(const uchar R, const uchar G, const uchar B, uchar &Lv, uchar &av, uchar &bv)
{
    // Ref: http://www.f4.fhtw-berlin.de/~barthel/ImageJ/ColorInspector/HTMLHelp/farbraumJava.htm
    //http://www.brucelindbloom.com

    float r, g, b, X, Y, Z, fx, fy, fz, xr, yr, zr;
    float Ls, as, bs;
    float eps = 216.0f / 24389.0f;
    float k = 24389.0f / 27.0f;

    float Xr = 0.964221f;  // reference white D50
    float Yr = 1.0f;
    float Zr = 0.825211f;

    // RGB to XYZ
    r = R / 255.0f; //R 0..1
    g = G / 255.0f; //G 0..1
    b = B / 255.0f; //B 0..1

    // assuming sRGB (D65)
    if (r <= 0.04045f)
        r = r / 12;
    else
        r = (float) pow((r + 0.055f) / 1.055f, 2.4f);

    if (g <= 0.04045f)
        g = g / 12;
    else
        g = (float) pow((g + 0.055f) / 1.055f, 2.4f);

    if (b <= 0.04045f)
        b = b / 12;
    else
        b = (float) pow((b + 0.055f) / 1.055f, 2.4f);


    X = 0.436052025f * r + 0.385081593f * g + 0.143087414f * b;
    Y = 0.222491598f * r + 0.71688606f * g + 0.060621486f * b;
    Z = 0.013929122f * r + 0.097097002f * g + 0.71418547f * b;

    // XYZ to Lab
    xr = X / Xr;
    yr = Y / Yr;
    zr = Z / Zr;

    if ( xr > eps )
        fx =  (float) pow(xr, 1 / 3.0f);
    else
        fx = (float) ((k * xr + 16.0f) / 116.0f);

    if ( yr > eps )
        fy =  (float) pow(yr, 1 / 3.0f);
    else
    fy = (float) ((k * yr + 16.0f) / 116.0f);

    if ( zr > eps )
        fz =  (float) pow(zr, 1 / 3.0f);
    else
        fz = (float) ((k * zr + 16.f) / 116);

    Ls = (116 * fy) - 16;
    as = 500 * (fx - fy);
    bs = 200 * (fy - fz);

    Lv = (uchar) (2.55f * Ls + .5f);
    av = (uchar) (as + .5f);
    bv = (uchar) (bs + .5f);
}
//;)
