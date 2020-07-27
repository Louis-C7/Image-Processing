#pragma once
#ifndef SCISSOR_H
#define SCISSOR_H
#include"Pixelnode.h"
#include<opencv2\opencv.hpp>
#include "PriorityQueue.h"
using namespace cv;
class Scissor
{
public:
	// 类型重命名
	using Single_Channel_Value_Type = float;  // 定义单通道图像的数据类型
	using RGB_img = cv::Point3f;              // 定义三通道RGB图像的数据类型，3维32-bit floating-point number
	using Gray_img = Single_Channel_Value_Type;
	// 存放3f的CostMap（因为调试需要访问所以暂时放在public）
	cv::Mat FzCostMap;
	cv::Mat FgCostMap;
	cv::Mat FdCostMap;
	Scissor() {}  // 构造函数
	Scissor(const cv::Mat& origin_img);  //初始化
	cv::Mat MakeCostImage();  // 画代价图
	void ComputeFzCostMap();  // 计算拉普拉斯零交（Laplacian Zero-Crossing）带来的代价和生成costmap
	void ComputeFgCostMap(); // 计算梯度大小代价和生成CostMap
	void ComputeFdCostMap();// 计算梯度方向代价和生成CostMap
	bool IsSetSeed() const {
		return isSetSeed;
	}
	void LiveWireDP(int seedrow, int seedcol);// 每次设置种子点  重复计算最短路径
	void CalculateMininumPath(CTypedPtrDblList<PixelNode>& path, int freePtRow, int freePtCols);//获取最短路径
	cv::Point2i CursorSnap(int row, int col, const int width);//当启用CursorSnap功能的时候，必须提供Sample的数据，如果Sample数据为空，那么将直接调用Live Wire DP；
private:
	cv::Mat OriginImage;  // 原图
	cv::Mat Ix;  // x方向梯度
	cv::Mat Iy;  // y方向梯度
	cv::Mat GradMag;    //梯度幅值
	// 权重
	static float Wz;
	static float Wg;
	static float Wd;

	int Rows;  // 图像y方向长度
	int Cols;  // 图像x方向长度
	int Channels;  // 图像通道数
	PixelNode* nodes;
	static cv::Mat Laplace_kernel;      // 拉普拉斯算子
	static cv::Mat Sobel_x;             // sobel算子
	static cv::Mat Sobel_y;
	PixelNode& GetPixelNode(int x, int y, int imgWidth);// 获取(r, c)像素点对应的Node
	void CumulateLinkCost(PixelNode* node, int linkIndex, int R, int C, const Mat& CostMap, float scale);// 计算(R, C)点LinkIdex位置上的linkCost
	bool isSetSeed = false;

};

#endif