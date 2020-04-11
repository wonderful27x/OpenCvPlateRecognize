#pragma once

#ifndef ANN_DISCERN_H
#define ANN_DISCERN_H

#include<opencv2/opencv.hpp>
using namespace std;
using namespace cv;
using namespace ml;

//�˹�������ʶ��
class AnnDiscern {
public:
	AnnDiscern(string annCharModel, string annChineseModel);
	~AnnDiscern();
	string discern(Mat src);
	void getHogFeature(HOGDescriptor* svmHog,Mat src,Mat &feature);
	bool cleanMud(Mat& plate);
private:
	Ptr<ANN_MLP> annChar;                                       //��ĸ+����ʶ��
	Ptr<ANN_MLP> annChinese;                                    //����ʶ��
	HOGDescriptor* descriptor = nullptr;                        //hog����
	static string chinese[];                                    //���ּ���
	static char chars[];                                        //��ĸ+���ּ���
	bool rectfilter(Mat mat);                                   //���˵������ַ��ľ��ο�
	int getCityIndex(int width, vector<Rect> rects);            //��ȡ�����ַ�����
	void getChinese(Rect city, Rect& chinese);                  //�ҵ������ַ�λ��
	void predict(vector<Mat> plateChars, string& plateNumber);  //��ʼʶ�����ַ�
};

#endif // !ANN_DISCERN_H
