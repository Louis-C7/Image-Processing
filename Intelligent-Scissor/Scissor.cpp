#include "Scissor.h"
#include "CostMap.h"
using namespace cv;
float Scissor::Wz = 0.3f;
float Scissor::Wg = 0.3f;
float Scissor::Wd = 0.1f;

Mat Scissor::Laplace_kernel = (Mat_<char>(3, 3) <<
	0, -1, 0,
	-1, 4, -1,
	0, -1, 0);
Mat Scissor::Sobel_x = (Mat_<char>(3, 3) <<
	-1, 0, 1,
	-2, 0, 2,
	-1, 0, 1);
Mat Scissor::Sobel_y = (Mat_<char>(3, 3) <<
	-1, -2, -1,
	0, 0, 0,
	1, 2, 1);
const double SQRT2 = 1.4142135623730950488016887242097;     //根号2
const double SQINV = 1.0 / SQRT2;
const double PI = 3.141592654f;
const double _2DIVI3PI = 2.0 / (3.0 * PI);
Scissor::Scissor(const Mat& origin_img)
{
	OriginImage = origin_img;
	Rows = OriginImage.rows;
	Cols = OriginImage.cols;
	Channels = OriginImage.channels();
	long numOfPixels = Rows * Cols;
	//分配与像素数量相同的的Nodes，并且创建映射
	nodes = new PixelNode[numOfPixels];
	for (int i = 0, cnt = 0; i < Rows; i++)
		for (int j = 0; j < Cols; j++)
		{
			(nodes + cnt)->row = i;
			(nodes + cnt)->column = j;
			(nodes + cnt)->totalCost = 0.0f;
			cnt++;
		}
	
	//isSetSeeed = false;
}

Mat Scissor::__MakeCostImage()
{
	Mat dst;
	MakeCostGraph(dst, nodes, OriginImage);
	return std::move(dst);
}

PixelNode& Scissor::GetPixelNode(int r, int c, int width)
{
	return *(nodes + r * width + c);
}

void Scissor::CumulateLinkCost(PixelNode* node, int linkIndex, int Qr, int Qc, const Mat& CostMap, float scale)
{
	if (Qc < 0 || Qc >= Cols || Qr < 0 || Qr >= Rows)
		return;
	const Gray_img* ptr = CostMap.ptr<Gray_img>(Qr, Qc);
	//float val = *ptr;
	node->linkCost[linkIndex] += *ptr * scale;
}

void Scissor::ComputeFzCostMap()
{
	filter2D(OriginImage, FzCostMap, CV_32F, Laplace_kernel);
	if (Channels == 1)
	{
		Mat_<Gray_img>::iterator it = FzCostMap.begin<Gray_img>();
		Mat_<Gray_img>::iterator itend = FzCostMap.end<Gray_img>();
		while (it != itend)
		{
			if (abs(*it) < 0.00001f)
			{
				*it = 1.0f * Wz;
			}
			else
			{
				*it = 0.0f;
			}
			it++;
		}
	}
	else //if(FzCostMap.channels() == 3)
	{
		/*Mat NewFzCostMap;
		NewFzCostMap.create(FzCostMap.size(), FzCostMap.type());
		//Mat_<Gray_img>::iterator fz_it = NewFzCostMap.begin<Gray_img>();
		Mat_<Vec3b>::iterator it = FzCostMap.begin<Vec3b>();
		Mat_<Vec3b>::iterator itend = FzCostMap.end<Vec3b>();
		while (it != itend)
		{
			for (int k = 0; k < 3; k++)
			{
				if ((*it)[k] != 0)
				{
					(*it)[k] = 1.0f*Wz;
				}
			}
			//*fz_it = max({ (*it)[0], (*it)[1], (*it)[2] }) * Wz;
			it++;
			//fz_it++;
		}
		swap(FzCostMap, NewFzCostMap);*/

		Mat NewFzCostMap;
		NewFzCostMap.create(FzCostMap.size(), CV_32FC1);

		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				Gray_img* Newptr = NewFzCostMap.ptr<Gray_img>(i, j);
				RGB_img* Fzptr = FzCostMap.ptr<RGB_img>(i, j);
				if (abs(Fzptr->x) < 0.00001f)
				{
					Fzptr->x = 1.0f * Wz;
				}
				else
				{
					Fzptr->x = 0.0f;
				}
				if (abs(Fzptr->y) < 0.00001f)
				{
					Fzptr->y = 1.0f * Wz;
				}
				else
				{
					Fzptr->y = 0.0f;
				}
				if (abs(Fzptr->z) < 0.00001f)
				{
					Fzptr->z = 1.0f * Wz;
				}
				else
				{
					Fzptr->z = 0.0f;
				}
				
			*Newptr = max({ Fzptr->x, Fzptr->y, Fzptr->z });
			}
		}
		swap(FzCostMap, NewFzCostMap);
	}

	for (int i = 1; i < Rows - 1; ++i)
    {
		for (int j = 1; j < Cols - 1; ++j)
		{
			PixelNode& node = GetPixelNode(i, j, Cols);
			for (int k = 0; k < 8; k++)
			{
				int offsetX, offsetY;
				node.GetNodeOffset(offsetX, offsetY, k);
				CumulateLinkCost(&node, k, node.row + offsetY, node.column + offsetX, FzCostMap, 1);
			}
		}
	}
}


