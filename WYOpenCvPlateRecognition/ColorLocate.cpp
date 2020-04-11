
#include"ColorLocate.h"

ColorLocate::ColorLocate()
{
}

ColorLocate::~ColorLocate()
{
}

/*
车牌定位,这里假定车牌是蓝色的
蓝色像素hsv空间
h:100-124
s:43-255
v:46-255
src：原图像
plateCandidates：输出候选车牌
*/
void ColorLocate::locate(Mat src, vector<Mat>& plateCandidates)
{
	//bgr转hsv
	Mat hsv;
	cvtColor(src, hsv, COLOR_BGR2HSV);
	//imshow("hsv", hsv);
	//保留蓝色分量，剔除其他分量
	int channels = hsv.channels();
	int width = hsv.cols * channels;
	int height = hsv.rows;
	uchar* p;
	int h, s, v;
	bool isBlue = false;
	//判断像素在内存是否连续存储，如果是可以优化以减少循环次数
	if (hsv.isContinuous()) {
		width = width * height;
		height = 1;
	}
	for (int h = 0; h < height; h++) {
		//指针指向行首
		p = hsv.ptr<uchar>(h);
		for (int w = 0; w < width; w += channels) {
			h = p[w + 0];
			s = p[w + 1];
			v = p[w + 2];

			isBlue = false;
			//判断像素是否为蓝色
			if (h>=100 && h<=124 &&
				s>=43  && s<=255 &&
				v>=46  && v<=255) {
				isBlue = true;
			}
			//是蓝色则将亮度调成最大，其他分量置零
			if (isBlue) {
				p[w + 0] = 0;
				p[w + 1] = 0;
				p[w + 2] = 255;
			}
			//否则全置零
			else {
				p[w + 0] = 0;
				p[w + 1] = 0;
				p[w + 2] = 0;
			}
		}
	}
	//imshow("hsv-蓝",hsv);
	//按通道进行分离
	vector<Mat> hsvSplit;
	split(hsv, hsvSplit);
	//imshow("hsv-分离", hsvSplit[2]);
	//二值化
	Mat shold;
	threshold(hsvSplit[2],shold,0,266,THRESH_OTSU + THRESH_BINARY);
	//形态学闭操作
	//  返回指定大小和形状的结构元素以用于形态运算,可以理解为形态学操作中腐蚀膨胀的模板
	//  p1->Size(17,3):元素大小，这里是经验值
	Mat element = getStructuringElement(MORPH_RECT, Size(17, 3));
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
		drawRotatedRect(clone, rotateRect, Scalar(0, 0, 255));

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
	imshow("出牌定位矩形框-初步删选-hsv", clone);

	//	8.判断角度 （初步筛选排除不可能是车牌的矩形）
	//	9.旋转矩形 （将偏斜的车牌调整为水平，为后面的车牌判断与字符识别提高成功率）
	//	10.调整大小 （确保候选车牌导入机器学习模型之前尺寸一致）
	tortuosity(src, rotateRects, plateCandidates);
	for each (Mat mat in plateCandidates)
	{
		//imshow("hsv 候选车牌",mat);
		//waitKey();
	}
	//waitKey();
}
