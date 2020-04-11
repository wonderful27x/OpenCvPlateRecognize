
#include"AnnDiscern.h"

char AnnDiscern::chars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
 string AnnDiscern::chinese[] = { "川", "鄂", "赣", "甘", "贵", "桂", "黑", "沪", "冀", "津", "京", "吉", "辽", "鲁", "蒙", "闽", "宁", "青", "琼", "陕", "苏", "晋", "皖", "湘", "新", "豫", "渝", "粤", "云", "藏", "浙" };

AnnDiscern::AnnDiscern(string aanCharModel, string aanChineseModel)
{
	this->annChar = ANN_MLP::load(aanCharModel);
	this->annChinese = ANN_MLP::load(aanChineseModel);
	this->descriptor = new HOGDescriptor(Size(32, 32), Size(16, 16), Size(8, 8), Size(8, 8), 3);
}


AnnDiscern::~AnnDiscern()
{
	annChar->clear();
	annChar.release();
	annChinese->clear();
	annChinese.release();
	if (descriptor) {
		delete descriptor;
		descriptor = nullptr;
	}

}

/*
人工神经网络识别
*/
string AnnDiscern::discern(Mat src)
{
	//预处理
	Mat gray;
	cvtColor(src, gray, COLOR_BGR2GRAY);
	Mat shold;
	threshold(gray, shold, 0, 255, THRESH_OTSU + THRESH_BINARY);
	//清除泥巴
	if (!cleanMud(shold)) {
		return string("无法识别的车牌");
	}

	imshow("擦除泥巴",shold);

	//取每个字符的轮廓
	vector<Rect> charRects;
	vector<vector<Point>> contours;
	findContours(
		shold,              //输入图像 
		contours,           //输出轮廓集合
		RETR_EXTERNAL,      //取外接轮廓
		CHAIN_APPROX_NONE   //取轮廓上所有像素点
	);

	//过滤
	for each (vector<Point> points in contours)
	{
		Rect rect = boundingRect(points);
		rectangle(src, rect, Scalar(0, 0, 255));
		Mat mat = shold(rect);
		if (rectfilter(mat)) {
			charRects.push_back(rect);
		}
	}
	for each (Rect charRect in charRects)
	{
		rectangle(src, charRect, Scalar(0, 255, 0));
	}
	imshow("车牌字符",src);

	//排序，按矩形框x坐标从小到大
	sort(
		charRects.begin(),
		charRects.end(),
		[](Rect& rectA, Rect rectB) {return rectA.x < rectB.x; }
	);

	//获取车牌城市字符位置
	int cityIndex = getCityIndex(src.cols,charRects);

	//反推汉字字符位置
	Rect chineseRect;
	getChinese(charRects[cityIndex], chineseRect);

	//获取车牌7个字符Mat
	vector<Mat> plateChars;
	plateChars.push_back(shold(chineseRect));

	if (charRects.size() < 7) {
		return string("无法识别的车牌");
	}

	for (int i = cityIndex; i < charRects.size(); i++)
	{
		Mat mat = shold(charRects[i]);
		plateChars.push_back(mat);
		if (plateChars.size() == 7) {
			break;
		}
	}

	char name[100];
	for (int i = 0; i < plateChars.size(); i++) {
		sprintf(name,"第%d个字符",i);
		imshow(name, plateChars[i]);
	}

	//开始识别字符
	string plateNumber;
	predict(plateChars,plateNumber);

	return plateNumber;
}


