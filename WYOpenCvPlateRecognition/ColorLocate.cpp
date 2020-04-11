
#include"ColorLocate.h"

ColorLocate::ColorLocate()
{
}

ColorLocate::~ColorLocate()
{
}

/*
���ƶ�λ,����ٶ���������ɫ��
��ɫ����hsv�ռ�
h:100-124
s:43-255
v:46-255
src��ԭͼ��
plateCandidates�������ѡ����
*/
void ColorLocate::locate(Mat src, vector<Mat>& plateCandidates)
{
	//bgrתhsv
	Mat hsv;
	cvtColor(src, hsv, COLOR_BGR2HSV);
	//imshow("hsv", hsv);
	//������ɫ�������޳���������
	int channels = hsv.channels();
	int width = hsv.cols * channels;
	int height = hsv.rows;
	uchar* p;
	int h, s, v;
	bool isBlue = false;
	//�ж��������ڴ��Ƿ������洢������ǿ����Ż��Լ���ѭ������
	if (hsv.isContinuous()) {
		width = width * height;
		height = 1;
	}
	for (int h = 0; h < height; h++) {
		//ָ��ָ������
		p = hsv.ptr<uchar>(h);
		for (int w = 0; w < width; w += channels) {
			h = p[w + 0];
			s = p[w + 1];
			v = p[w + 2];

			isBlue = false;
			//�ж������Ƿ�Ϊ��ɫ
			if (h>=100 && h<=124 &&
				s>=43  && s<=255 &&
				v>=46  && v<=255) {
				isBlue = true;
			}
			//����ɫ�����ȵ������������������
			if (isBlue) {
				p[w + 0] = 0;
				p[w + 1] = 0;
				p[w + 2] = 255;
			}
			//����ȫ����
			else {
				p[w + 0] = 0;
				p[w + 1] = 0;
				p[w + 2] = 0;
			}
		}
	}
	//imshow("hsv-��",hsv);
	//��ͨ�����з���
	vector<Mat> hsvSplit;
	split(hsv, hsvSplit);
	//imshow("hsv-����", hsvSplit[2]);
	//��ֵ��
	Mat shold;
	threshold(hsvSplit[2],shold,0,266,THRESH_OTSU + THRESH_BINARY);
	//��̬ѧ�ղ���
	//  ����ָ����С����״�ĽṹԪ����������̬����,�������Ϊ��̬ѧ�����и�ʴ���͵�ģ��
	//  p1->Size(17,3):Ԫ�ش�С�������Ǿ���ֵ
	Mat element = getStructuringElement(MORPH_RECT, Size(17, 3));
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
		drawRotatedRect(clone, rotateRect, Scalar(0, 0, 255));

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
	imshow("���ƶ�λ���ο�-����ɾѡ-hsv", clone);

	//	8.�жϽǶ� ������ɸѡ�ų��������ǳ��Ƶľ��Σ�
	//	9.��ת���� ����ƫб�ĳ��Ƶ���Ϊˮƽ��Ϊ����ĳ����ж����ַ�ʶ����߳ɹ��ʣ�
	//	10.������С ��ȷ����ѡ���Ƶ������ѧϰģ��֮ǰ�ߴ�һ�£�
	tortuosity(src, rotateRects, plateCandidates);
	for each (Mat mat in plateCandidates)
	{
		//imshow("hsv ��ѡ����",mat);
		//waitKey();
	}
	//waitKey();
}
