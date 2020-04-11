#include"SobelLocate.h"

SobelLocate::SobelLocate()
{
}

SobelLocate::~SobelLocate()
{
}

/*
sobel�㷨���ƶ�λ
src����ʶ��ԭͼ
plateCandidates����λ�ĳ��ƺ�ѡ
*/
void SobelLocate::locate(Mat src, vector<Mat>& plateCandidates)
{
	//  1.��˹ģ����ȥ�룬Ϊ��Ե����㷨��׼����
	//  p2->Size(5, 5)����˹ģ���뾶��opencv��ֻ֧�ֻ����뾶���뾶Խ��ͼƬԽģ��,�����Size(5, 5)��һ������ֵ
	//  p3->0:X�����ϵĸ�˹�˱�׼ƫ�
	Mat blur;
	GaussianBlur(src, blur, Size(5, 5), 0);
	//imshow("��˹ģ��", blur);

	//	2.�ҶȻ� ��Ϊ��Ե����㷨׼���ҶȻ�������
	Mat gray;
	cvtColor(blur, gray, COLOR_BGR2GRAY);
	//imshow("�Ҷ�ͼ", gray);

	//	3.sobel���� ���õ�ͼ���һ��ˮƽ��������
	//  p2->CV_16S������16λ����������ֹ����������ݲ�����(�󵼺�ĵ���ֵ����С��0���ߴ���255��8λ�治��)
	//  p3->1����x��
	Mat sobel_16;
	Sobel(gray, sobel_16, CV_16S, 1, 0);
	Mat sobel;
	//ת��8λ
	convertScaleAbs(sobel_16, sobel);
	//imshow("sobel", sobel);

	//	4.��ֵ�� ����ͼ���ÿ��������һ����ֵ����Ϊ��������̬ѧ����׼������
	//  ����ֵ�����ʺ����ǳ��
	//  ����ֵ�����ʺ�ǳ������
	//  p2->0:��ֵȡ0������Ӧ
	//  p3->255����ֵ���ֵ
	//  p4->THRESH_OTSU + THRESH_BINARY:��ֵ���㷨
	Mat shold;
	threshold(sobel,shold,0,255, THRESH_OTSU + THRESH_BINARY);
	//imshow("��ֵ��", shold);

	//	5.��̬ѧ���� ���ղ������������ַ����ӳ�һ����ͨ���򣬱���ȡ������
	//  ����ָ����С����״�ĽṹԪ����������̬����,�������Ϊ��̬ѧ�����и�ʴ���͵�ģ��
	//  p1->Size(17,3):Ԫ�ش�С�������Ǿ���ֵ
	Mat element = getStructuringElement(MORPH_RECT,Size(17,3));
	Mat close;
	//  ������̬ѧ��������̬ѧ���еĻ����Ǹ�ʴ������
	//  p2->MORPH_CLOSE���ղ����������ͺ�ʴ��
	//  p3->element����̬ѧ������ģ��
	morphologyEx(shold, close, MORPH_CLOSE, element);
	//imshow("close", close);

	//	6.������ ������ͨ�����Χ�������������γ���Ӿ��Σ�
	// ����һ������contours���ڽ�����Ӿ���������������������ɵģ��������ɵ���ɵ�
	vector<vector<Point>> contours;
	findContours(
		close,              //����ͼ�� 
		contours,           //�����������
		RETR_EXTERNAL,      //ȡ�������
		CHAIN_APPROX_NONE   //ȡ�������������ص�
	);

	//	7.�жϳߴ� ������ɸѡ�ų��������ǳ��Ƶľ��Σ��й����Ƶ�һ���С��440mm * 140mm����߱�Ϊ3.14��
	RotatedRect rotateRect;
	vector<RotatedRect> rotateRects;
	Mat clone = src.clone();
	for each (vector<Point> contour in contours)
	{
		//ȡ��С��Ӿ��Σ�����ת/���Ƕȣ�
		rotateRect = minAreaRect(contour);  

		////��ԭͼ�ϻ���ת������С��Ӿ���
		////rotateRect.boundingRect()����ת������С��Ӿ���
		//rectangle(src,rotateRect.boundingRect(),Scalar(0,0,255));

		//ֱ����ԭͼ�ϻ�����ת����
		drawRotatedRect(clone,rotateRect,Scalar(0,0,255));

		//�Գߴ�������г���ɸѡ
		if (verifySizes(rotateRect)) {
			rotateRects.push_back(rotateRect);
		}
	}
	//imshow("���ƶ�λ���ο�-δɾѡ",src);
	for each (RotatedRect rect in rotateRects)
	{
		////��ԭͼ�ϻ�����ɾѡ�����ת������С��Ӿ���
		//rectangle(src, rect.boundingRect(), Scalar(0, 255, 0));

		//ֱ����ԭͼ�ϻ�����ɾѡ�����ת����
		drawRotatedRect(clone, rect, Scalar(0, 255, 0));
	}
	imshow("���ƶ�λ���ο�-����ɾѡ-sobel", clone);

	//	8.�жϽǶ� ������ɸѡ�ų��������ǳ��Ƶľ��Σ�
	//	9.��ת���� ����ƫб�ĳ��Ƶ���Ϊˮƽ��Ϊ����ĳ����ж����ַ�ʶ����߳ɹ��ʣ�
	//	10.������С ��ȷ����ѡ���Ƶ������ѧϰģ��֮ǰ�ߴ�һ�£�
	tortuosity(src, rotateRects, plateCandidates);
	for each (Mat mat in plateCandidates)
	{
		//imshow("sobel ��ѡ����",mat);
		//waitKey();
	}
	//waitKey();
}