//清除车牌上的泥巴-擦除车牌上铆钉留下的噪点
//算法思想：逐行判断像素的跳变次数，从而确定此行是否有噪点
bool AnnDiscern::cleanMud(Mat& plate)
{
	//非噪点跳变次数范围
	int minChangeCount = 10;
	int maxChangeCount = 60;
	//记录每一行的跳变次数
	vector<int> changes;
	for (int h = 0; h < plate.rows; h++) {
		int count = 0;
		for (int w = 0; w < plate.cols - 1; w++) {
			if (plate.at<uchar>(h, w) != plate.at<uchar>(h, w + 1)) {
				count++;
			}
		}
		changes.push_back(count);
	}

	//车牌字符的高度
	int charRows = 0;
	for each (int change in changes)
	{
		if (change > minChangeCount&& change < maxChangeCount) {
			charRows++;
		}
	}

	//车牌字符高占比,太小则说明不是车牌，或者无法识别
	float heightPercent = charRows * 10.f / plate.rows;
	if (heightPercent < 0.4) {
		return false;
	}
	
	//车牌字符面积占比
	int area = plate.rows * plate.cols;
	float areaRatio = countNonZero(plate) * 1.0 / area;
	if (areaRatio < 0.2 || areaRatio >0.6) {
		return false;
	}

	//跳变次数低于最小次数，说明此行有泥巴，则全置为0
	for (int h = 0; h < changes.size(); h++) {
		if (changes[h] < minChangeCount) {
			for (int w = 0; w < plate.cols; w++)
			{
				plate.at<uchar>(h, w) = 0;
			}
		}
	}

	return true;
}

//过滤非字符矩形框
bool AnnDiscern::rectfilter(Mat mat)
{
	//字符理想宽高比
	float aspectRatio = 0.5f;
	//真实宽高比
	float realRatio = float(mat.cols) / float(mat.rows);

	//字符高度范围
	float minHeight = 10.0f;
	float maxHeight = 35.0f;

	//容错率
	float error = 0.7f;

	//最小宽高比
	float minAspectRatio = 0.05f;
	//最大宽高比
	float maxAspectRatio = aspectRatio + aspectRatio * error;

	//字符面积占比
	int area = mat.rows * mat.cols;
	float areaRatio = countNonZero(mat) *1.0 / area;

	if (realRatio >= minAspectRatio && realRatio <= maxAspectRatio &&
		mat.rows >= minHeight && mat.rows <= maxHeight &&
		areaRatio >= 0.1f && areaRatio <= 1.0f) {
		return true;
	}

	return false;
}

//获取车牌城市字符索引
//车牌有7位，城市字符为第二位，宽占比大约2/7
int AnnDiscern::getCityIndex(int width,vector<Rect> rects)
{
	float left = width * 1.0 / 7;
	float right = width * 2.0 / 7;
	for (int i = 0; i < rects.size(); i++) {
		Rect rect = rects[i];
		float mid = rect.x + rect.width / 2.0;
		if (mid > left && mid < right) {
			return i;
		}
	}
	return 0;
}

//找到汉字字符位置
void AnnDiscern::getChinese(Rect city, Rect& chinese)
{
	//汉族字符宽度比城市字符宽些
	int width = city.width * 1.15f;
	int x = city.x - width;
	chinese.x = x >= 0 ? x : 0;
	chinese.y = city.y;
	chinese.width = width;
	chinese.height = city.height;
}

//车牌字符识别
void AnnDiscern::predict(vector<Mat> plateChars, string &plateNumber)
{
	for (int i = 0; i < plateChars.size(); i++)
	{
		Mat charMat = plateChars[i];
		Mat feature;
		getHogFeature(descriptor, charMat, feature);

		Mat sample = feature.reshape(1,1);
		Mat response;
		Point minLoc;
		Point maxLoc;

		//非零，即车牌后六位数字+字母
		if (i) {
			annChar->predict(sample,response);
			//寻找矩阵(一维数组当作向量, 用Mat定义) 中最小值和最大值的位置.
			minMaxLoc(response, 0, 0, &minLoc, &maxLoc);
			int index = maxLoc.x;
			plateNumber += chars[index];
		}
		//汉字
		else{
			annChinese->predict(sample, response);
			minMaxLoc(response, 0, 0, &minLoc, &maxLoc);
			int index = maxLoc.x;
			plateNumber += chinese[index];
		}

	}
}

//提取hog特征
void AnnDiscern::getHogFeature(HOGDescriptor* svmHog, Mat src, Mat& feature)
{
	//归一化
	Mat trainImg = Mat(svmHog->winSize,CV_32S);
	resize(src,trainImg,svmHog->winSize);
	//计算特征
	vector<float> descriptor;
	svmHog->compute(trainImg, descriptor,svmHog->winSize);

	Mat featureMat(descriptor);
	featureMat.copyTo(feature);

	trainImg.release();
	featureMat.release();
}
