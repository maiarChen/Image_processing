#include<iostream>
#include<opencv2/opencv.hpp>  
#include"opencv2/stitching/stitcher.hpp"
#include"ImageProcessing.h"

using namespace std;
using namespace cv;



//构造函数colorHandle
ImageProcessing::ImageProcessing(Mat Img) {
	srcImg = Img;
}



/* 颜色空间缩减-灰度化-二值化 */
//cutColor函数用于缩减颜色空间  --right--
Mat ImageProcessing::cutColor(int n)
{
	int channels = srcImg.channels();
	int nRows = srcImg.rows;
	int nCols = srcImg.cols* channels;
	if (srcImg.isContinuous())
	{
		nCols *= nRows;
		nRows = 1;
	}
	Mat dstImg= srcImg.clone();
	int i, j;
	for (i = 0; i<nRows; ++i) {
		for (j = 0; j<nCols; ++j) {
			int value = srcImg.ptr(i)[j];
			dstImg.ptr(i)[j] = n * (value / n);
		}
	}
	//得到颜色空间缩减后的图像
	return dstImg;
}
//rgb2gray函数用于将rgb图像转换为灰度图像  --right--
Mat ImageProcessing::rgb2gray()
{
	cv::cvtColor(srcImg, srcImg, CV_RGB2GRAY);
	//得到灰度化之后的图
	return srcImg;
}
//rgb2black函数用于将rgb图像转换黑白图像  --right--
Mat ImageProcessing::rgb2black(int num)
{
	int threNum = num;
	cv::cvtColor(srcImg, srcImg, CV_RGB2GRAY);
	threshold(srcImg, srcImg, threNum, 255, THRESH_BINARY);
	//得到二值化之后的图像
	return srcImg;
}
/* 颜色空间缩减-灰度化-二值化 */



