#pragma once
#ifndef SCISSOR_H
#define SCISSOR_H
#include"Pixelnode.h"
#include<opencv2\opencv.hpp>

class Scissor
{
public:
	using Single_Channel_Value_Type = float;
	using RGB_img = cv::Point3f;
	using Gray_img = Single_Channel_Value_Type;
	cv::Mat FzCostMap;
	cv::Mat FgCostMap;
	cv::Mat FdCostMap;
	Scissor() {}
	Scissor(const cv::Mat& origin_img);
	cv::Mat __MakeCostImage();
	void ComputeFzCostMap();
	void ComputeFgCostMap();
	void ComputeFdCostMap();
	void GetCostMap();
private:
	cv::Mat OriginImage;

	
	cv::Mat Ix;
	cv::Mat Iy;

	static float Wz;
	static float Wg;
	static float Wd;

	int Rows;
	int Cols;
	int Channels;
	PixelNode* nodes;
	static cv::Mat Laplace_kernel;      // 拉普拉斯算子
	static cv::Mat Sobel_x;
	static cv::Mat Sobel_y;
	PixelNode& GetPixelNode(int x, int y, int imgWidth);

	void CumulateLinkCost(PixelNode* node, int linkIndex, int Qr, int Qc, const Mat& CostMap, float scale);

};

#endif