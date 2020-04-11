
#include"PlateRecognize.h"

PlateRecognize::PlateRecognize(string svmModelPath, string annCharModel, string annChineseModel)
{
	this->sobelLocate = new SobelLocate();
	this->colorLocate = new ColorLocate();
	this->svmPredict = new SvmPredict(svmModelPath);
	this->annDiscern = new AnnDiscern(annCharModel, annChineseModel);
}

PlateRecognize::~PlateRecognize()
{
	if (!sobelLocate) {
		delete(sobelLocate);
		sobelLocate = nullptr;
	}
	if (!colorLocate) {
		delete(colorLocate);
		colorLocate = nullptr;
	}
	if (!svmPredict) {
		delete(svmPredict);
		svmPredict = nullptr;
	}
	if (!annDiscern) {
		delete(annDiscern);
		annDiscern = nullptr;
	}
}


/*
车牌识别函数
src：包含车牌信息的待识别图像
return：返回车牌字符串
*/
string PlateRecognize::plateRecognize(Mat src)
{
	//候选车牌,sobel算法
	vector<Mat> plateCandidatesSobel;
	//候选车牌，颜色空间HSV算法
	vector<Mat> plateCandidatesColor;
	//开始定位车牌，并返回车牌候选列表
	sobelLocate->locate(src, plateCandidatesSobel);
	//开始定位车牌，并返回车牌候选列表
	colorLocate->locate(src, plateCandidatesColor);
	//合并两个候选车牌集合
	vector<Mat> plates;
	plates.insert(plates.end(), plateCandidatesSobel.begin(), plateCandidatesSobel.end());
	plates.insert(plates.end(), plateCandidatesColor.begin(), plateCandidatesColor.end());

	//char name[100] ;
	//for (int i = 0; i < plates.size(); i++) {
	//	sprintf(name,"候选车牌%d",i);
	//	imshow(name,plates[i]);
	//	waitKey();
	//}

	//使用svm模型算法筛选出最终车牌
	Mat finalPlate;
	svmPredict->predict(plates, finalPlate);
	//人工神经网络识别
	string plateNumber = annDiscern->discern(finalPlate);

	return plateNumber;
}