/* 线性-非线性灰度变换 */
//linearGrayScaleTransformation函数用于进行线性灰度变换，使用时需要传入两个参数用于调整contrastValue(对比度)/brightValue(亮度)  --no-- srcImg改为dstImg
Mat ImageProcessing::linearGrayScaleTransformation(int contrastValue, int brightValue) {
	//创建窗口
	//Mat srcImgClone = srcImg.clone();
	//srcImgClone = Mat::zeros(srcImg.size(), srcImg.type());
	// 三个for循环，执行运算 g_dstImage(i,j) = a*g_srcImage(i,j) + b 
	//实现彩色图像的线性灰度变换

	//if (contrastValue == 0) {
	//	contrastValue = 1;
	//}
	//if (brightValue == 0) {
	//	brightValue = 1;
	//}

	Mat dstImg = srcImg.clone();

	for (int x = 0; x < srcImg.rows; x++)
	{
		for (int y = 0; y < srcImg.cols; y++)
		{
			for (int c = 0; c < 3; c++)
			{
				dstImg.at<Vec3b>(x, y)[c] = saturate_cast<uchar>((contrastValue*0.01)*(srcImg.at<Vec3b>(x, y)[c]) + brightValue);
			}
		}
	}
	return dstImg;
}
//pieceWiselinearGrayScaleTransformation函数用于进行分段线性灰度变换，使用时需要传入两个点的坐标共四个参数用于进行调整  --right--
Mat ImageProcessing::pieceWiselinearGrayScaleTransformation(double X_1, double Y_1, double X_2, double Y_2) {
	int channels = srcImg.channels();
	int nRows = srcImg.rows;
	int nCols = srcImg.cols* channels;
	if (srcImg.isContinuous())
	{
		nCols *= nRows;
		nRows = 1;
	}
	uchar* p;
	int i, j;
	Mat dstImg = srcImg.clone();
	double intercept_1, intercept_2, intercept_3; //斜率

	intercept_1 = (Y_1 / X_1);
	intercept_2 = (Y_2 - Y_1) / (X_2 - X_1);
	intercept_3 = (255 - Y_2) / (255 - X_2);

	uchar table[256];
	for (int k = 0; k<X_1; k++) {
		table[k] = intercept_1 * k;
	}
	for (int k = X_1; k<X_2; k++) {
		table[k] = intercept_2 * (k - X_1) + Y_1;
	}
	for (int k = X_2; k<256; k++) {
		table[k] = intercept_3 * (k - X_2) + Y_2;
	}

	for (i = 0; i<nRows; i++) {
		p = dstImg.ptr<uchar>(i);
		for (j = 0; j<nCols; j++) {
			p[j] = table[p[j]];
		}
	}
	return dstImg;
}
//loglinearGrayScaleTransformation函数用于进行非线性-对数灰度变换，使用时需要传入常数c  --right--
Mat ImageProcessing::loglinearGrayScaleTransformation(int c) {
	//uchar* p;
	int channels = srcImg.channels();
	int nRows = srcImg.rows;
	int nCols = srcImg.cols* channels;
	if (srcImg.isContinuous())
	{
		nCols *= nRows;
		nRows = 1;
	}
	Mat dstImg = srcImg.clone();
	double g = 0;
	for (int x = 0; x < nRows; x++) {
		for (int y = 0; y < nCols; y++) {
			g = srcImg.ptr(x)[y];
			g = c*log((double)(1 + g));
			dstImg.ptr(x)[y] = g;
		}
	}

	return dstImg;
}
//powerLawlinearGrayScaleTransformation函数用于进行非线性-幂律灰度变换，使用时需要传入常数k  --right--
Mat ImageProcessing::powerLawlinearGrayScaleTransformation(double index) {
	int channels = srcImg.channels();
	int nRows = srcImg.rows;
	int nCols = srcImg.cols* channels;
	if (srcImg.isContinuous())
	{
		nCols *= nRows;
		nRows = 1;
	}
	Mat dstImg = srcImg.clone();
	uchar* p;
	double g = 0;
	for (int x = 0; x < nRows; x++) {
		for (int y = 0; y < nCols; y++) {
			g = srcImg.ptr(x)[y];
			g = saturate_cast<uchar>(pow((double)g/255., index)*255.0f);
			dstImg.ptr(x)[y] = g;
		}
	}
	return dstImg;
}
/* 线性-非线性灰度变换 */



/* 滤波器 */
//blurFilter函数用于进行均值滤波处理  --right--
Mat ImageProcessing::blurFilter(int width, int height) {
	blur(srcImg, filterResImage, Size(width, height), Point(-1, -1), BORDER_DEFAULT);
	return filterResImage;
}
//gaussianBlurFilter函数用于进行高斯滤波处理  --right--
Mat ImageProcessing::gaussianBlurFilter(int width, int height) {
	GaussianBlur(srcImg, filterResImage, Size(width, height), 0, 0, BORDER_DEFAULT);
	return filterResImage;
}
//medianBlurFilter函数用于进行中值滤波处理，使用时需传入常数kSize作为滤波模板的尺寸大小，必须是大于1的奇数  --right--
Mat ImageProcessing::medianBlurFilter(int kSize) {
	medianBlur(srcImg, filterResImage, kSize);
	return filterResImage;
}
//fil2DLapLace函数用于进行拉普拉斯滤波处理  --right--
Mat ImageProcessing::fil2DLaplace(int lapSize) {
	int laplace_Size = lapSize;
	if (laplace_Size % 2 == 0) {
		laplace_Size = laplace_Size + 1;
	}

	Mat kernel(laplace_Size, laplace_Size, CV_32FC1);
	for (int m = 0; m < laplace_Size*laplace_Size; m++) {
		if (m == ((laplace_Size*laplace_Size) / 2 + 1)) {
			kernel.at<float>(m) = -(laplace_Size*laplace_Size);
		}
		else {
			kernel.at<float>(m) = 1;
		}
	}
	filter2D(srcImg, filterResImage, -1, kernel, Point(-1, -1), 0, BORDER_DEFAULT);
	return filterResImage;
}
/* 滤波器 */



