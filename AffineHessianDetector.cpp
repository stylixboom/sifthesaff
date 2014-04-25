#include <iostream>
#include <fstream>
#include <string>

#include "pyramid.h"
#include "helpers.h"
#include "affine.h"
#include "siftdesc.h"
#include "opencv2/core/core.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#define D 128

using namespace cv;
using namespace std;

struct Keypoint
{
	float x, y, s;
	float a11,a12,a21,a22;
	float response;
	int type;
	unsigned char desc[D];
};

class AffineHessianDetector : public HessianDetector, AffineShape, HessianKeypointCallback, AffineShapeCallback
{
	bool RootSIFT;
	const Mat image;
	SIFTDescriptor sift;
	vector<Keypoint> keys;

public:
	int g_numberOfPoints;
	int g_numberOfAffinePoints;
	AffineHessianDetector(const Mat &image, const PyramidParams &par, const AffineShapeParams &ap, const SIFTDescriptorParams &sp, bool isRootSIFT) :
	HessianDetector(par),
	AffineShape(ap),
	image(image),
	sift(sp)
	{
		RootSIFT = isRootSIFT;
		g_numberOfPoints = 0;
		g_numberOfAffinePoints = 0;
		this->setHessianKeypointCallback(this);
		this->setAffineShapeCallback(this);
	}

	~AffineHessianDetector(void){ Reset(); };

	void onHessianKeypointDetected(const Mat &blur, float x, float y, float s, float pixelDistance, int type, float response)
	{
		g_numberOfPoints++;
		findAffineShape(blur, x, y, s, pixelDistance, type, response);
	}

	void onAffineShapeFound(
	const Mat &blur, float x, float y, float s, float pixelDistance,
	float a11, float a12,
	float a21, float a22,
	int type, float response, int iters)
	{
		// convert shape into a up is up frame
		rectifyAffineTransformationUpIsUp(a11, a12, a21, a22);

		// now sample the patch
		if (!normalizeAffine(image, x, y, s, a11, a12, a21, a22))
		{
			// compute SIFT
			sift.computeSiftDescriptor(this->patch);
			// store the keypoint
			keys.push_back(Keypoint());
			Keypoint &k = keys.back();
			k.x = x; k.y = y; k.s = s; k.a11 = a11; k.a12 = a12; k.a21 = a21; k.a22 = a22; k.response = response; k.type = type;
			for (int i=0; i<D; i++)
			k.desc[i] = (unsigned char)sift.vec[i];
			// debugging stuff
			if (0)
			{
				cout << "x: " << x << ", y: " << y
				<< ", s: " << s << ", pd: " << pixelDistance
				<< ", a11: " << a11 << ", a12: " << a12 << ", a21: " << a21 << ", a22: " << a22
				<< ", t: " << type << ", r: " << response << endl;
				for (size_t i=0; i<sift.vec.size(); i++)
				cout << " " << sift.vec[i];
				cout << endl;
			}
			g_numberOfAffinePoints++;
		}
	}

	int extractKeypoints(vector< vector<float> >& kp, vector< vector<float> >& desc)
	{
		/*
		kp = new float[5*keys.size()];
		desc = new float[D*keys.size()];
		float* kp_ptr = kp;
		float* desc_ptr = desc;
		*/
		for (size_t i=0; i<keys.size(); i++)
		{
			Keypoint k = keys[i];

			float sc = AffineShape::par.mrSize * k.s;
			Mat A = (Mat_<float>(2,2) << k.a11, k.a12, k.a21, k.a22);
			SVD svd(A, SVD::FULL_UV);

			float *d = (float *)svd.w.data;
			d[0] = 1.0f/(d[0]*d[0]*sc*sc);
			d[1] = 1.0f/(d[1]*d[1]*sc*sc);

			A = svd.u * Mat::diag(svd.w) * svd.u.t();

			vector<float> p;
			p.push_back(k.x);
			p.push_back(k.y);
			p.push_back(A.at<float>(0,0));
			p.push_back(A.at<float>(0,1));
			p.push_back(A.at<float>(1,1));
			kp.push_back(p);
			if (RootSIFT)
			{
				float sum_of_desc = 0;
				for (size_t i=0; i<D; i++)
					sum_of_desc += k.desc[i];
				vector<float> des;
				for (size_t i=0; i<D; i++)
					des.push_back(sqrt(k.desc[i]/sum_of_desc));
				desc.push_back(des);
			}
			else
			{
				vector<float> des;
				for (size_t i=0; i<D; i++)
					des.push_back(k.desc[i]);
				desc.push_back(des);
			}

		}
		return keys.size();
	}

	void Reset()
	{
		// Release Mem
		if(keys.size() != 0)
		{
			keys.clear();

			g_numberOfPoints = 0;
			g_numberOfAffinePoints = 0;
		}
	}
};
