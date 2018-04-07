﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include"Mat2QImage.h"
#include<QMessageBox>
#include"Binary.h"
#include"LinearGrayScale.h"
#include"PieceWiselinearGrayScale.h"
#include "CutColor.h"
#include"LoglinearGrayScale.h"
#include"ImageProcessing.h"
#include"PowerLawGrayScale.h"
#include"EdgeDetection.h"
#include"Histogram.h"
#include"FreqFilter.h"

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent)
{
	ui = new Ui::MainWindow;
	qRegisterMetaType<Mat>("Mat");//注册mat在信号槽中可传递
	setWindowState(Qt::WindowMaximized);//可最大化
	ui->setupUi(this);
	ui->menu_image->setEnabled(false);
	ui->menu_video->setEnabled(false);
	save_off();
	
	ui->actionopen->setShortcuts(QKeySequence::Open);
	ui->actionsave->setShortcuts(QKeySequence::Save);
	ui->actionsaveas->setShortcuts(QKeySequence::SaveAs);

	//信号槽连接
	connect(ui->actionopen, &QAction::triggered, this, &MainWindow::open);
	connect(ui->actionsave, &QAction::triggered, this, &MainWindow::save);
	connect(ui->actionsaveas, &QAction::triggered, this, &MainWindow::saveAs);
	connect(ui->clearFormer, &QPushButton::clicked, this, &MainWindow::clearFormer);
	connect(ui->clearResult, &QPushButton::clicked, this, &MainWindow::clearResult);

	connect(ui->action_Gray, &QAction::triggered, this, &MainWindow::gray);//灰度化
	connect(ui->action_LinearGray, &QAction::triggered, this, &MainWindow::linearGraySolt);//线性灰度变换
	connect(ui->action_PieceWiselinearGray, &QAction::triggered, this, &MainWindow::pieceWiselinearGraySolt);//分段线性灰度变换
	connect(ui->action_LoglinearGray, &QAction::triggered, this, &MainWindow::loglinearGrayScaleSolt);//对数变换
	connect(ui->action_PowerLawlinearGray, &QAction::triggered, this, &MainWindow::powerLawGrayScaleSolt);//幂率变换

	connect(ui->action_CutColor, &QAction::triggered, this, &MainWindow::cutColorSolt);//颜色空间缩减
	connect(ui->action_Hist, &QAction::triggered, this, &MainWindow::HistogramSolt);//直方图

	connect(ui->action_Binary, &QAction::triggered, this, &MainWindow::binarySolt);//二值化
	connect(ui->action_Sobel, &QAction::triggered, this, &MainWindow::EdgeDetectionSolt);//边缘检测
	connect(ui->action_FreqFilter, &QAction::triggered, this, &MainWindow::freqFilter);//频域滤波
}

MainWindow::~MainWindow()
{
	delete ui;
}

//打开图像
void MainWindow::open()
{
	//调用窗口打开文件
	filename = QFileDialog::getOpenFileName(this,
		tr("open picture"),
		".",
		tr("Image file(*.png *.jpg *.bmp)"));
	
	if (!filename.isEmpty()) {
		srcImage.release();
		//文件路径不为空,用一个opencv Mat全局变量接收图像
		srcImage = imread(filename.toLocal8Bit().data());
		formerScene.clear();//清空原图面板

		//QImage加载成功则展示
		if (srcQimage.load(filename)) {
			ui->menu_image->setEnabled(true);
			QGraphicsScene *fScene =&formerScene;
			fScene->addPixmap(QPixmap::fromImage(srcQimage));
			ui->formerView->setScene(fScene);
			ui->formerView->show();
		}
		else {
			QMessageBox::information(this,
				tr("failed"),
				tr("open picture failed!"));
			return;
		}
	}
}

//保存
void MainWindow::save() {
	string fileSave = filename.toStdString();
	imwrite(fileSave, dstImage);
}

