#include"PlateLocate.h"

PlateLocate::PlateLocate()
{
}

PlateLocate::~PlateLocate()
{
}

/**
* �ߴ�У��(��߱�&���)
*/
int PlateLocate::verifySizes(RotatedRect rotatedRect)
{
	//�ݴ���
	float error = 0.75f;
	//�����߱ȣ�136/36��ѵ�������ߴ磩
	float aspect = float(136) / float(36);
	//��ʵ��߱�
	float realAspect = float(rotatedRect.size.width) / float(rotatedRect.size.height);
	if (realAspect < 1) realAspect = (float)rotatedRect.size.height / (float)rotatedRect.size.width;
	//��ʵ���
	float area = rotatedRect.size.height * rotatedRect.size.width;
	//��С ������ �����ϵĶ���
	//������ž��� ��ʱ����
	//��������һЩû��ϵ�� �⻹�ǳ���ɸѡ��
	//�й����Ƶ�һ���С��440mm * 140mm
	//QUESTION ����Ϊʲô����д�����
	int areaMin = 44 * aspect * 14;
	int areaMax = 440 * aspect * 140;

	//�������� error��ΪҲ����
	//��С��߱�
	float aspectMin = aspect - aspect * error;
	//����߱�
	float aspectMax = aspect + aspect * error;

	if ((area < areaMin || area > areaMax) || (realAspect < aspectMin || realAspect > aspectMax))
		return 0;
	return 1;
}

/**
ת����ȫ���Σ���ֹ���ο򳬳�ͼ��߽�
ȡ������ʱ���п��ܿ����ͼ�񳬳���ԭͼ�񣬵��¾��ο��ڳ���һЩ������������ݣ�
������Ҫ������ȥ���ⲿ�ֳ���������
���Ǹ��˸о�����ת��û�б�Ҫ����Ϊ�������ֵ����ݶ�ͼ���ʶ��Ӧ�ò������Ӱ�죬
��������󷵻ػ������ķ���任��ͼ���ͼ�������㣡����

���safeRect�ƺ���Ȼ�����⣬����ͼƬplate1.jpg
*/
void PlateLocate::safeRect(Mat src, RotatedRect rect, Rect2f& safa_rect)
{
	//RotatedRect û������
	//תΪ�����Ĵ�����ı߿�
	Rect2f boudRect = rect.boundingRect2f();
	//���Ͻ� x,y
	float tl_x = boudRect.x > 0 ? boudRect.x : 0;
	float tl_y = boudRect.y > 0 ? boudRect.y : 0;
	//�������� ���� x��y ��0��ʼ�� ����-1
	//���������10��x���������9�� ����src.clos-1 
	//���½�
	float br_x = boudRect.x + boudRect.width < src.cols
		? boudRect.x + boudRect.width - 1
		: src.cols - 1;
	float br_y = boudRect.y + boudRect.height < src.rows
		? boudRect.y + boudRect.height - 1
		: src.rows - 1;
	float  w = br_x - tl_x;
	float h = br_y - tl_y;
	if (w <= 0 || h <= 0) return;
	safa_rect = Rect2f(tl_x, tl_y, w, h);
}

/**
* ��ת
*/
void PlateLocate::rotation(Mat src, Mat& dst, Size rect_size, Point2f center, double angle)
{
	//���÷���任
	Mat mat_rotated;
	//������ ��С�᲻һ�������ǶԽ��߿϶�������
	int max = sqrt(pow(src.rows, 2) + pow(src.cols, 2));
	//�����ת����/�任����
	Mat rot_mat = getRotationMatrix2D(center, angle, 1);
	//��ֹ�бߣ��Ա任�������ƽ��,ʹת����ͼ��������λ��
	rot_mat.at<double>(0, 2) += (max - src.cols) / 2;
	rot_mat.at<double>(1, 2) += (max - src.rows) / 2;
	warpAffine(
		src,           //����
		mat_rotated,   //���
		rot_mat,       //�任����
		Size(max, max),//��������С
		INTER_CUBIC    //һ�ַ���任�㷨
	);
	//��ȡ �����ѳ��ƶ���������ȡ��
	int width = rect_size.width > rect_size.height ? rect_size.width : rect_size.height;
	int height = rect_size.width > rect_size.height ? rect_size.height : rect_size.width;
	Point2f cutCenter = Point2f(mat_rotated.cols / 2, mat_rotated.rows / 2);
	getRectSubPix(
		mat_rotated,                             //����
		Size(width, height),                     //��Ҫ��ȡ�Ĵ�С
		cutCenter,                               //��ȡ���ĵ�
		dst                                      //���
	);
//	imshow("��תǰ", src);
//	imshow("��ת��", mat_rotated);
//	imshow("��ȡ��", dst);
//	waitKey();
	mat_rotated.release();
	rot_mat.release();
}

