#include"PlateLocate.h"

PlateLocate::PlateLocate()
{
}

PlateLocate::~PlateLocate()
{
}

/**
* 尺寸校验(宽高比&面积)
*/
int PlateLocate::verifySizes(RotatedRect rotatedRect)
{
	//容错率
	float error = 0.75f;
	//理想宽高比（136/36：训练样本尺寸）
	float aspect = float(136) / float(36);
	//真实宽高比
	float realAspect = float(rotatedRect.size.width) / float(rotatedRect.size.height);
	if (realAspect < 1) realAspect = (float)rotatedRect.size.height / (float)rotatedRect.size.width;
	//真实面积
	float area = rotatedRect.size.height * rotatedRect.size.width;
	//最小 最大面积 不符合的丢弃
	//给个大概就行 随时调整
	//尽量给大一些没关系， 这还是初步筛选。
	//中国车牌的一般大小是440mm * 140mm
	//QUESTION 这里为什么这样写不理解
	int areaMin = 44 * aspect * 14;
	int areaMax = 440 * aspect * 140;

	//比例浮动 error认为也满足
	//最小宽高比
	float aspectMin = aspect - aspect * error;
	//最大宽高比
	float aspectMax = aspect + aspect * error;

	if ((area < areaMin || area > areaMax) || (realAspect < aspectMin || realAspect > aspectMax))
		return 0;
	return 1;
}

