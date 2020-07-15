#include "Pixelnode.h"
#include "Scissor.h"
#include <opencv2\highgui\highgui.hpp>
#include<opencv2\imgcodecs.hpp>
#include<opencv2\opencv.hpp>
#include<iostream>
#include<algorithm>

using namespace std;
using namespace cv;

Scissor scissor;
Mat img;
/*PixelNode* nodes;
PixelNode& GetPixelNode(int r, int c, int width)
{
	return *(nodes + r * width + c);
}
void CumulateLinkCost(PixelNode* node, int linkIndex, int Qr, int Qc, const Mat& CostMap, float scale)
{
	if (Qc < 0 || Qc >= CostMap.cols || Qr < 0 || Qr >= CostMap.rows)
		return;
	const PixelGray* ptr = CostMap.ptr<PixelGray>(Qr, Qc);
	float val = *ptr;
	node->linkCost[linkIndex] += val * scale;
}*/


	/*for (int i = 0; i < FzCostMap.rows; i++)
	{
		for (int j = 0; j < FzCostMap.cols; j++)
		{
			PixelNode* pix = FzCostMap.ptr<PixelNode>(i, j);
			PixelNode& node = node.GetPixelNode(nodes, i, j, FzCostMap.cols);
			for (int i = 0; i < 8; i++)
			{
				int offsetX, offsetY;
				node.GetNodeOffset(offsetX, offsetY, i);
				node.CumulateLinkCost(&node, i, node.row + offsetY, node.column + offsetX, FzCostMap, 1);
			}
		}
	}*/
	/*auto cumulateOp = [&](PixelGray& val, const int* position) ->void
	{
		PixelNode& node = GetPixelNode(position[0], position[1], FzCostMap.cols);
		for (int i = 0; i < 8; i++)
		{
			int offsetX, offsetY;
			node.GetNodeOffset(offsetX, offsetY, i);
			CumulateLinkCost(&node, i, node.row + offsetY, node.column + offsetX,
				FzCostMap, 1);
		}
	};
	FzCostMap.forEach<PixelGray>(cumulateOp);*/






/*void colorReduce(Mat& img)
{
	//单通道图像
	if (img.channels() == 1)
	{
		Mat_<uchar>::iterator it = img.begin<uchar>();
		Mat_<uchar>::iterator itend = img.end<uchar>();
		while (it != itend)
		{
			//相关操作
			*it = 0;
			it++;
		}
	}
	//三通道图像
	else if (img.channels() == 3)
	{
		Mat_<Vec3b>::iterator it = img.begin<Vec3b>();
		Mat_<Vec3b>::iterator itend = img.end<Vec3b>();
		while (it != itend)
		{
			//相关操作
			(*it)[0] = 255;
			(*it)[1] = 255;
			(*it)[2] = 255;
			it++;
		}
	}
}*/



int main(int argc, char* argv[])
{
	Mat img0 = imread("Avatar.jpg");
	cvtColor(img0, img, COLOR_BGR2BGRA);
	//colorReduce(img);
	scissor = Scissor(img);
	imshow("Original Image", img0);
	waitKey(0);
	scissor.ComputeFzCostMap();
	imshow("fz", scissor.FzCostMap);
	waitKey(0);
	scissor.ComputeFgCostMap();
	imshow("fg", scissor.FgCostMap);
	waitKey(0);
	scissor.ComputeFdCostMap();
	//imshow("fd", scissor.FdCostMap);
	//waitKey(0);
	imshow("CostMap", scissor.__MakeCostImage());
	waitKey(0);
	//GetCostMap(fg, fz, fd, cost_map);
}