/* DCT变换 */
//dctTransformation函数用于进行dct变换处理  --right--
Mat ImageProcessing::dctTransformation(double t) {

	double T;
	T = t;
	int h = srcImg.rows;
	int w = srcImg.cols;
	if (srcImg.rows % 2 != 0 || srcImg.cols % 2 != 0) {
		copyMakeBorder(srcImg, srcImg, 0, srcImg.rows % 2, 0, srcImg.cols % 2, BORDER_CONSTANT, Scalar::all(0));
	}

	//从BGR空间转换到YUV空间     
	Mat yuvimg(srcImg.size(), CV_8UC3);
	//定义YUV空间图像yuvimg
	cvtColor(srcImg, yuvimg, CV_BGR2YUV); 
	//定义输出图像为dst
	Mat dst(srcImg.size(), CV_8UC3);        				    
	vector<Mat> channels(3);
	//YUV通道分离
	split(yuvimg, channels);
	//提取YUV颜色空间各通道      
	Mat_<double>Y = channels.at(0);
	Mat_<double>U = channels.at(1);
	Mat_<double>V = channels.at(2);
	//DCT系数的三个通道    
	Mat DCTY(srcImg.size(), CV_64FC1);
	Mat DCTU(srcImg.size(), CV_64FC1);
	Mat DCTV(srcImg.size(), CV_64FC1);
	//DCT变换    
	dct(Y, DCTY);
	dct(U, DCTU);
	dct(V, DCTV);
	//三通道，作阈值处理，进行压缩      
	for (int i = 0; i < h; i++)
	{
		double *y = DCTY.ptr<double>(i);
		double *u = DCTU.ptr<double>(i);
		double *v = DCTV.ptr<double>(i);
		for (int j = 0; j < w; j++)
		{
			if (abs(y[j]) < T) { y[j] = 0; }
			if (abs(u[j]) < T) { u[j] = 0; }
			if (abs(v[j]) < T) { v[j] = 0; }

		}
	}
	//DCT逆变换    
	idct(DCTY, Y);
	idct(DCTU, U);
	idct(DCTV, V);
	channels.at(0) = Mat_<uchar>(Y);
	channels.at(1) = Mat_<uchar>(U);
	channels.at(2) = Mat_<uchar>(V);
	merge(channels, yuvimg);
	//将压缩后图像从YUV空间重新转换到BGR空间    
	cvtColor(yuvimg, dst, CV_YUV2BGR);
	return dst;
}
/* DCT变换 */