/**
转换安全矩形，防止矩形框超出图像边界
取轮廓的时候有可能框出的图像超出了原图像，导致矩形框内出现一些额外的像素数据，
所以需要做限制去掉这部分超出的像素
但是个人感觉这种转换没有必要，因为超出部分的数据对图像的识别应该不会造成影响，
这样处理后返回会给后面的仿射变换和图像截图带来不便！！！

这个safeRect似乎仍然有问题，测试图片plate1.jpg
*/
void PlateLocate::safeRect(Mat src, RotatedRect rect, Rect2f& safa_rect)
{
	//RotatedRect 没有坐标
	//转为正常的带坐标的边框
	Rect2f boudRect = rect.boundingRect2f();
	//左上角 x,y
	float tl_x = boudRect.x > 0 ? boudRect.x : 0;
	float tl_y = boudRect.y > 0 ? boudRect.y : 0;
	//这里是拿 坐标 x，y 从0开始的 所以-1
	//比如宽长度是10，x坐标最大是9， 所以src.clos-1 
	//右下角
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
* 旋转
*/
void PlateLocate::rotation(Mat src, Mat& dst, Size rect_size, Point2f center, double angle)
{
	//运用仿射变换
	Mat mat_rotated;
	//矫正后 大小会不一样，但是对角线肯定能容纳
	int max = sqrt(pow(src.rows, 2) + pow(src.cols, 2));
	//获得旋转矩阵/变换矩阵
	Mat rot_mat = getRotationMatrix2D(center, angle, 1);
	//防止切边，对变换矩阵进行平移,使转换后图像处于中心位置
	rot_mat.at<double>(0, 2) += (max - src.cols) / 2;
	rot_mat.at<double>(1, 2) += (max - src.rows) / 2;
	warpAffine(
		src,           //输入
		mat_rotated,   //输出
		rot_mat,       //变换矩阵
		Size(max, max),//输出矩阵大小
		INTER_CUBIC    //一种仿射变换算法
	);
	//截取 尽量把车牌多余的区域截取掉
	int width = rect_size.width > rect_size.height ? rect_size.width : rect_size.height;
	int height = rect_size.width > rect_size.height ? rect_size.height : rect_size.width;
	Point2f cutCenter = Point2f(mat_rotated.cols / 2, mat_rotated.rows / 2);
	getRectSubPix(
		mat_rotated,                             //输入
		Size(width, height),                     //需要截取的大小
		cutCenter,                               //截取中心点
		dst                                      //输出
	);
//	imshow("旋转前", src);
//	imshow("旋转后", mat_rotated);
//	imshow("截取后", dst);
//	waitKey();
	mat_rotated.release();
	rot_mat.release();
}

///**
//* 旋转
//*/
//void PlateLocate::rotation(Mat src, Mat& dst, Size rect_size, Point2f center, double angle)
//{
//	//获得旋转矩阵/变换矩阵
//	Mat rot_mat = getRotationMatrix2D(center, angle, 1);
//	//运用仿射变换
//	Mat mat_rotated;
//	//矫正后 大小会不一样，但是对角线肯定能容纳
//	int max = sqrt(pow(src.rows, 2) + pow(src.cols, 2));
//	warpAffine(
//		src,           //输入
//		mat_rotated,   //输出
//		rot_mat,       //变换矩阵
//		Size(max, max),//输出矩阵大小
//		INTER_CUBIC    //一种仿射变换算法
//	);
//	//截取 尽量把车牌多余的区域截取掉
//	getRectSubPix(
//		mat_rotated,                             //输入
//		Size(rect_size.width, rect_size.height), //需要截取的大小
//		center,                                  //截取中心点
//		dst                                      //输出
//	);
//	imshow("旋转前", src);
//	imshow("旋转后", mat_rotated);
//	imshow("截取后", dst);
//	waitKey();
//	mat_rotated.release();
//	rot_mat.release();
//}

/**
 矩形矫正（仿射变换，抠出车牌，归一化大小)
 src：检测车牌原图
 rotatedRects：经过初步筛选的可旋转车牌矩形框
 plateCandidates：处理后的车牌候选矩阵
*/
void PlateLocate::tortuosity(Mat src, vector<RotatedRect>& rotatedRects, vector<Mat>& plateCandidates)
{
	//循环要处理的矩形
	for (RotatedRect rotatedRect : rotatedRects) {
		//矩形角度
		float rotateAngle = rotatedRect.angle;
		float r = (float)rotatedRect.size.width / (float)rotatedRect.size.height;
		if (r < 1) {
			rotateAngle = 90 + rotateAngle;
		}
		//矩形大小
		Size rotatedRectSize = rotatedRect.size;
		//让rect在一个安全的范围(不能超过src)
		Rect2f  safa_rect;
		safeRect(src, rotatedRect, safa_rect);
		//候选车牌
		//抠图  这里不是产生一张新图片 而是在src身上定位到一个Mat 让我们处理
		//数据和src是同一份
		Mat src_rect = src(safa_rect);
		//imshow("test", src_rect);
		//waitKey();
		//真正的候选车牌
		Mat dst;
		//不需要旋转的 旋转角度小没必要旋转了
		if (rotateAngle - 5 < 0 && rotateAngle + 5 > 0) {
			dst = src_rect.clone();
		}
		else {
			//相对于roi的中心点 不减去左上角坐标是相对于整个图的
			//减去左上角则是相对于候选车牌的中心点 坐标
			Point2f rotatedRectCenter = rotatedRect.center - safa_rect.tl();
			Mat rotated_mat;
			//矫正 rotated_mat: 矫正后的图片
			rotation(src_rect, rotated_mat, rotatedRectSize, rotatedRectCenter, rotateAngle);
			dst = rotated_mat;
		}
		//调整大小
		Mat plate_mat;
		//高+宽
		plate_mat.create(36, 136, CV_8UC3);//跟样本尺寸有关
		resize(dst, plate_mat, plate_mat.size());
		plateCandidates.push_back(plate_mat);
		dst.release();
	}
}

//画旋转矩形
void PlateLocate::drawRotatedRect(Mat dst,RotatedRect rotatedRect, Scalar scalar)
{
	Point2f points[4];
	//获取旋转矩形四个顶点
	rotatedRect.points(points);
	//连线
	for (int i = 0; i < 4; i++)
	{
		line(dst, points[i], points[(i + 1) % 4], scalar);
	}
}
