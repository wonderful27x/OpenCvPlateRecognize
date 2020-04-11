
#include"AnnDiscern.h"

char AnnDiscern::chars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
 string AnnDiscern::chinese[] = { "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "³", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "ԥ", "��", "��", "��", "��", "��" };

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
�˹�������ʶ��
*/
string AnnDiscern::discern(Mat src)
{
	//Ԥ����
	Mat gray;
	cvtColor(src, gray, COLOR_BGR2GRAY);
	Mat shold;
	threshold(gray, shold, 0, 255, THRESH_OTSU + THRESH_BINARY);
	//������
	if (!cleanMud(shold)) {
		return string("�޷�ʶ��ĳ���");
	}

	imshow("�������",shold);

	//ȡÿ���ַ�������
	vector<Rect> charRects;
	vector<vector<Point>> contours;
	findContours(
		shold,              //����ͼ�� 
		contours,           //�����������
		RETR_EXTERNAL,      //ȡ�������
		CHAIN_APPROX_NONE   //ȡ�������������ص�
	);

	//����
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
	imshow("�����ַ�",src);

	//���򣬰����ο�x�����С����
	sort(
		charRects.begin(),
		charRects.end(),
		[](Rect& rectA, Rect rectB) {return rectA.x < rectB.x; }
	);

	//��ȡ���Ƴ����ַ�λ��
	int cityIndex = getCityIndex(src.cols,charRects);

	//���ƺ����ַ�λ��
	Rect chineseRect;
	getChinese(charRects[cityIndex], chineseRect);

	//��ȡ����7���ַ�Mat
	vector<Mat> plateChars;
	plateChars.push_back(shold(chineseRect));

	if (charRects.size() < 7) {
		return string("�޷�ʶ��ĳ���");
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
		sprintf(name,"��%d���ַ�",i);
		imshow(name, plateChars[i]);
	}

	//��ʼʶ���ַ�
	string plateNumber;
	predict(plateChars,plateNumber);

	return plateNumber;
}


//��������ϵ����-����������í�����µ����
//�㷨˼�룺�����ж����ص�����������Ӷ�ȷ�������Ƿ������
bool AnnDiscern::cleanMud(Mat& plate)
{
	//��������������Χ
	int minChangeCount = 10;
	int maxChangeCount = 60;
	//��¼ÿһ�е��������
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

	//�����ַ��ĸ߶�
	int charRows = 0;
	for each (int change in changes)
	{
		if (change > minChangeCount&& change < maxChangeCount) {
			charRows++;
		}
	}

	//�����ַ���ռ��,̫С��˵�����ǳ��ƣ������޷�ʶ��
	float heightPercent = charRows * 10.f / plate.rows;
	if (heightPercent < 0.4) {
		return false;
	}
	
	//�����ַ����ռ��
	int area = plate.rows * plate.cols;
	float areaRatio = countNonZero(plate) * 1.0 / area;
	if (areaRatio < 0.2 || areaRatio >0.6) {
		return false;
	}

	//�������������С������˵����������ͣ���ȫ��Ϊ0
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

//���˷��ַ����ο�
bool AnnDiscern::rectfilter(Mat mat)
{
	//�ַ������߱�
	float aspectRatio = 0.5f;
	//��ʵ��߱�
	float realRatio = float(mat.cols) / float(mat.rows);

	//�ַ��߶ȷ�Χ
	float minHeight = 10.0f;
	float maxHeight = 35.0f;

	//�ݴ���
	float error = 0.7f;

	//��С��߱�
	float minAspectRatio = 0.05f;
	//����߱�
	float maxAspectRatio = aspectRatio + aspectRatio * error;

	//�ַ����ռ��
	int area = mat.rows * mat.cols;
	float areaRatio = countNonZero(mat) *1.0 / area;

	if (realRatio >= minAspectRatio && realRatio <= maxAspectRatio &&
		mat.rows >= minHeight && mat.rows <= maxHeight &&
		areaRatio >= 0.1f && areaRatio <= 1.0f) {
		return true;
	}

	return false;
}

//��ȡ���Ƴ����ַ�����
//������7λ�������ַ�Ϊ�ڶ�λ����ռ�ȴ�Լ2/7
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

//�ҵ������ַ�λ��
void AnnDiscern::getChinese(Rect city, Rect& chinese)
{
	//�����ַ���ȱȳ����ַ���Щ
	int width = city.width * 1.15f;
	int x = city.x - width;
	chinese.x = x >= 0 ? x : 0;
	chinese.y = city.y;
	chinese.width = width;
	chinese.height = city.height;
}

//�����ַ�ʶ��
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

		//���㣬�����ƺ���λ����+��ĸ
		if (i) {
			annChar->predict(sample,response);
			//Ѱ�Ҿ���(һά���鵱������, ��Mat����) ����Сֵ�����ֵ��λ��.
			minMaxLoc(response, 0, 0, &minLoc, &maxLoc);
			int index = maxLoc.x;
			plateNumber += chars[index];
		}
		//����
		else{
			annChinese->predict(sample, response);
			minMaxLoc(response, 0, 0, &minLoc, &maxLoc);
			int index = maxLoc.x;
			plateNumber += chinese[index];
		}

	}
}

//��ȡhog����
void AnnDiscern::getHogFeature(HOGDescriptor* svmHog, Mat src, Mat& feature)
{
	//��һ��
	Mat trainImg = Mat(svmHog->winSize,CV_32S);
	resize(src,trainImg,svmHog->winSize);
	//��������
	vector<float> descriptor;
	svmHog->compute(trainImg, descriptor,svmHog->winSize);

	Mat featureMat(descriptor);
	featureMat.copyTo(feature);

	trainImg.release();
	featureMat.release();
}