/* DFT变换 */
//dftTransformation函数用于对图像进行离散傅里叶变换处理
Mat ImageProcessing::dftTransformation() {
	Mat img = srcImg;
	cvtColor(img, img, CV_RGB2GRAY);
	//判断并拓展图像
	int h = getOptimalDFTSize(img.rows);//获取原图长
	int w = getOptimalDFTSize(img.cols);//获取原图宽
	Mat img_pad; // 拓展后的图像
	if (w > img.cols || h > img.rows)
		copyMakeBorder(img, img_pad, 0, h - img.rows, 0, w - img.cols, BORDER_CONSTANT, Scalar::all(0));
	else {
		img.copyTo(img_pad);
	}

	//谱的中心化
	img_pad.convertTo(img_pad, CV_32FC1);
	for (int i = 0; i<img_pad.rows; i++)
	{
		float *ptr = img_pad.ptr<float>(i);
		for (int j = 0; j < img_pad.cols; j++)
			ptr[j] *= pow(-1.0, i + j);
	}
	Mat plane[] = { img_pad, Mat::zeros(img_pad.size(), CV_32F) };
	Mat complexImg;
	merge(plane, 2, complexImg);

	dft(complexImg, complexImg);//离散傅里叶变换

	ShowSpectrum(complexImg, "原图傅里叶频谱");

	return complexImg;
}
//ShowSpectrum函数用于显示dft变换后的频谱图
void ImageProcessing::ShowSpectrum(Mat input, string caption) {
	Mat plane[2];
	split(input, plane);
	magnitude(plane[0], plane[1], plane[0]);
	plane[0] += Scalar::all(1);
	log(plane[0], plane[0]);
	normalize(plane[0], plane[0], 0, 1, CV_MINMAX);
	cvNamedWindow(caption.c_str(), CV_WINDOW_AUTOSIZE);
	imshow(caption.c_str(), plane[0]);
}
//ShowImage函数用于将进行高/低通滤波过后的频域图像转换为能够正常显示的图像
Mat ImageProcessing::ShowImage(Mat input, string caption) {
	Mat plane[2];
	split(input, plane);
	for (int i = 0; i<plane[0].rows; i++)
	{
		float *ptr = plane[0].ptr<float>(i);
		for (int j = 0; j < plane[0].cols; j++)
			ptr[j] *= pow(-1, i + j);
	}
	normalize(plane[0], plane[0], 0, 1, CV_MINMAX);//归一化

	for (int i = 0; i<plane[0].rows; i++)//遍历乘以255
	{
		float *ptr = plane[0].ptr<float>(i);
		for (int j = 0; j < plane[0].cols; j++)
			ptr[j] *= 255;
	}
	Mat tmp = plane[0];
	plane[0].convertTo(tmp, CV_8UC1);
	
	return tmp;
}
//gausHighFilter函数用于进行高斯高通滤波处理
Mat ImageProcessing::gausHighLowFilter(Mat complexImg,float d0, bool flag) {
	Mat GaussianHighLowFilter = complexImg.clone();

	Mat GaussianKernel(GaussianHighLowFilter.size(), CV_32FC2);//高斯滤波器模板
	float D0 = 2 * pow(double(d0), 2.0);//d0的平方
	for (int i = 0; i < GaussianHighLowFilter.rows; i++) {
		float*p = GaussianKernel.ptr<float>(i);
		for (int j = 0; j < GaussianHighLowFilter.cols; j++) {
			float d2 = pow(i - GaussianHighLowFilter.rows / 2, 2.0) + pow(j - GaussianHighLowFilter.cols / 2, 2.0);//（u，v）离中心点距离
			if (flag) {//低通
				p[2 * j] = expf(-d2 / D0);
				p[2 * j + 1] = expf(-d2 / D0);
			}
			else {//高通
				p[2 * j] = 1 - expf(-d2 / D0);
				p[2 * j + 1] = 1 - expf(-d2 / D0);
			}
		}
	}

	Mat gausHighFilterImage;
	Mat gausLowFilterImage;

	Mat outImage;

	if (flag) {//低通
		gausLowFilterImage = GaussianHighLowFilter;
		//ShowSpectrum(GaussianKernel, "高斯模板");
		multiply(gausLowFilterImage, GaussianKernel, gausLowFilterImage);//矩阵相乘，输出到input
		//ShowSpectrum(gausLowFilterImage, "高斯低通傅里叶频谱");
		idft(gausLowFilterImage, complexImg);//高斯低通滤波后傅里叶逆变换
		outImage=ShowImage(complexImg, "高斯高通后图像");
	}
	else {//高通
		gausHighFilterImage = GaussianHighLowFilter;
		//ShowSpectrum(GaussianKernel, "高斯模板");
		multiply(gausHighFilterImage, GaussianKernel, gausHighFilterImage);//矩阵相乘，输出到input
		//ShowSpectrum(gausHighFilterImage, "高斯高通傅里叶频谱");
		idft(gausHighFilterImage, complexImg);//高斯高通滤波后傅里叶逆变换
		outImage = ShowImage(complexImg, "高斯高通后图像");
	}
	//int a = complexImg.channels();
	return outImage;
}
//IdealHighLowFilter函数用于进行理想滤波处理
Mat ImageProcessing::IdealHighLowFilter(Mat complexImg, float d0, bool flag) {
	Mat IdealHighLowFilter = complexImg.clone();
	Mat IdealKernel(IdealHighLowFilter.size(), CV_32FC2);//理想滤波器模板
	for (int i = 0; i < IdealHighLowFilter.rows; i++) {
		float*p = IdealKernel.ptr<float>(i);
		for (int j = 0; j < IdealHighLowFilter.cols; j++) {
			float d = sqrt(pow(i - IdealHighLowFilter.rows / 2, 2.0) + pow(j - IdealHighLowFilter.cols / 2, 2.0));//（u，v）离中心点距离
			if (flag) {//低通
				if (d <= d0) {
					//小于d0的都为1
					p[2 * j] = 1;
					p[2 * j + 1] = 1;
				}else{
					//大于d0的都为0
					p[2 * j] = 0;
					p[2 * j + 1] = 0;
				}

			}
			else {//高通
				if (d <= d0) {
					//小于d0的都为0
					p[2 * j] = 0;
					p[2 * j + 1] = 0;
				}
				else{
					//大于d0的都为1
					p[2 * j] = 1;
					p[2 * j + 1] = 1;
				}
			}
		}
	}

	Mat IdealHighFilterImage;
	Mat IdealLowFilterImage;

	Mat outImage;

	if (flag) {//低通
		IdealLowFilterImage = IdealHighLowFilter;
		//ShowSpectrum(GaussianKernel, "理想模板");
		multiply(IdealLowFilterImage, IdealKernel, IdealLowFilterImage);//矩阵相乘，输出到input
		//ShowSpectrum(gausLowFilterImage, "理想低通傅里叶频谱");
		idft(IdealLowFilterImage, complexImg);//理想低通滤波后傅里叶逆变换
		outImage = ShowImage(complexImg, "理想低通后图像");
	}
	else {//高通
		IdealHighFilterImage = IdealHighLowFilter;
		//ShowSpectrum(GaussianKernel, "理想模板");
		multiply(IdealHighFilterImage, IdealKernel, IdealHighFilterImage);//矩阵相乘，输出到input
		//ShowSpectrum(gausHighFilterImage, "理想高通傅里叶频谱");
		idft(IdealHighFilterImage, complexImg);//理想高通滤波后傅里叶逆变换
		outImage = ShowImage(complexImg, "理想高通后图像");
	}
	
	return outImage;
}
//ButterworthHighLowFilter函数用于进行巴特沃斯滤波处理
Mat ImageProcessing::ButterworthHighLowFilter(Mat complexImg, float d0, float n, bool flag) {
	Mat ButterworthHighLowFilter = complexImg.clone();
	Mat ButterworthKernel(ButterworthHighLowFilter.size(), CV_32FC2);//巴特沃斯滤波器模板
	for (int i = 0; i < ButterworthHighLowFilter.rows; i++) {
		float*p = ButterworthKernel.ptr<float>(i);
		for (int j = 0; j < ButterworthHighLowFilter.cols; j++) {
			float d = sqrt(pow(i - ButterworthHighLowFilter.rows / 2, 2.0) + pow(j - ButterworthHighLowFilter.cols / 2, 2.0));//（u，v）离中心点距离
			if (flag) {//低通
				p[2 * j] = 1.0 / (1 + pow((d / d0), 2 * n));
				p[2 * j + 1] = 1.0 / (1 + pow((d / d0), 2 * n));
			}
			else {//高通
				p[2 * j] = 1 - (1.0 / (1 + pow((d / d0), 2 * n)));
				p[2 * j + 1] = 1 - (1.0 / (1 + pow((d / d0), 2 * n)));
			}
		}
	}

	Mat ButterworthHighFilterImage;
	Mat ButterworthLowFilterImage;

	Mat outImage;

	if (flag) {//低通
		ButterworthLowFilterImage = ButterworthHighLowFilter;
		//ShowSpectrum(GaussianKernel, "巴特沃斯模板");
		multiply(ButterworthHighLowFilter, ButterworthKernel, ButterworthHighLowFilter);//矩阵相乘，输出到input
		//ShowSpectrum(gausLowFilterImage, "巴特沃斯低通傅里叶频谱");
		idft(ButterworthLowFilterImage, complexImg);//巴特沃斯低通滤波后傅里叶逆变换
		outImage = ShowImage(complexImg, "巴特沃斯低通后图像");
	}
	else {//高通
		ButterworthHighFilterImage = ButterworthHighLowFilter;
		//ShowSpectrum(GaussianKernel, "巴特沃斯模板");
		multiply(ButterworthHighLowFilter, ButterworthKernel, ButterworthHighLowFilter);//矩阵相乘，输出到input
		//ShowSpectrum(gausHighFilterImage, "巴特沃斯高通傅里叶频谱");
		idft(ButterworthHighFilterImage, complexImg);//巴特沃斯高通滤波后傅里叶逆变换
		outImage = ShowImage(complexImg, "巴特沃斯高通后图像");
	}

	return outImage;
}
/* DFT变换 */