//另存为
void MainWindow::saveAs() {
	QString otherFilename = QFileDialog::getSaveFileName(this,
		tr("save picture"),
		".",
		tr("Image file(*.png *.jpg *.bmp)"));
	if (!otherFilename.isEmpty()) {
		string fileSave = otherFilename.toStdString();
		imwrite(fileSave, dstImage);
	}
	else {
		return;
	}
}

//保存选项打开
void MainWindow::save_on() {
	if (!dstQimage.isNull()) {
		ui->actionsave->setEnabled(true);
		ui->actionsaveas->setEnabled(true);
	}
	else {
		ui->actionsave->setEnabled(false);
		ui->actionsaveas->setEnabled(false);
	}
}

//保存选项关闭
void MainWindow::save_off() {
	ui->actionsave->setEnabled(false);
	ui->actionsaveas->setEnabled(false);
}

//显示在生成图界面
void MainWindow::display() {
	resultScene.clear();//清空结果面板
	QGraphicsScene *rScene = &resultScene;

	//int with = ui->resultView->width();
	//int height = ui->resultView->height();
	QPixmap pixmap = QPixmap::fromImage(dstQimage);
	//QPixmap fitpixmap = pixmap.scaled(with, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);//按比例缩小
	//QPixmap fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);//填充满
	rScene->addPixmap(pixmap);
	ui->resultView->setScene(rScene);
	ui->resultView->show();
}

//清除原图
void MainWindow::clearFormer() {
	formerScene.clear();//清空原图面板
	ui->menu_image->setEnabled(false);
	ui->menu_video->setEnabled(false);
}

//清除生成图
void MainWindow::clearResult() {
	resultScene.clear();//清空原图面板
	save_off();
}

//旋转
void MainWindow::turn()
{
	flip(srcImage, dstImage, 1);//翻转

}

//灰度化
void MainWindow::gray() {
	ImageProcessing img(srcImage);
	dstImage = img.rgb2gray();
	//将Mat图像转换为QImage图像，才能显示
	dstQimage = Mat2QImage(dstImage);
	display();
	save_on();
}

//二值化
void MainWindow::binarySolt() {
	Binary *BinaryDialog = new Binary();
	//对话框关闭时销毁
	BinaryDialog->setAttribute(Qt::WA_DeleteOnClose);
	BinaryDialog->setWindowTitle(tr("Binary"));
	//BinaryDialog的阈值发送给二值的core
	connect(BinaryDialog, SIGNAL(BinaryThres(int)), this, SLOT(binaryCore(int)));
	BinaryDialog->show();
}
void MainWindow::binaryCore(int thres) {
	ImageProcessing img(srcImage);
	dstImage = img.rgb2black(thres);
	dstQimage = Mat2QImage(dstImage);
	display();
	save_on();
}

//线性灰度变换
void MainWindow::linearGraySolt() {
	LinearGrayScale *LinearGrayScaleDialog = new LinearGrayScale();
	LinearGrayScaleDialog->setAttribute(Qt::WA_DeleteOnClose);
	LinearGrayScaleDialog->setWindowTitle(tr("LinearGrayScale"));
	connect(LinearGrayScaleDialog, SIGNAL(contrastAndBright(int,int)), this, SLOT(linearGrayCore(int,int)));
	LinearGrayScaleDialog->show();
}
void MainWindow::linearGrayCore(int contrastValue, int brightValue) {
	ImageProcessing img(srcImage);
	dstImage = img.linearGrayScaleTransformation(contrastValue, brightValue);
	//将Mat图像转换为QImage图像，才能显示
	dstQimage = Mat2QImage(dstImage);
	display();
	save_on();
}

