#include <omp.h>

#include "imageProcessing.h"

using namespace cv;

int centerOfMass(cv::Mat src, cv::Point &center)
{
	if(src.channels() != 1) {
		return -1;
	}
	
	uint32_t count = 0;
	uint32_t xsum = 0;
	uint32_t ysum = 0;


    uchar* p;
    uchar c;
    
    //#pragma omp parallel for num_threads(4)\
	//	reduction(+:count) reduction(+:xsum) reduction(+:ysum)
    for(int i = 0; i < src.rows; ++i)
    {
        p = src.ptr<uchar>(i);
        for(int j = 0; j < src.cols; ++j)
        {
            c = p[j];
            if(c > 127) {
				xsum += j;
				ysum += i;
				count++;
			}
		}
	}
	
	if(count != 0) {
		center.x = xsum / count;
		center.y = ysum / count;
	} else {
		center.x = 320;
		center.y = 240;
	}
	
	return 0;
}

bool triThreshold(cv::Mat src, cv::Mat &dst, uint8_t highs[3], uint8_t lows[3])
{
	if(src.channels() == 3) {
		Mat tmp(src.rows, src.cols, CV_8U, Scalar(0));
		
		uchar* p;

		//#pragma omp parallel for num_threads(4)
		for(int i = 0; i < src.rows; i++) {
			p = src.ptr<uchar>(i);
			
			for(int j = 0; j < src.cols; j++) {
				uchar flag = 1;
				
				for(int k = 0; k < 3; k++) {
					uchar c;
					c = p[j * 3 + k];
					
					if(c > highs[k] || c < lows[k]) {
							flag = 0;
							break;
					}
				}

				//#pragma omp critical
				{
					tmp.at<uchar>(i, j) = flag * 255;
				}
			}
		}

		dst = tmp;
		return true;
	} else {
		return false;
	}
}
