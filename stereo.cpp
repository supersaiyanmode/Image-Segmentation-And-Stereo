// Skeleton code for B657 A4 Part 3.
// D. Crandall
//
// Run like this, for example:
//   ./stereo part3/Aloe/view1.png part3/Aloe/view5.png part3/Aloe/gt.png
// and output files will appear in part3/Aloe
//
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
#include <CImg.h>
#include <assert.h>

#include <MRF.h>
#include <Config.h>

using namespace cimg_library;
using namespace std;

double sqr(double a) { return a*a; }

// This code may or may not be helpful. :) It computes a 
//  disparity map by looking for best correspondences for each
//  window independently (no MRF).
//
CImg<double> naive_stereo(const CImg<double> &input1, 
		const CImg<double> &input2) {
	int max_disp = config.get<int>("stereo.max_disp");
	int window_size = config.get<int>("stereo.window_size");
	CImg<double> result(input1.width(), input1.height());

	for(int i=0; i<input1.height(); i++) {
		for(int j=0; j<input1.width(); j++) {
			pair<int, double> best_disp(0, INFINITY);

			for (int d=0; d < max_disp; d++) {
				double cost = 0;
				for(int ii = max(i-window_size, 0); 
						ii <= min(i+window_size, input1.height()-1); ii++) {
					for(int jj = max(j-window_size, 0); 
								jj <= min(j+window_size, input1.width()-1); jj++) {
						cost += sqr(input1(min(jj+d, input1.width()-1), ii) - 
											input2(jj, ii));
					}
				}
				if(cost < best_disp.second) {
					best_disp = make_pair(d, cost);
				}
			}
			result(j,i) = best_disp.first;
		}
	}
	return result;
}

// implement this!
//  this placeholder just returns a random disparity map
//
CImg<double> mrf_stereo(const CImg<double> &img1, const CImg<double> &img2)
{
	int window_size = config.get<int>("stereo.window_size");

	int depth = 8;
	MRF mrf(img1.width(), img1.height(), depth);
	CImg<double> D(img1.width(), img1.height(), depth, 1);
	for(int i=0; i<img1.height(); i++) {
		for(int j=0; j<img1.width(); j++) {
			for (int d=0; d < depth; d++) {
				double cost = 0;
				for(int ii = max(i-window_size, 0); 
						ii <= min(i+window_size, img1.height()-1); ii++) {
					for(int jj = max(j-window_size, 0); 
								jj <= min(j+window_size, img1.width()-1); jj++) {
						cost += sqr(img1(min(jj+d, img1.width()-1), ii) - 
											img2(jj, ii));
					}
				}
				D(j,i, d, 0) = cost;
			}
		}
	}
	std::cout<<"Done with D function."<<std::endl;

	return mrf.solve(D);
}

int main(int argc, char *argv[])
{
	if(argc != 4 && argc != 3)
	{
		cerr << "usage: " << argv[0] << " image_file1 image_file2 [gt_file]" << endl;
		return 1;
	}

	string input_filename1 = argv[1], input_filename2 = argv[2];
	string gt_filename;
	if(argc == 4)
		gt_filename = argv[3];

	// read in images and gt
	CImg<double> image1(input_filename1.c_str());
	CImg<double> image2(input_filename2.c_str());
	
	image1 = image1.get_resize_halfXY();
	image2 = image2.get_resize_halfXY();

	CImg<double> gt;

	if(gt_filename != "")
	{
		gt = CImg<double>(gt_filename.c_str());

		// gt maps are scaled by a factor of 3, undo this...
		for(int i=0; i<gt.height(); i++)
			for(int j=0; j<gt.width(); j++)
				gt(j,i) = gt(j,i) / 3.0;
	}

	// do naive stereo (matching only, no MRF)
	CImg<double> naive_disp = naive_stereo(image1, image2);
	naive_disp = naive_disp.get_resize_doubleXY();
	naive_disp.get_normalize(0,255).save((input_filename1 + "-disp_naive.png").c_str());

	// do stereo using mrf
	CImg<double> mrf_disp = mrf_stereo(image1, image2);
	mrf_disp = mrf_disp.get_resize_doubleXY();
	mrf_disp.get_normalize(0,255).save((input_filename1 + "-disp_mrf.png").c_str());

	// Measure error with respect to ground truth, if we have it...
	if(gt_filename != "")
	{
		cout << "Naive stereo technique mean error = " << (naive_disp-gt).sqr().sum()/gt.height()/gt.width() << endl;
		cout << "MRF stereo technique mean error = " << (mrf_disp-gt).sqr().sum()/gt.height()/gt.width() << endl;

	}

	return 0;
}