///**
//* ��ת
//*/
//void PlateLocate::rotation(Mat src, Mat& dst, Size rect_size, Point2f center, double angle)
//{
//	//�����ת����/�任����
//	Mat rot_mat = getRotationMatrix2D(center, angle, 1);
//	//���÷���任
//	Mat mat_rotated;
//	//������ ��С�᲻һ�������ǶԽ��߿϶�������
//	int max = sqrt(pow(src.rows, 2) + pow(src.cols, 2));
//	warpAffine(
//		src,           //����
//		mat_rotated,   //���
//		rot_mat,       //�任����
//		Size(max, max),//��������С
//		INTER_CUBIC    //һ�ַ���任�㷨
//	);
//	//��ȡ �����ѳ��ƶ���������ȡ��
//	getRectSubPix(
//		mat_rotated,                             //����
//		Size(rect_size.width, rect_size.height), //��Ҫ��ȡ�Ĵ�С
//		center,                                  //��ȡ���ĵ�
//		dst                                      //���
//	);
//	imshow("��תǰ", src);
//	imshow("��ת��", mat_rotated);
//	imshow("��ȡ��", dst);
//	waitKey();
//	mat_rotated.release();
//	rot_mat.release();
//}

/**
 ���ν���������任���ٳ����ƣ���һ����С)
 src����⳵��ԭͼ
 rotatedRects����������ɸѡ�Ŀ���ת���ƾ��ο�
 plateCandidates�������ĳ��ƺ�ѡ����
*/
void PlateLocate::tortuosity(Mat src, vector<RotatedRect>& rotatedRects, vector<Mat>& plateCandidates)
{
	//ѭ��Ҫ����ľ���
	for (RotatedRect rotatedRect : rotatedRects) {
		//���νǶ�
		float rotateAngle = rotatedRect.angle;
		float r = (float)rotatedRect.size.width / (float)rotatedRect.size.height;
		if (r < 1) {
			rotateAngle = 90 + rotateAngle;
		}
		//���δ�С
		Size rotatedRectSize = rotatedRect.size;
		//��rect��һ����ȫ�ķ�Χ(���ܳ���src)
		Rect2f  safa_rect;
		safeRect(src, rotatedRect, safa_rect);
		//��ѡ����
		//��ͼ  ���ﲻ�ǲ���һ����ͼƬ ������src���϶�λ��һ��Mat �����Ǵ���
		//���ݺ�src��ͬһ��
		Mat src_rect = src(safa_rect);
		//imshow("test", src_rect);
		//waitKey();
		//�����ĺ�ѡ����
		Mat dst;
		//����Ҫ��ת�� ��ת�Ƕ�Сû��Ҫ��ת��
		if (rotateAngle - 5 < 0 && rotateAngle + 5 > 0) {
			dst = src_rect.clone();
		}
		else {
			//�����roi�����ĵ� ����ȥ���Ͻ����������������ͼ��
			//��ȥ���Ͻ���������ں�ѡ���Ƶ����ĵ� ����
			Point2f rotatedRectCenter = rotatedRect.center - safa_rect.tl();
			Mat rotated_mat;
			//���� rotated_mat: �������ͼƬ
			rotation(src_rect, rotated_mat, rotatedRectSize, rotatedRectCenter, rotateAngle);
			dst = rotated_mat;
		}
		//������С
		Mat plate_mat;
		//��+��
		plate_mat.create(36, 136, CV_8UC3);//�������ߴ��й�
		resize(dst, plate_mat, plate_mat.size());
		plateCandidates.push_back(plate_mat);
		dst.release();
	}
}

//����ת����
void PlateLocate::drawRotatedRect(Mat dst,RotatedRect rotatedRect, Scalar scalar)
{
	Point2f points[4];
	//��ȡ��ת�����ĸ�����
	rotatedRect.points(points);
	//����
	for (int i = 0; i < 4; i++)
	{
		line(dst, points[i], points[(i + 1) % 4], scalar);
	}
}
