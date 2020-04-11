// WYOpenCvPlateRecognition.cpp: 定义应用程序的入口点。
//

#include "WYOpenCvPlateRecognition.h"

/*
这是一个使用openCv进行车牌识别的项目
车牌识别一共有三大步骤：
（一）.定位：而定位又分为10步。https://www.jianshu.com/p/8ed28846ac9a
	1.高斯模糊（去噪，为边缘检测算法做准备）
	2.灰度化 （为边缘检测算法准备灰度化环境）
	3.sobel运算 （得到图像的一阶水平方向导数）
	4.二值化 （对图像的每个像素做一个阈值处理，为后续的形态学操作准备。）
	5.形态学操作 （将车牌字符连接成一个连通区域，便于取轮廓）
	6.求轮廓 （将连通域的外围画出来，便于形成外接矩形）
	7.判断尺寸 （初步筛选排除不可能是车牌的矩形，中国车牌的一般大小是440mm*140mm，宽高比为3.14）
	8.判断角度 （初步筛选排除不可能是车牌的矩形）
	9.旋转矩形 （将偏斜的车牌调整为水平，为后面的车牌判断与字符识别提高成功率）
	10.调整大小 （确保候选车牌导入机器学习模型之前尺寸一致）
	注意：上面定位算法步骤是以sobel算法为例来说的，还有颜色空间HSV算法，文字算法等，不同的算法稍有区别
（二）.检测：
（三）.识别：

注意：整个项目没有对内存进行释放，需要优化
*/
int main()
{
	//svm模型
	string svmModelPath = "E:/VisualStudio/FILE/svm/plate/model/plateSvm.xml";
	//ann字母+数字模型
	string annCharModel = "E:/VisualStudio/FILE/ann/plateNum/model/annChar.xml";
	//ann汉字模型
	string annChineseModel = "E:/VisualStudio/FILE/ann/plateNum/model/annChinese.xml";
	//车牌识别封装类
	PlateRecognize plateRecognize(svmModelPath, annCharModel, annChineseModel);
	//车牌图片路径
	string matPath = "E:/VisualStudio/FILE/pictures/plate2.jpg";
	//读入矩阵
	Mat src = imread(matPath);
	//归一化大小
	//resize(src, src, Size(500, 400));
	//开始识别并返回车牌字符串
	string plate = plateRecognize.plateRecognize(src);
	cout << "车牌："<< plate << endl;
	//imshow("车牌原图", src);
	waitKey();
	return 0;
}
