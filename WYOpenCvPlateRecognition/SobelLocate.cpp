#include"SobelLocate.h"

SobelLocate::SobelLocate()
{
}

SobelLocate::~SobelLocate()
{
}

/*
sobel算法车牌定位
src：待识别原图
plateCandidates：定位的车牌候选
*/
void SobelLocate::locate(Mat src, vector<Mat>& plateCandidates)
{
	//  1.高斯模糊（去噪，为边缘检测算法做准备）
	//  p2->Size(5, 5)：高斯模糊半径，opencv中只支持基数半径，半径越大图片越模糊,这里的Size(5, 5)是一个经验值
	//  p3->0:X方向上的高斯核标准偏差。
	Mat blur;
	GaussianBlur(src, blur, Size(5, 5), 0);
	//imshow("高斯模糊", blur);

	//	2.灰度化 （为边缘检测算法准备灰度化环境）
	Mat gray;
	cvtColor(blur, gray, COLOR_BGR2GRAY);
	//imshow("灰度图", gray);

	//	3.sobel运算 （得到图像的一阶水平方向导数）
	//  p2->CV_16S：采用16位深度输出，防止溢出导致数据不完整(求导后的导数值可能小于0或者大于255，8位存不下)
	//  p3->1：对x求导
	Mat sobel_16;
	Sobel(gray, sobel_16, CV_16S, 1, 0);
	Mat sobel;
	//转回8位
	convertScaleAbs(sobel_16, sobel);
	//imshow("sobel", sobel);

	//	4.二值化 （对图像的每个像素做一个阈值处理，为后续的形态学操作准备。）
	//  正二值化：适合深底浅字
	//  反二值化：适合浅底深字
	//  p2->0:阈值取0，自适应
	//  p3->255：域值最大值
	//  p4->THRESH_OTSU + THRESH_BINARY:二值化算法
	Mat shold;
	threshold(sobel,shold,0,255, THRESH_OTSU + THRESH_BINARY);
	//imshow("二值化", shold);

	//	5.形态学操作 （闭操作：将车牌字符连接成一个连通区域，便于取轮廓）
	//  返回指定大小和形状的结构元素以用于形态运算,可以理解为形态学操作中腐蚀膨胀的模板
	//  p1->Size(17,3):元素大小，这里是经验值
	Mat element = getStructuringElement(MORPH_RECT,Size(17,3));
	Mat close;
	//  进行形态学操作，形态学运行的基础是腐蚀和膨胀
	//  p2->MORPH_CLOSE：闭操作（先膨胀后腐蚀）
	//  p3->element：形态学操作的模板
	morphologyEx(shold, close, MORPH_CLOSE, element);
	//imshow("close", close);

	//	6.求轮廓 （将连通域的外围画出来，便于形成外接矩形）
	// 定义一个集合contours用于接收外接矩形轮廓，轮廓是由线组成的，而线是由点组成的
	vector<vector<Point>> contours;
	findContours(
		close,              //输入图像 
		contours,           //输出轮廓集合
		RETR_EXTERNAL,      //取外接轮廓
		CHAIN_APPROX_NONE   //取轮廓上所有像素点
	);

	//	7.判断尺寸 （初步筛选排除不可能是车牌的矩形，中国车牌的一般大小是440mm * 140mm，宽高比为3.14）
	RotatedRect rotateRect;
	vector<RotatedRect> rotateRects;
	Mat clone = src.clone();
	for each (vector<Point> contour in contours)
	{
		//取最小外接矩形（可旋转/带角度）
		rotateRect = minAreaRect(contour);  

		////在原图上画旋转矩形最小外接矩形
		////rotateRect.boundingRect()：旋转矩形最小外接矩形
		//rectangle(src,rotateRect.boundingRect(),Scalar(0,0,255));

		//直接在原图上画出旋转矩形
		drawRotatedRect(clone,rotateRect,Scalar(0,0,255));

		//对尺寸面积进行初步筛选
		if (verifySizes(rotateRect)) {
			rotateRects.push_back(rotateRect);
		}
	}
	//imshow("出牌定位矩形框-未删选",src);
	for each (RotatedRect rect in rotateRects)
	{
		////在原图上画初步删选后的旋转矩形最小外接矩形
		//rectangle(src, rect.boundingRect(), Scalar(0, 255, 0));

		//直接在原图上画初步删选后的旋转矩形
		drawRotatedRect(clone, rect, Scalar(0, 255, 0));
	}
	imshow("出牌定位矩形框-初步删选-sobel", clone);

	//	8.判断角度 （初步筛选排除不可能是车牌的矩形）
	//	9.旋转矩形 （将偏斜的车牌调整为水平，为后面的车牌判断与字符识别提高成功率）
	//	10.调整大小 （确保候选车牌导入机器学习模型之前尺寸一致）
	tortuosity(src, rotateRects, plateCandidates);
	for each (Mat mat in plateCandidates)
	{
		//imshow("sobel 候选车牌",mat);
		//waitKey();
	}
	//waitKey();
}