void Scissor::ComputeFgCostMap()
{
	filter2D(OriginImage, Ix, CV_32F, Sobel_x);
	filter2D(OriginImage, Iy, CV_32F, Sobel_y);
	FgCostMap.create(Ix.size(), CV_32FC1);
	if (Channels == 1)
	{
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				Gray_img* ix = Ix.ptr<Gray_img>(i, j);
				Gray_img* iy = Iy.ptr<Gray_img>(i, j);
				Gray_img* out = FgCostMap.ptr<Gray_img>(i, j);
				*out = sqrt(*ix * *ix + *iy * *iy);
			}
		}
	}
	else
	{
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				RGB_img* ix = Ix.ptr<RGB_img>(i, j);
				RGB_img* iy = Iy.ptr<RGB_img>(i, j);
				Gray_img* out = FgCostMap.ptr<Gray_img>(i, j);
				Gray_img X = sqrt(ix->x * ix->x + iy->x * iy->x);
				Gray_img Y = sqrt(ix->y * ix->y + iy->y * iy->y);
				Gray_img Z = sqrt(ix->z * ix->z + iy->z * iy->z);
				*out = max({ X, Y, Z });
			}
		}
	}

	float max_G = 0.0f;
	float min_G = -1.0f;
	for (int k = 0; k < FgCostMap.rows; ++k)
	{
		Gray_img* ptr = FgCostMap.ptr<Gray_img>(k, 0);
		Gray_img* ptr_end = ptr + FgCostMap.cols;
		for (; ptr != ptr_end; ++ptr)
		{
			if (*ptr > max_G)
				max_G = *ptr;
			if (*ptr < min_G)
				min_G = *ptr;
		}
	}

	for (int i = 0; i < FgCostMap.rows; i++)
	{
		for (int j = 0; j < FgCostMap.cols; j++)
		{
			Gray_img* fg = FgCostMap.ptr<Gray_img>(i, j);
			*fg = (1.0f - (*fg - min_G) / (max_G - min_G)) * Wg;
		}
	}

	for (int i = 1; i < Rows - 1; ++i)
	{
		for (int j = 1; j < Cols - 1; ++j)
		{
			PixelNode& node = GetPixelNode(i, j, Cols);
			for (int k = 0; k < 8; k++)
			{
				int offsetX, offsetY;
				node.GetNodeOffset(offsetX, offsetY, k);
				CumulateLinkCost(&node, k, node.row + offsetY, node.column + offsetX, 
					FgCostMap, (i % 2 == 0) ? SQINV : 1.0f);
			}
		}
	}
}