/* 图像拼接 */  
//motage函数用于进行图像拼接，使用时需要传入四张图片的文件名  --pause--
Mat ImageProcessing::montage(string picName1, string picName2, string picName3, string picName4) {

	bool try_use_gpu = false;
	vector<Mat> imgs;
	string resultName = "result.jpg";

	Mat img1, img2, img3, img4;
	//push_back是算法里面的一个函数名。在vector类中作用为在vector尾部加入一个数据。
	img1 = imread(picName1, 1);
	imgs.push_back(img1);
	img2 = imread(picName2, 1);
	imgs.push_back(img2);
	img3 = imread(picName3, 1);
	imgs.push_back(img3);
	img4 = imread(picName4, 1);
	imgs.push_back(img4);

	Mat pano;
	//调用createDefault函数生成默认的参数
	Stitcher stitcher = Stitcher::createDefault(try_use_gpu);
	//使用stitcher函数进行拼接
	Stitcher::Status status = stitcher.stitch(imgs, pano);

	if (status != Stitcher::OK)
	{
		cout << "Can't stitch images, error code = " << int(status) << endl;
	}

	//保存结果图像
	imshow("resImg", pano);

	return pano;

	imwrite(resultName, pano);
}
/* 图像拼接 */



/* 直方图显示 */
//picHistogramRChannelShow函数用于建立图像的r通道直方图并进行显示  --right--
Mat ImageProcessing::picHistogramRChannelShow() {
	//Mat srcImg = imread(filename, CV_LOAD_IMAGE_COLOR);
	//src是要分离通道的对象
	vector<Mat> allChannels(srcImg.channels());

	split(srcImg, allChannels);

	//计算直方图calcHist()

	r_Channel = allChannels[2];

	MatND dstHist_0;
	int dims = 1;	//直方图的维数
	float hranges[] = { 0,255 };
	const float* ranges[] = { hranges };	//所需直方图的每一维的范围
	int size = 256;		//在每一维上直方图的个数
	int channels = 0;		//用来计算直方图的channels的数组
	calcHist(&r_Channel, 1, &channels, Mat(), dstHist_0, dims, &size, ranges);

	//绘制直方图minMaxLoc()
	int scale = 1;
	Mat rChannelsRes(size, size* scale, CV_8UC3, Scalar(0));

	//获取最大值和最小值
	double minValue = 0;
	double maxValue = 0;
	minMaxLoc(dstHist_0, &minValue, &maxValue, 0, 0);

	//绘制出直方图
	int hpt = saturate_cast<int>(0.9*size);
	for (int i = 0; i<256; i++) {
		float binValue = dstHist_0.at<float>(i);
		int realValue = saturate_cast<int>(binValue*hpt / maxValue);
		rectangle(rChannelsRes, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue), Scalar(0, 0, 255));
	}

	rChannelsSave = rChannelsRes;

	imshow("r直方图", rChannelsRes);
	waitKey(2000);

	return rChannelsRes;
}
//picHistogramGChannelShow函数用于建立图像的g通道直方图并进行显示  --right--
Mat ImageProcessing::picHistogramGChannelShow() {
	//Mat srcImg = imread(filename, CV_LOAD_IMAGE_COLOR);
	//src是要分离通道的对象
	vector<Mat> allChannels(srcImg.channels());

	split(srcImg, allChannels);

	//计算直方图calcHist()

	g_Channel = allChannels[1];

	MatND dstHist_0;
	int dims = 1;	//直方图的维数
	float hranges[] = { 0,255 };
	const float* ranges[] = { hranges };	//所需直方图的每一维的范围
	int size = 256;		//在每一维上直方图的个数
	int channels = 0;		//用来计算直方图的channels的数组
	calcHist(&g_Channel, 1, &channels, Mat(), dstHist_0, dims, &size, ranges);

	//绘制直方图minMaxLoc()
	int scale = 1;
	Mat gChannelsRes(size, size* scale, CV_8UC3, Scalar(0));

	//获取最大值和最小值
	double minValue = 0;
	double maxValue = 0;
	minMaxLoc(dstHist_0, &minValue, &maxValue, 0, 0);

	//绘制出直方图
	int hpt = saturate_cast<int>(0.9*size);
	for (int i = 0; i<256; i++) {
		float binValue = dstHist_0.at<float>(i);
		int realValue = saturate_cast<int>(binValue*hpt / maxValue);
		rectangle(gChannelsRes, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue), Scalar(0, 255, 0));
	}

	gChannelsSave = gChannelsRes;

	imshow("g直方图", gChannelsRes);
	waitKey(2000);

	return gChannelsRes;
}
//picHistogramBChannelShow函数用于建立图像的b通道直方图并进行显示  --right--
Mat ImageProcessing::picHistogramBChannelShow() {
	//Mat srcImg = imread(filename, CV_LOAD_IMAGE_COLOR);
	//src是要分离通道的对象
	vector<Mat> allChannels(srcImg.channels());

	split(srcImg, allChannels);

	//计算直方图calcHist()

	b_Channel = allChannels[0];

	MatND dstHist_0;
	int dims = 1;	//直方图的维数
	float hranges[] = { 0,255 };
	const float* ranges[] = { hranges };	//所需直方图的每一维的范围
	int size = 256;		//在每一维上直方图的个数
	int channels = 0;		//用来计算直方图的channels的数组
	calcHist(&b_Channel, 1, &channels, Mat(), dstHist_0, dims, &size, ranges);

	//绘制直方图minMaxLoc()
	int scale = 1;
	Mat bChannelsRes(size, size* scale, CV_8UC3, Scalar(0));

	//获取最大值和最小值
	double minValue = 0;
	double maxValue = 0;
	minMaxLoc(dstHist_0, &minValue, &maxValue, 0, 0);

	//绘制出直方图
	int hpt = saturate_cast<int>(0.9*size);
	for (int i = 0; i<256; i++) {
		float binValue = dstHist_0.at<float>(i);
		int realValue = saturate_cast<int>(binValue*hpt / maxValue);
		rectangle(bChannelsRes, Point(i*scale, size - 1), Point((i + 1)*scale - 1, size - realValue), Scalar(255, 0, 0));
	}

	bChannelsSave = bChannelsRes;

	imshow("b直方图", bChannelsRes);
	waitKey(2000);

	return bChannelsRes;
}
//picSrcChannelsSave函数存储三个通道的直方图
Mat ImageProcessing::picSrcChannelsSave() {

	vector<Mat> getAllChannels(srcImg.channels());

	split(srcImg, getAllChannels);

	r_Channel = getAllChannels[0];
	g_Channel = getAllChannels[1];
	b_Channel = getAllChannels[2];

	equalizeHist(r_Channel, r_Channel);
	equalizeHist(g_Channel, g_Channel);
	equalizeHist(b_Channel, b_Channel);

	Mat channelsMerge;
	merge(getAllChannels, channelsMerge);

	return channelsMerge;
}
/* 直方图显示 */