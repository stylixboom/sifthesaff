/*
 * ins_offline.h
 *
 *  Created on: January 25, 2013
 *      Author: Siriwat Kasamwattanarote
 */

#include "SIFThesaff.h"

using namespace std;
using namespace cv;
using namespace alphautils;

SIFThesaff::SIFThesaff(int Colorspace, bool isNormalize, bool isRootSIFT)
{
    colorspace = Colorspace;
    normalize = isNormalize;
	RootSIFT = isRootSIFT;
	g_numberOfPoints = 0;
	g_numberOfAffinePoints = 0;
	num_kp = 0;
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
		threshold = 16.0f/4.6875f;
		//threshold = 0.04f * 256 / 3;
		//threshold = 16.0f/5.0f; // affect detector, higher t for lower number of feature, lower t for higher number of feature (high sensitive)
		max_iter = 16; // affect detector, higher it for higher number of feature, lower it for lower number of feature (low sensitive)
		desc_factor = 3.0f*sqrt(3.0f); // affect descriptor
		patch_size = 41; // affect descriptor
		verbose = false;
	}
};

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
			int head_len = GetSIFTHeadSize();
			int len_sift = GetSIFTD();
			int num_sift = num_kp;

            // Write width
            OutFile.write(reinterpret_cast<char*>(&width), sizeof(width));

            // Write height
            OutFile.write(reinterpret_cast<char*>(&height), sizeof(height));

			// Write len_sift
			OutFile.write(reinterpret_cast<char*>(&len_sift), sizeof(len_sift));

			// Write num_sift
			OutFile.write(reinterpret_cast<char*>(&num_sift), sizeof(num_sift));

			// Write sift "x y a b c" and "all dimensions"
			for(int sift_count = 0; sift_count != num_sift; sift_count++)
			{
				// Write sift head
				OutFile.write(reinterpret_cast<char*>(&kp[sift_count][0]), head_len * sizeof(kp[sift_count][0]));

				// Write sift data
				OutFile.write(reinterpret_cast<char*>(&desc[sift_count][0]), len_sift * sizeof(desc[sift_count][0]));
			}
			// Close file
			OutFile.close();
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

            for(size_t kpIdx = 0; kpIdx < kp.size(); kpIdx++)
            {
                OutFile << kp[kpIdx][0] << " " << kp[kpIdx][1] << " " << kp[kpIdx][2] << " " << kp[kpIdx][3] << " " << kp[kpIdx][4] << " ";
                for(size_t descIdx = 0; descIdx < desc[kpIdx].size(); descIdx++)
                    OutFile << desc[kpIdx][descIdx] << " ";
                OutFile << endl;
            }

            OutFile.close();
        }
	}
}

void SIFThesaff::importKeypoints(const string& in, bool isBinary)
{
    if(!is_path_exist(in)) // no exist
    {
        cout << "File \"" << in << "\" not found" << endl;
        return;
    }

	Reset();

    // Oxford format
	if(isBinary)
	{
		ifstream InFile (in.c_str(), ios::binary);
		if (InFile)
		{
			int head_len = GetSIFTHeadSize();

            // Read width
            InFile.read((char*)(&width), sizeof(width));

            // Read height
            InFile.read((char*)(&height), sizeof(height));

			// Read len_sift
            int len_sift;
            InFile.read((char*)(&len_sift), sizeof(len_sift));

			// Read num_sift
            int num_sift;
            InFile.read((char*)(&num_sift), sizeof(num_sift));
            num_kp = num_sift;

			// Read sift "x y a b c" and "all dimensions"
			for(int sift_count = 0; sift_count != num_sift; sift_count++)
			{
				// Read sift head "x y a b c"
                vector<float> sub_kp;
				for(int head_count = 0; head_count < head_len; head_count++)
				{
                    float read_head;
                    InFile.read((char*)(&read_head), sizeof(read_head));
                    sub_kp.push_back(read_head);
                }
                kp.push_back(sub_kp);

				// Read sift data "128D"
                vector<float> sub_desc;
				for(int data_count = 0; data_count < len_sift; data_count++)
				{
                    float read_data;
                    InFile.read((char*)(&read_data), sizeof(read_data));
                    sub_desc.push_back(read_data);
                }
                desc.push_back(sub_desc);
			}
			// Close file
			InFile.close();
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
}

int SIFThesaff::checkNumKp(const string& in, bool isBinary)
{
    int ret_num_sift = -1;

    if(!is_path_exist(in)) // no exist
    {
        cout << "File \"" << in << "\" not found" << endl;
        return -1;
    }

    // Oxford format
	if(isBinary)
	{
		ifstream InFile (in.c_str(), ios::binary);
		if (InFile)
		{
            // Read width
            InFile.read((char*)(&width), sizeof(width));

            // Read height
            InFile.read((char*)(&height), sizeof(height));

			// Read len_sift
            int len_sift;
            InFile.read((char*)(&len_sift), sizeof(len_sift));

			// Read ret_num_sift
            InFile.read((char*)(&ret_num_sift), sizeof(ret_num_sift));

            // Stop reading the rest

			// Close file
			InFile.close();
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

	return ret_num_sift; // num_kp
}

void SIFThesaff::extractPerdochSIFT(const string& imgPath)
{
	Mat tmp = imread(imgPath);
	extractPerdochSIFT(tmp);
	tmp.release();
}

void SIFThesaff::extractPerdochSIFT(const Mat& tmp)
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
}

void SIFThesaff::Reset(void)
{
	// Release Mem
	if(kp.size() != 0)
	{
		for(size_t idx = 0; idx < kp.size(); idx++)
		{
			kp[idx].clear();
			desc[idx].clear();
		}
		kp.clear();
		desc.clear();

		g_numberOfPoints = 0;
		g_numberOfAffinePoints = 0;
		num_kp = 0;
	}
}

int SIFThesaff::GetSIFTD(void)
{
	return D;
}

int SIFThesaff::GetSIFTHeadSize(void)
{
	return HEADSIZE;
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
