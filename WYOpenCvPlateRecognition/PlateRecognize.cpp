
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
����ʶ����
src������������Ϣ�Ĵ�ʶ��ͼ��
return�����س����ַ���
*/
string PlateRecognize::plateRecognize(Mat src)
{
	//��ѡ����,sobel�㷨
	vector<Mat> plateCandidatesSobel;
	//��ѡ���ƣ���ɫ�ռ�HSV�㷨
	vector<Mat> plateCandidatesColor;
	//��ʼ��λ���ƣ������س��ƺ�ѡ�б�
	sobelLocate->locate(src, plateCandidatesSobel);
	//��ʼ��λ���ƣ������س��ƺ�ѡ�б�
	colorLocate->locate(src, plateCandidatesColor);
	//�ϲ�������ѡ���Ƽ���
	vector<Mat> plates;
	plates.insert(plates.end(), plateCandidatesSobel.begin(), plateCandidatesSobel.end());
	plates.insert(plates.end(), plateCandidatesColor.begin(), plateCandidatesColor.end());

	//char name[100] ;
	//for (int i = 0; i < plates.size(); i++) {
	//	sprintf(name,"��ѡ����%d",i);
	//	imshow(name,plates[i]);
	//	waitKey();
	//}

	//ʹ��svmģ���㷨ɸѡ�����ճ���
	Mat finalPlate;
	svmPredict->predict(plates, finalPlate);
	//�˹�������ʶ��
	string plateNumber = annDiscern->discern(finalPlate);

	return plateNumber;
}

