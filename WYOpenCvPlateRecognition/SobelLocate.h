#pragma once

//���ƶ�λ-sobel�㷨
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

