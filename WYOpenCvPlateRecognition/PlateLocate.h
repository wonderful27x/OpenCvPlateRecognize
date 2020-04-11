#pragma once

//车牌定位基类
#ifndef PLATE_LOCATE_H
#define PLATE_LOCATE_H

#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;

class PlateLocate{
public:
	PlateLocate();
	~PlateLocate();
	int verifySizes(RotatedRect rotatedRect);
	void safeRect(Mat src, RotatedRect rect, Rect2f& safa_rect);
	void rotation(Mat src, Mat& dst, Size rect_size, Point2f center, double angle);
	void tortuosity(Mat src, vector<RotatedRect>& rotatedRects, vector<Mat>& plateCandidates);
	void drawRotatedRect(Mat dst,RotatedRect rotatedRect,Scalar scalar);
};

#endif // !PLATE_LOCATE_H
