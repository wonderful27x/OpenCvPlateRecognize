#pragma once

#ifndef COLOR_LOCATE_H
#define COLOR_OCATE_H

#include"PlateLocate.h"

//车牌定位-颜色空间HSV算法
class ColorLocate :public PlateLocate {
public:
	ColorLocate();
	~ColorLocate();
	void locate(Mat src, vector<Mat>& plateCandidates);
};

#endif // COLOR_LOCATE_H


