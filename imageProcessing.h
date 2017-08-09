
#ifndef _IMAGEPROCESSING_H
#define _IMAGEPROCESSING_H

#include <opencv2/opencv.hpp>

int centerOfMass(cv::Mat src, cv::Point &center);
bool triThreshold(cv::Mat src, cv::Mat &dst, uint8_t highs[3], uint8_t lows[3]);

#endif