//分段线性变换
void MainWindow::pieceWiselinearGraySolt() {
	PieceWiselinearGrayScale *PieceWiselinearGrayScaleDialog = new PieceWiselinearGrayScale();
	PieceWiselinearGrayScaleDialog->setAttribute(Qt::WA_DeleteOnClose);
	PieceWiselinearGrayScaleDialog->setWindowTitle(tr("PieceWiselinearGrayScale"));
	connect(PieceWiselinearGrayScaleDialog, SIGNAL(inflectionPoint(int,int,int,int)), this, SLOT(pieceWiselinearGrayCore(int,int,int,int)));
	PieceWiselinearGrayScaleDialog->show();
}
void MainWindow::pieceWiselinearGrayCore(int X1, int Y1, int X2, int Y2) {
	ImageProcessing img(srcImage);
	dstImage = img.pieceWiselinearGrayScaleTransformation(X1,Y1,X2,Y2);
	//将Mat图像转换为QImage图像，才能显示
	dstQimage = Mat2QImage(dstImage);
	display();
	save_on();
}

//对数
void MainWindow::loglinearGrayScaleSolt() {
	LoglinearGrayScale *loglinearGrayScaleDialog = new LoglinearGrayScale();
	loglinearGrayScaleDialog->setAttribute(Qt::WA_DeleteOnClose);
	loglinearGrayScaleDialog->setWindowTitle(tr("PieceWiselinearGrayScale"));
	connect(loglinearGrayScaleDialog, SIGNAL(Loglinear(int)), this, SLOT(loglinearGrayScaleCore(int)));
	loglinearGrayScaleDialog->show();
}
void MainWindow::loglinearGrayScaleCore(int c) {
	ImageProcessing img(srcImage);
	dstImage = img.loglinearGrayScaleTransformation(c);
	//将Mat图像转换为QImage图像，才能显示
	dstQimage = Mat2QImage(dstImage);
	display();
	save_on();
}

//幂率
void MainWindow::powerLawGrayScaleSolt() {
	PowerLawGrayScale *PowerLawGrayScaleDialog = new PowerLawGrayScale();
	PowerLawGrayScaleDialog->setAttribute(Qt::WA_DeleteOnClose);
	PowerLawGrayScaleDialog->setWindowTitle(tr("PieceWiselinearGrayScale"));
	connect(PowerLawGrayScaleDialog, SIGNAL(PowerLaw(double)), this, SLOT(powerLawGrayScaleCore(double)));
	PowerLawGrayScaleDialog->show();
}
void MainWindow::powerLawGrayScaleCore(double index) {
	ImageProcessing img(srcImage);
	dstImage = img.powerLawlinearGrayScaleTransformation(index);
	//将Mat图像转换为QImage图像，才能显示
	dstQimage = Mat2QImage(dstImage);
	display();
	save_on();
}

//颜色空间缩减
void MainWindow::cutColorSolt() {
	CutColor *CutColorDialog = new CutColor();
	//对话框关闭时销毁
	CutColorDialog->setAttribute(Qt::WA_DeleteOnClose);
	CutColorDialog->setWindowTitle(tr("CutColor"));
	//BinaryDialog的阈值发送给二值的core
	connect(CutColorDialog, SIGNAL(CutColorRank(int)), this, SLOT(cutColorCore(int)));
	CutColorDialog->show();
}
void MainWindow::cutColorCore(int n) {
	ImageProcessing img(srcImage);
	dstImage = img.cutColor(n);
	dstQimage = Mat2QImage(dstImage);
	display();
	save_on();
}

//直方图及均衡化
void MainWindow::HistogramSolt() {
	Histogram *hist = new Histogram();
	hist->getSrcImg(srcImage);
	//对话框关闭时销毁
	hist->setAttribute(Qt::WA_DeleteOnClose);
	hist->setWindowTitle(tr("histogram"));
	QObject::connect(hist, SIGNAL(equaliPic(Mat)), this, SLOT(HistogramCore(Mat)));
	hist->show();
}
void MainWindow::HistogramCore(Mat res) {
	dstImage = res;
	dstQimage = Mat2QImage(dstImage);
	display();
	save_on();
}

