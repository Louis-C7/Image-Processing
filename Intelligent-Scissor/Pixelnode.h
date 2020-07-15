#pragma once
#ifndef PIXELNODE_H
#define PIXELNODE_H
#include<opencv2\opencv.hpp>
using namespace cv;
#define INITIAL 0
#define ACTIVE  1
#define EXPANDED 2
struct PixelNode
{
public:
	int column, row;//表示结点所在矩阵中的行与列
	double linkCost[8];//邻域的权值
	int state;
	double totalCost;
	PixelNode* prevNode;//用于形成最小路径
	PixelNode() : linkCost{ 0,0,0,0,0,0,0,0 },
		prevNode(nullptr),
		column(0),
		row(0),
		state(INITIAL),
		totalCost(0)
	{}
	void GetNodeOffset(int& offsetX, int& offsetY, int linkIndex);//从邻域0,1,2,3,4,5,6,7中获取偏移量
	
	//PixelNode& GetPixelNode(PixelNode* nodes, int r, int c, int width);
	PixelNode& GetPixelNode(int r, int c, int width);
	cv::Vec2f genVector(int linkIndex)
	{
		int offsetX, offsetY;
		GetNodeOffset(offsetX, offsetY, linkIndex);
		return cv::Vec2f(offsetX, offsetY);
	}
};
#endif