void Scissor::ComputeFdCostMap()
{
	FdCostMap.create(OriginImage.size(), CV_32FC1);
	Mat RotateDMat(OriginImage.size(), CV_32FC2);
	if (Channels == 1)
	{
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				Gray_img* pIx = Ix.ptr<Gray_img>(i, j);
				Gray_img* pIy = Iy.ptr<Gray_img>(i, j);
				Vec2f* pRotateDMat = RotateDMat.ptr<Vec2f>(i, j);
				*pRotateDMat = normalize(Vec2f(*pIy, -*pIx));
			}
		}
	}
	else
	{
		//
	}
	for (int r = 1; r < Rows - 1; ++r)
	{
		for (int c = 1; c < Cols - 1; ++c)
		{
			auto RotateDp = *(RotateDMat.ptr<Vec2f>(r, c));
			auto& node = GetPixelNode(r, c, Cols);
			for (int i = 0; i < 8; i++)
			{
				Gray_img* fd = FdCostMap.ptr<Gray_img>(r, c);
				int offsetX, offsetY;
				node.GetNodeOffset(offsetX, offsetY, i);
				auto Index = cv::Vec2f(offsetX, offsetY);
				auto RotateDq = *(RotateDMat.ptr<Vec2f>(r + offsetY, c + offsetX));
				float k = 1.0f / sqrt(Index.dot(Index));
				if (RotateDp.dot(Index) >= 0.0f)
				{
					auto L_pq = k * Index;
					float Dp = RotateDp.dot(L_pq);
					float Dq = L_pq.dot(RotateDq);
					*fd = ((_2DIVI3PI * PI * (acos(Dp) + acos(Dq))) * Wd);
					node.linkCost[i] += *fd;
				}
				else
				{
					auto L_pq = -k * Index;
					float Dp = RotateDp.dot(L_pq);
					float Dq = L_pq.dot(RotateDq);
					*fd = ((_2DIVI3PI * (acos(Dp) + acos(Dq))) * Wd);
					node.linkCost[i] += *fd;
				}
			}
		}
	}
}


/*void Scissor::ComputeFdCostMap()
{
	//避免重复计算RotateDp，利用空间换时间
	Mat RotateDMat(OriginImage.size(), CV_32FC2);
	//梯度方向的权重需要注意的是图像的边界像素问题
	if (Channels == 1)
	{
		//为所有Pixel先产生RotateDVal
		RotateDMat.forEach<Vec2f>(
			[&](Vec2f& val, const int* position)
		{
			Gray_img* pIx = Ix.ptr<Gray_img>(position[0], position[1]);
			Gray_img* pIy = Iy.ptr<Gray_img>(position[0], position[1]);
			val = normalize(Vec2f(*pIy, -*pIx));
		}
		);

	}
	else
	{
	}



	auto GenLpq = [&](const Vec2f& rotateDp, const Vec2f& qMinusp) ->Vec2f
	{
		float k = 1.0f / sqrt(qMinusp.dot(qMinusp));
		if (rotateDp.dot(qMinusp) >= 0.0f)
			return k * qMinusp;
		return -k * qMinusp;
	};

	for (int r = 1; r < Rows - 1; ++r)
	{
		for (int c = 1; c < Cols - 1; ++c)
		{
			auto RotateDp = *(RotateDMat.ptr<Vec2f>(r, c));
			auto& node = GetPixelNode(r, c, Cols);
			for (int i = 0; i < 8; i++)
			{
				auto qMinusp = node.genVector(i);
				int offsetX, offsetY;
				node.GetNodeOffset(offsetX, offsetY, i);
				auto RotateDq = *(RotateDMat.ptr<Vec2f>(r + offsetY, c + offsetX));
				auto Lpq = GenLpq(RotateDp, qMinusp);
				float Dp = RotateDp.dot(Lpq);
				float Dq = Lpq.dot(RotateDq);
				node.linkCost[i] += ((_2DIVI3PI * (acos(Dp) + acos(Dq))) * Wd);
			}
		}
	}
}*/





/*void Scissor::ComputeFzCostMap()
{
	Rows = OriginImage.rows;
	Cols = OriginImage.cols;
	Channels = OriginImage.channels();
	filter2D(OriginImage, FzCostMap, CV_32F, Laplace_kernel);
	if (FzCostMap.channels() == 1)
	{
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				Gray_img it = FzCostMap.at<Gray_img>(Rows, Cols);
				if (it != 0)
				{
					FzCostMap.at<Gray_img>(Rows, Cols) = 0;
				}
			}
		}
	}
	else //if(FzCostMap.channels() == 3)
	{
		Mat NewFzCostMap;
		NewFzCostMap.create(FzCostMap.size(), FzCostMap.type());
		
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
			    Gray_img* Newptr = NewFzCostMap.ptr<Gray_img>(i, j);
				RGB_img* Fzptr = FzCostMap.ptr<RGB_img>(i, j);
				for (int k = 0; k < 2; k++)
				{
					if (abs(*Fzptr[k]) < 0.00001f)
					{
						*Fzptr[k] = 1.0f * Wz;
					}
					else
					{
						*Fzptr[k] = 0.0f;
					}
				}
				*NewFzCostMap = max({*Fzptr[0], *Fzptr[1], *Fzptr[2]})
			}
		}
	}
}*/



