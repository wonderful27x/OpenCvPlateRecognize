#pragma once

#ifndef SVM_PREDICT_H
#define SVM_PREDICT_H

#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;
using namespace ml;

//svm ≤‚∆¿…∏—°À„∑®
class SvmPredict {
public:
	SvmPredict(string svmModelPath);
	~SvmPredict();
	void predict(vector<Mat> plateCandidates, Mat& finalPlate);
	void getHogFeatures(HOGDescriptor* descriptor,Mat src,Mat &dst);
private:
	Ptr<SVM> svm;
	HOGDescriptor* descriptor;

};

#endif // !SVM_PREDICT_H

