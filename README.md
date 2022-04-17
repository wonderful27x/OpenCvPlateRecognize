## 车牌识别
* **知识点**
	1. 特征
		* HAAR: 由黑白区域构成的模板，黑白区域像素和之差
		* LBP: 局部二值化，描述图像局部纹理，旋转不变性和灰度不变性	
		* HOG: 方向梯度直方图，图像重叠区域密集描述符，关注图像结构或形状，适合轮廓检测
	2. svm支持向量机
		* 定义在特征空间上的间隔最大的线性分类器。
		* 目标: 找出基于特征的分解函数
		* 核函数: 对于线性不可分，把低维向量映射到高维空间，从而找出一个分解超平面	
	3. ann人工神经网络-MLP前馈多层感知机
		* 输入层、隐含层、输出层，每层由多个神经元组成
		* 神经元: 输入、激活函数、输出
		* 对于基于HOG特征的神经网络，通常输入层的神经元数量等于HOG特征的维度，输出层的输出等于时别目标数，比如识别26个字母
	4. 模型训练
		* 基于HOG特征的车牌模型
		* 基于HOG特征的数字-字母-省份简称汉字模型
	5. 识别流程
		* 加载模型，SVM::load,ANN_MLP::load
		* 高斯模糊去噪
		* 灰度化cvtColor
		* Sobel边缘检测
		* 二值化
		* 腐蚀膨胀得到连通区域
		* 求轮廓得到外接矩形
		* 尺寸调整、角度旋转得到候选车牌列表 
		* 使用颜色空间HSV处理得到候选车牌列表2
		* 合并候选车牌列表
		* 提取候选车牌HOG特征送入SVM模型，选出可信度最高的
		* 抠出字符提取HOG特征，送入ANN模型，识别出字符
	

