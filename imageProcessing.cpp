#include <omp.h>

#include "imageProcessing.h"

using namespace cv;

int centerOfMass(cv::Mat src, cv::Point &center)
{
	assert(src.channels() == 1);
	int count = 0;
	double xsum = 0;
	double ysum = 0;

#pragma omp parallel for
	for(int i = 0; i < src.rows; i++) {
		for(int j = 0; j < src.cols; j++) {
			if(src.at<uchar>(i, j) != 0) {
				xsum += j;
				ysum += i;
				count++;
			}
		}
	}
	
	center.x = cvRound(xsum / count);
	center.y = cvRound(ysum / count);
	
	return 0;
}

bool triThreshold(cv::Mat src, cv::Mat &dst, uint8_t highs[3], uint8_t lows[3])
{
	if(src.channels() == 3) {
		Mat tmp(src.rows, src.cols, CV_8U, Scalar(0));

#pragma omp parallel for
		for(int i = 0; i < src.rows; i++) {
			for(int j = 0; j < src.cols; j++) {
				uchar flag = 1;
				for(int k = 0; k < 3; k++) {
					if(src.at<Vec3b>(i, j)[k] > highs[k]\
						|| src.at<Vec3b>(i, j)[k] < lows[k]) {
							flag = 0;
							break;
					}
				}

				tmp.at<uchar>(i, j) = flag * 255;
			}
	}

		dst = tmp;
		return true;
	} else {
		return false;
	}
}
