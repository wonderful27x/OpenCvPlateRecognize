#pragma once

#ifndef COLOR_LOCATE_H
#define COLOR_OCATE_H

#include"PlateLocate.h"

//���ƶ�λ-��ɫ�ռ�HSV�㷨
class ColorLocate :public PlateLocate {
public:
	ColorLocate();
	~ColorLocate();
	void locate(Mat src, vector<Mat>& plateCandidates);
};

#endif // COLOR_LOCATE_H


