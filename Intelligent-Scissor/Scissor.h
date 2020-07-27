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
	// ����������
	using Single_Channel_Value_Type = float;  // ���嵥ͨ��ͼ�����������
	using RGB_img = cv::Point3f;              // ������ͨ��RGBͼ����������ͣ�3ά32-bit floating-point number
	using Gray_img = Single_Channel_Value_Type;
	// ���3f��CostMap����Ϊ������Ҫ����������ʱ����public��
	cv::Mat FzCostMap;
	cv::Mat FgCostMap;
	cv::Mat FdCostMap;
	Scissor() {}  // ���캯��
	Scissor(const cv::Mat& origin_img);  //��ʼ��
	cv::Mat MakeCostImage();  // ������ͼ
	void ComputeFzCostMap();  // ����������˹�㽻��Laplacian Zero-Crossing�������Ĵ��ۺ�����costmap
	void ComputeFgCostMap(); // �����ݶȴ�С���ۺ�����CostMap
	void ComputeFdCostMap();// �����ݶȷ�����ۺ�����CostMap
	bool IsSetSeed() const {
		return isSetSeed;
	}
	void LiveWireDP(int seedrow, int seedcol);// ÿ���������ӵ�  �ظ��������·��
	void CalculateMininumPath(CTypedPtrDblList<PixelNode>& path, int freePtRow, int freePtCols);//��ȡ���·��
	cv::Point2i CursorSnap(int row, int col, const int width);//������CursorSnap���ܵ�ʱ�򣬱����ṩSample�����ݣ����Sample����Ϊ�գ���ô��ֱ�ӵ���Live Wire DP��
private:
	cv::Mat OriginImage;  // ԭͼ
	cv::Mat Ix;  // x�����ݶ�
	cv::Mat Iy;  // y�����ݶ�
	cv::Mat GradMag;    //�ݶȷ�ֵ
	// Ȩ��
	static float Wz;
	static float Wg;
	static float Wd;

	int Rows;  // ͼ��y���򳤶�
	int Cols;  // ͼ��x���򳤶�
	int Channels;  // ͼ��ͨ����
	PixelNode* nodes;
	static cv::Mat Laplace_kernel;      // ������˹����
	static cv::Mat Sobel_x;             // sobel����
	static cv::Mat Sobel_y;
	PixelNode& GetPixelNode(int x, int y, int imgWidth);// ��ȡ(r, c)���ص��Ӧ��Node
	void CumulateLinkCost(PixelNode* node, int linkIndex, int R, int C, const Mat& CostMap, float scale);// ����(R, C)��LinkIdexλ���ϵ�linkCost
	bool isSetSeed = false;

};

#endif