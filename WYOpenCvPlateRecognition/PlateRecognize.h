#pragma once

//车牌识别操作类
#ifndef PLATE_RECOGNIZE_H
#define PLATE_RECOGNIZE_H

#include"SobelLocate.h"
#include"ColorLocate.h"
#include"SvmPredict.h"
#include "AnnDiscern.h"

class PlateRecognize {
public:
	PlateRecognize(string svmModelPath, string annCharModel, string annChineseModel);
	~PlateRecognize();
	string plateRecognize(Mat src);

private:
	SobelLocate* sobelLocate = nullptr;
	ColorLocate* colorLocate = nullptr;
	SvmPredict* svmPredict = nullptr;
	AnnDiscern* annDiscern = nullptr;
};

#endif // !PLATE_RECOGNIZE