//边缘检测
void MainWindow::EdgeDetectionSolt() {
	EdgeDetection *EdgeDetectionDialog = new EdgeDetection();
	//对话框关闭时销毁
	EdgeDetectionDialog->setAttribute(Qt::WA_DeleteOnClose);
	EdgeDetectionDialog->setWindowTitle(tr("EdgeDetection"));
	//BinaryDialog的阈值发送给二值的core
	connect(EdgeDetectionDialog, SIGNAL(EdgeNum(int, int, int, int)), this, SLOT(EdgeDetectionCore(int,int,int,int)));
	EdgeDetectionDialog->show();
}
void MainWindow::EdgeDetectionCore(int w,int b,int s,int kSize) {
	ImageProcessing img(srcImage);
	dstImage = img.edgeDetection(w,b,s,kSize);
	dstQimage = Mat2QImage(dstImage);
	display();
	save_on();
}

//滤波
void MainWindow::freqFilter() {
	FreqFilter *FreqFilterDialog = new FreqFilter();
	FreqFilterDialog->getSrcImg(srcImage);
	FreqFilterDialog->srcSpectrum();
	FreqFilterDialog->setAttribute(Qt::WA_DeleteOnClose);
	FreqFilterDialog->setWindowTitle(tr("EdgeDetection"));
	FreqFilterDialog->show();

	//ImageProcessing img(srcImage);
	//dstImage = img.dftTransformation();
	////dstImage = img.gausHighLowFilter(dstImage,20,false);
	////dstImage = img.IdealHighLowFilter(dstImage,20, false);
	//dstImage = img.ButterworthHighLowFilter(dstImage, 20, 5, false);
	////int a = dstImage.channels();
	//dstQimage = Mat2QImage(dstImage);

	display();
	save_on();
}

void MainWindow::spaceFilter() {

	ImageProcessing img(srcImage);
	dstImage = img.dftTransformation();
	//dstImage = img.gausHighLowFilter(dstImage,20,false);
	//dstImage = img.IdealHighLowFilter(dstImage,20, false);
	dstImage = img.ButterworthHighLowFilter(dstImage, 20, 5, false);
	//int a = dstImage.channels();
	dstQimage = Mat2QImage(dstImage);

	display();
	save_on();
}

//滤波
//void MainWindow::filter() {
//	//模态对话框，主框体仅仅与一个消息框体同存
//	QDialog dialog;
//	dialog.setWindowTitle(tr("Hello, dialog!"));
//	dialog.exec();
//}

//void MainWindow::filter() {
//	//非模态对话框，主框体可与多消息框体同存
//	Binary *dialog = new Binary;
//	//dialog->setAttribute(Qt::WA_DeleteOnClose);//消息窗口关闭时释放
//	dialog->setWindowTitle(tr("Hello, dialog!"));
//	dialog->show();
//}

/*所谓的模态Dialog就是将当前线程放入阻塞队列，
所谓的非模态Dialog就是再创建一个线程专门用来显示对话框，
可以这么理解，事实上，模态、非模态是针对事件循环的。不过事件循环其实也是在一个新的线程里面的
*/
//void MainWindow::on_openBtn_clicked()
//{
//	QString fileName = QFileDialog::getOpenFileName(
//		this, "打开图片",
//		".",
//		"图片 (*.bmp *.jpg *.pbm *.pgm *.png *.ppm *.xbm *.xpm);;所有文件(*.*)");
//	if (fileName != "")
//	{
//		if (img->load(fileName))
//		{
//			QGraphicsScene *scene = new QGraphicsScene;
//			scene->addPixmap(QPixmap::fromImage(*img));
//			ui->formerView->setScene(scene);
//			ui->formerView->resize(img->width() + 10, img->height() + 10);
//			ui->formerView->show();
//		}
//		else
//		{
//			QMessageBox::information(this,
//				tr("打开图像失败"),
//				tr("打开图像失败!"));
//			delete img; return;
//		}
//	}
//}