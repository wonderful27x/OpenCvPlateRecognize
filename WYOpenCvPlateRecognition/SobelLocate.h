#pragma once

//车牌定位-sobel算法
#ifndef SOBEL_LOCATE_H
#define SOBEL_LOCATE_H

#include"PlateLocate.h"

class SobelLocate : public PlateLocate{
public:
	SobelLocate();
	~SobelLocate();
	void locate(Mat src,vector<Mat> &plateCandidates);
};

#endif // !SOBEL_LOCATE

