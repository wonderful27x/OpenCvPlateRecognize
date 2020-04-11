#include"SvmPredict.h"

SvmPredict::SvmPredict(string svmModelPath)
{
	//加载svm模型
	svm = SVM::load(svmModelPath);
	//创建特征描述
	descriptor = new HOGDescriptor(Size(128, 64), Size(16, 16), Size(8, 8), Size(8, 8), 3);
}

SvmPredict::~SvmPredict()
{
}

/*
使用svm算法对候选车牌进行测评，筛选出评分最低的（最接近模型的）作为最终车牌
plateCandidates：候选车牌
finalPlate：最终输出车牌
https://www.jianshu.com/p/0381cac6dc30
*/
void SvmPredict::predict(vector<Mat> plateCandidates, Mat& finalPlate)
{
	float score;                 //svm评分
	float minScore = FLT_MAX;    //最小评分，初始化为最大
	int index = -1;              //最小评分索引
	for (int i = 0; i < plateCandidates.size(); i++) {
		Mat plate = plateCandidates[i];
		//预处理
		Mat gray;
		cvtColor(plate, gray, COLOR_BGR2GRAY);
		Mat shold;
		threshold(gray, shold, 0, 255, THRESH_OTSU + THRESH_BINARY);
		//提取特征
		Mat features;
		getHogFeatures(descriptor,shold,features);
		//归一化，此函数既可以改变矩阵的通道数，又可以对矩阵元素进行序列化
		//p1->1:通道数为1，p2->1：行数为1
		Mat sample = features.reshape(1, 1);
		//交给svm测评，返回测评分数
		score = svm->predict(sample,noArray(),StatModel::Flags::RAW_OUTPUT);
		printf("侯选车牌%d svm评分：%f\n", i, score);
		if (score < minScore) {
			minScore = score;
			index = i;
		}
	}

	if (index >= 0) {
		finalPlate = plateCandidates[index].clone();
		imshow("SVM最终车牌",finalPlate);
		printf("侯选车牌为%d\n", index);
		waitKey();
	}
}

void SvmPredict::getHogFeatures(HOGDescriptor* descriptor, Mat src, Mat& dst)
{
	//归一化
	Mat trainImg = Mat(descriptor->winSize, CV_32S);
	resize(src, trainImg, descriptor->winSize);
	//计算特征
	vector<float> svmDescriptor;
	descriptor->compute(trainImg, svmDescriptor, descriptor->winSize);
	//转成Mat输出
	Mat feature(svmDescriptor);
	feature.copyTo(dst);
	//释放内存
	trainImg.release();
	feature.release();
	cout << "winW: " << descriptor->winSize.width << "winH: " << descriptor->winSize.height << endl;
}
