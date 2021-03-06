#include "CoverGen.h"
#include "utils.h"

CoverGen::CoverGen(std::string input, int width, int height)
{
	this->input = input;
	this->width = width;
	this->height = height;
}

Mat CoverGen::getStaticFrame()
{
	//原始图像裁剪到正方形，并缩放到对应大小
	Mat inputImage = imread(this->input);
	Size inputSize = inputImage.size();
	Mat centerSquare = inputImage(getCenterRect(inputSize.width, inputSize.height));

	int detSize = (int)std::ceil(std::min(this->height, this->width) * 0.9);
	int origSize = centerSquare.size().height;

	if (detSize < origSize) {
		resize(centerSquare, centerSquare, Size(detSize, detSize), 0, 0, INTER_AREA);
	}
	else {
		resize(centerSquare, centerSquare, Size(detSize, detSize), 0, 0, INTER_CUBIC);
	}

	//正方形图像缩放到背景同样大小并模糊
	Mat bgImage = centerSquare.clone();
	int bgSize = std::max(this->height, this->width);
	if (bgSize < origSize) {
		resize(bgImage, bgImage, Size(bgSize, bgSize), 0, 0, INTER_AREA);
	}
	else {
		resize(bgImage, bgImage, Size(bgSize, bgSize), 0, 0, INTER_CUBIC);
	}

	int gaussBlurSize = bgSize / 6;
	if (gaussBlurSize % 2 == 0) gaussBlurSize += 1;
	GaussianBlur(bgImage, bgImage, Size(gaussBlurSize, gaussBlurSize), 0);

	Mat background(this->height, this->width, CV_8UC3, Scalar(255, 255, 255)); //全白背景
	Mat shadowMask(this->height, this->width, CV_8UC4, Scalar(0, 0, 0, 0)); //阴影

	//背景贴上模糊图像0.4不透明度
	Mat bgImageROI = bgImage(getCenterRectFromSquare(bgSize, this->width, this->height));
	addWeighted(background, 0.6, bgImageROI, 0.4, 0, background);

	//背景中心贴上原始图像
	Rect centerArea = getCenterImageArea(this->width, this->height, 0.9);

	rectangle(shadowMask, centerArea, Scalar(128, 128, 128, 210), FILLED);
	GaussianBlur(shadowMask, shadowMask, Size(45, 45), 0);

	addAlphaMat(background, shadowMask);

	Mat backCenterROI = background(centerArea);
	addWeighted(backCenterROI, 0, centerSquare, 1, 0, backCenterROI);
	return background;
}