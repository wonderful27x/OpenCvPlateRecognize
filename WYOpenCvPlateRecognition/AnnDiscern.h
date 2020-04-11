#pragma once

#ifndef ANN_DISCERN_H
#define ANN_DISCERN_H

#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;
using namespace ml;

//人工神经网络识别
class AnnDiscern {
public:
	AnnDiscern(string annCharModel, string annChineseModel);
	~AnnDiscern();
	string discern(Mat src);
	void getHogFeature(HOGDescriptor* svmHog,Mat src,Mat &feature);
	bool cleanMud(Mat& plate);
private:
	Ptr<ANN_MLP> annChar;                                       //字母+数字识别
	Ptr<ANN_MLP> annChinese;                                    //汉字识别
	HOGDescriptor* descriptor = nullptr;                        //hog特征
	static string chinese[];                                    //汉字集合
	static char chars[];                                        //字母+数字集合
	bool rectfilter(Mat mat);                                   //过滤掉不是字符的矩形框
	int getCityIndex(int width, vector<Rect> rects);            //获取城市字符索引
	void getChinese(Rect city, Rect& chinese);                  //找到汉字字符位置
	void predict(vector<Mat> plateChars, string& plateNumber);  //开始识别车牌字符
};

#endif // !ANN_DISCERN_H
