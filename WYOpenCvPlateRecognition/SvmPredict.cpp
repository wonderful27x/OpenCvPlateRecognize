#include"SvmPredict.h"

SvmPredict::SvmPredict(string svmModelPath)
{
	//����svmģ��
	svm = SVM::load(svmModelPath);
	//������������
	descriptor = new HOGDescriptor(Size(128, 64), Size(16, 16), Size(8, 8), Size(8, 8), 3);
}

SvmPredict::~SvmPredict()
{
}

/*
ʹ��svm�㷨�Ժ�ѡ���ƽ��в�����ɸѡ��������͵ģ���ӽ�ģ�͵ģ���Ϊ���ճ���
plateCandidates����ѡ����
finalPlate�������������
https://www.jianshu.com/p/0381cac6dc30
*/
void SvmPredict::predict(vector<Mat> plateCandidates, Mat& finalPlate)
{
	float score;                 //svm����
	float minScore = FLT_MAX;    //��С���֣���ʼ��Ϊ���
	int index = -1;              //��С��������
	for (int i = 0; i < plateCandidates.size(); i++) {
		Mat plate = plateCandidates[i];
		//Ԥ����
		Mat gray;
		cvtColor(plate, gray, COLOR_BGR2GRAY);
		Mat shold;
		threshold(gray, shold, 0, 255, THRESH_OTSU + THRESH_BINARY);
		//��ȡ����
		Mat features;
		getHogFeatures(descriptor,shold,features);
		//��һ�����˺����ȿ��Ըı�����ͨ�������ֿ��ԶԾ���Ԫ�ؽ������л�
		//p1->1:ͨ����Ϊ1��p2->1������Ϊ1
		Mat sample = features.reshape(1, 1);
		//����svm���������ز�������
		score = svm->predict(sample,noArray(),StatModel::Flags::RAW_OUTPUT);
		printf("��ѡ����%d svm���֣�%f\n", i, score);
		if (score < minScore) {
			minScore = score;
			index = i;
		}
	}

	if (index >= 0) {
		finalPlate = plateCandidates[index].clone();
		imshow("SVM���ճ���",finalPlate);
		printf("��ѡ����Ϊ%d\n", index);
		waitKey();
	}
}

void SvmPredict::getHogFeatures(HOGDescriptor* descriptor, Mat src, Mat& dst)
{
	//��һ��
	Mat trainImg = Mat(descriptor->winSize, CV_32S);
	resize(src, trainImg, descriptor->winSize);
	//��������
	vector<float> svmDescriptor;
	descriptor->compute(trainImg, svmDescriptor, descriptor->winSize);
	//ת��Mat���
	Mat feature(svmDescriptor);
	feature.copyTo(dst);
	//�ͷ��ڴ�
	trainImg.release();
	feature.release();
	cout << "winW: " << descriptor->winSize.width << "winH: " << descriptor->winSize.height << endl;
}
