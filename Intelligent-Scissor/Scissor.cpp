#include "Scissor.h"
#include "CostMap.h"
using namespace std;
using namespace cv;
// 特征函数的权值
float Scissor::Wz = 0.3f;
float Scissor::Wg = 0.3f;
float Scissor::Wd = 0.1f;
// 拉普拉斯算子
Mat Scissor::Laplace_kernel = (Mat_<char>(3, 3) <<
	0, -1, 0,
	-1, 4, -1,
	0, -1, 0);
// Sobel算子
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
const double PI = 3.141592654f;                             //pi
const double _2DIVI3PI = 2.0 / (3.0 * PI);                  // 2/3 * pi

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
	Mat GrayImage, GrayIx, GrayIy;
	cvtColor(origin_img, GrayImage, COLOR_BGRA2GRAY);
	filter2D(GrayImage, Ix, CV_32F, Sobel_x);
	filter2D(GrayImage, Iy, CV_32F, Sobel_y);
	magnitude(Ix, Iy, GradMag);
}

PixelNode& Scissor::GetPixelNode(int r, int c, int width)
{
	return *(nodes + r * width + c);
}

void Scissor::CumulateLinkCost(PixelNode* node, int linkIndex, int R, int C, const Mat& CostMap, float scale)
{
	if (C < 0 || C >= Cols || R < 0 || R >= Rows)
		return;
	const Gray_img* ptr = CostMap.ptr<Gray_img>(R, C);
	node->linkCost[linkIndex] += *ptr * scale;
}

void Scissor::ComputeFzCostMap()
{
	filter2D(OriginImage, FzCostMap, CV_32F, Laplace_kernel);  // 与拉普拉斯算子卷积
	//	Function:
	//	1	if(Lz(p) == 0)
	//	0	if(lz(p) != 0)
	// 如果输入是单通道的图像
	if (Channels == 1)
	{
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				// 创建指针来遍历图像
				Gray_img* Fzptr = FzCostMap.ptr<Gray_img>(i, j);
				if (abs(*Fzptr) < 0.00001f)
				{
					*Fzptr = 1.0f * Wz;
				}
				else
				{
					*Fzptr = 0.0f;
				}
			}
		}
	}
	// 如果输入是三通道RGB图像
	else
	{
		// 创建一个矩阵来暂存数据
		Mat NewFzCostMap;
		NewFzCostMap.create(FzCostMap.size(), CV_32FC1);

		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				Gray_img* Newptr = NewFzCostMap.ptr<Gray_img>(i, j);
				RGB_img* Fzptr = FzCostMap.ptr<RGB_img>(i, j);
				// 对三个通道分别操作
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
				// 取三通道中的最大值
				*Newptr = max({ Fzptr->x, Fzptr->y, Fzptr->z });
			}
		}
		swap(FzCostMap, NewFzCostMap);
	}

	for (int i = 1; i < Rows - 1; ++i)
	{
		for (int j = 1; j < Cols - 1; ++j)
		{
			// 指向第(i, j)个像素的node对象
			PixelNode& node = GetPixelNode(i, j, Cols);
			// 对八个领域分别操作
			for (int k = 0; k < 8; k++)
			{
				int offsetX, offsetY;
				// 获取第k个领域的位置偏移量
				node.GetNodeOffset(offsetX, offsetY, k);
				// 累计LinkCost
				CumulateLinkCost(&node, k, node.row + offsetY, node.column + offsetX, FzCostMap, 1);
			}
		}
	}
}


void Scissor::ComputeFgCostMap()
{
	// 分别求像素在x和y方向的梯度Ix、Iy
	filter2D(OriginImage, Ix, CV_32F, Sobel_x);
	filter2D(OriginImage, Iy, CV_32F, Sobel_y);
	FgCostMap.create(Ix.size(), CV_32FC1);
	// 对于单通道图像
	if (Channels == 1)
	{
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				Gray_img* ix = Ix.ptr<Gray_img>(i, j);
				Gray_img* iy = Iy.ptr<Gray_img>(i, j);
				Gray_img* out = FgCostMap.ptr<Gray_img>(i, j);

				*out = sqrt(*ix * *ix + *iy * *iy);             // 梯度G = sqrt（Ix^2 + Iy^2)
			}
		}
	}
	// 对三通道RGB图像
	else
	{
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				RGB_img* ix = Ix.ptr<RGB_img>(i, j);
				RGB_img* iy = Iy.ptr<RGB_img>(i, j);
				Gray_img* out = FgCostMap.ptr<Gray_img>(i, j);
				// 分别求三个通道的梯度G
				Gray_img X = sqrt(ix->x * ix->x + iy->x * iy->x);
				Gray_img Y = sqrt(ix->y * ix->y + iy->y * iy->y);
				Gray_img Z = sqrt(ix->z * ix->z + iy->z * iy->z);
				// 取最大值
				*out = max({ X, Y, Z });
			}
		}
	}

	float max_G = 0.0f;
	float min_G = -1.0f;
	// 寻找G的最大值和最小值
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
			// 静态特征代价函数fg = 1 - （fg - min(fg)） / (max(fg) - min(fg))
			// Wg为特征权重
			*fg = (1.0f - (*fg - min_G) / (max_G - min_G)) * Wg;
		}
	}

	for (int i = 1; i < Rows - 1; ++i)
	{
		for (int j = 1; j < Cols - 1; ++j)
		{
			// 指向第(i, j)个像素的node对象
			PixelNode& node = GetPixelNode(i, j, Cols);
			// 对八个领域分别操作
			for (int k = 0; k < 8; k++)
			{
				int offsetX, offsetY;
				// 获取第k个领域的位置偏移量
				node.GetNodeOffset(offsetX, offsetY, k);
				// 累计LinkCost
				// 为了使最大梯度保持在一个单位，利用欧几里得距离对梯度大小代价进行缩放。
				// 如果q是p的对角线邻居，fg(q)被缩放1
				// 如果q是水平或垂直邻居，被缩放1/√2
				CumulateLinkCost(&node, k, node.row + offsetY, node.column + offsetX,
					FgCostMap, (i % 2 == 0) ? SQINV : 1.0f);
			}
		}
	}
}

void Scissor::ComputeFdCostMap()
{
	// D(p)为点p处梯度方向的单位向量，并将D(p)定义为与D(p)垂直(顺时针旋转90度)的单位向量
	FdCostMap.create(OriginImage.size(), CV_32FC1);
	Mat RotateDMat(OriginImage.size(), CV_32FC2);
	// 对所有像素求D'(p)
	// 对于单通道图像
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
	// 对于三通道RGB图像
	else
	{
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				RGB_img* pIx = Ix.ptr<RGB_img>(i, j);
				RGB_img* pIy = Iy.ptr<RGB_img>(i, j);
				float sumIx = pIx->x + pIx->y + pIx->z;
				float sumIy = pIy->x + pIy->y + pIy->z;
				Vec2f* pRotateDMat = RotateDMat.ptr<Vec2f>(i, j);
				*pRotateDMat = normalize(Vec2f(sumIy, -sumIx));
			}
		}
	}
	for (int r = 1; r < Rows - 1; ++r)
	{
		for (int c = 1; c < Cols - 1; ++c)
		{
			auto RotateDp = *(RotateDMat.ptr<Vec2f>(r, c));
			auto& node = GetPixelNode(r, c, Cols);                                     // 指向第(r, c)个像素的node对象
			// 对8个领域分别操作
			for (int i = 0; i < 8; i++)
			{
				Gray_img* fd = FdCostMap.ptr<Gray_img>(r, c);
				int offsetX, offsetY;
				node.GetNodeOffset(offsetX, offsetY, i);                               // 获取第i个领域的位置偏移量
				auto Index = cv::Vec2f(offsetX, offsetY);                              // Index = q - p
				auto RotateDq = *(RotateDMat.ptr<Vec2f>(r + offsetY, c + offsetX));    // 求D'(q)
				float k = 1.0f / sqrt(Index.dot(Index));
				if (RotateDp.dot(Index) >= 0.0f)                             // 当D'(p) * (q - p) >= 0
				{
					auto L_pq = k * Index;                                  // L(p, q)是像素p和q之间的单位向量
					float Dp = RotateDp.dot(L_pq);                          // Dp(p, q) = D'(p) * L(p, q)
					float Dq = L_pq.dot(RotateDq);                          // Dq(p, q) = L(p, q) * D'(q)
					*fd = ((_2DIVI3PI * PI * (acos(Dp) + acos(Dq))) * Wd);  // fd(p, q) = 2/3 * Pi * {acos[Dp(p, q)] + acos[Dq(p, q)]}
					node.linkCost[i] += *fd;
				}
				else                                                         // 当D'(p) * (q - p) < 0
				{
					auto L_pq = -k * Index;                                  // 同上
					float Dp = RotateDp.dot(L_pq);
					float Dq = L_pq.dot(RotateDq);
					*fd = ((_2DIVI3PI * (acos(Dp) + acos(Dq))) * Wd);
					node.linkCost[i] += *fd;
				}
			}
		}
	}
}

Mat Scissor::MakeCostImage()
{
	Mat dst;
	MakeCostGraph(dst, nodes, OriginImage);
	return std::move(dst);
}


void Scissor::LiveWireDP(int seedRow, int seedCol)
{
	if (seedRow < 0 || seedRow >= Rows || seedCol < 0 || seedCol >= Cols)
		return;
	auto InitialStateFunctor = [&]()//初始化函数，设置所有节点状态为initial
	{
		int NodeSizes = Rows * Cols;
		int index = 0;
		while (index < NodeSizes)
		{
			(nodes + index)->state = INITIAL;
			index++;
		}
	};
	//将所有状态设置为INITIAL
	InitialStateFunctor();
	PixelNode* seed = &GetPixelNode(seedRow, seedCol, Cols);//获得种子节点的地址
	seed->totalCost = 0.0f;//初始化种子节点的总代价
	//使用优先队列
	CTypedPtrHeap<PixelNode> pq;//使用优先队列
	pq.Insert(seed);//插入种子节点
	while (!pq.IsEmpty())//当优先队列不为空时循环
	{
		PixelNode* q = pq.ExtractMin();//获取代价最小节点的地址q
		q->state = EXPANDED;//设置其状态为已扩展
		for (int i = 0; i < 8; i++)//扩展8领域
		{
			int nbrCol, nbrRow;//8领域偏移量
			q->GetNodeOffset(nbrCol, nbrRow, i);//根据i得到8领域偏移量
			nbrCol += q->column;//加上当前节点的位置，获得8领域节点的行列位置
			nbrRow += q->row;
			//Boarder Checking
			if (nbrRow <= 1 || nbrRow >= (Rows - 1) || nbrCol <= 1 || nbrCol >= Cols - 1)
				continue;
			PixelNode* r = &GetPixelNode(nbrRow, nbrCol, Cols);//获取领域点地址
			if (r->state != EXPANDED)//判断是否还未扩展
			{
				if (r->state == INITIAL)
				{
					r->totalCost = q->totalCost + q->linkCost[i];//总代价累计
					r->state = ACTIVE;//设置状态为激活
					r->prevNode = q;//设置其前向节点为q（r是q的8领域节点）
					pq.Insert(r);//优先队列中插入次领域节点
				}
				else if (r->state == ACTIVE)
				{
					double totalTempCost = q->totalCost + q->linkCost[i]; //计算从新路径q连接的总代价
					if (totalTempCost < r->totalCost)//如果新的总代价小于原总代价
					{
						r->totalCost = totalTempCost;//设置新总代价为新路径总代价
						r->prevNode = q;//设置新路径
						pq.Update(r);//更新优先队列
					}
				}
			}
		}
	}
	seed->prevNode = nullptr;
	isSetSeed = true;
}

void Scissor::CalculateMininumPath(CTypedPtrDblList<PixelNode>& path, int freePtRow, int freePtCol)
{
	if (freePtRow <= 1 || freePtRow >= Rows - 1 || freePtCol <= 1 || freePtCol >= Cols - 1)//检测边界条件,如果自由点超出图像边界，则返回
		return;
	PixelNode* freePtNode = &GetPixelNode(freePtRow, freePtCol, Cols);//获取自由点的地址
	while (freePtNode != nullptr)
	{
		path.AddHead(freePtNode);//插入节点
		freePtNode = freePtNode->prevNode;//循环回溯自由节点的前一个节点
	}
}


Point2i Scissor::CursorSnap(int row, int col, const int width)
{
	//return selected point
	if (width <= 0)
		LiveWireDP(row, col);
	else
	{
		int seedRow = row;
		int seedCol = col;
		//Get ROI with the given width
		int startRow = max(row - width, 0);
		int endRow = min(row + width, Rows - 1);
		int startCol = max(col - width, 0);
		int endCol = min(col + width, Cols - 1);
		//Find position of maximum gradient pixel
		Mat ROI = GradMag(Range(startRow, endRow + 1), Range(startCol, endCol + 1));
		Point2i maxLoc;
		float maxVal = 0.;
		for (int i = 0; i < ROI.rows; i++)
			for (int j = 0; j < ROI.cols; j++)
			{
				if (maxVal < ROI.at<float>(i, j))
				{
					maxVal = ROI.at<float>(i, j);
					maxLoc.y = i;
					maxLoc.x = j;
				}
			}
		//minMaxLoc(ROI, &minVal, &maxVal, &minLoc, &maxLoc);
		seedRow = startRow + maxLoc.y;
		seedCol = startCol + maxLoc.x;
		//std::cout << "MaxLoc: (" << maxLoc.y << ", " << maxLoc.x << ")" << std::endl;
		std::cout << "using Cursor Snapping" << std::endl;
		std::cout << "Mouse pos: (" << row << ", " << col << ")" << std::endl;
		//std::cout << "Mouse pos GradMag: " << GradMag.at<float>(row, col) << std::endl;
		std::cout << "Selected pos: (" << seedRow << ", " << seedCol << ")" << std::endl;
		//std::cout << "Selected GradMag: " << GradMag.at<float>(seedRow, seedCol) << std::endl;
		Point2i point;
		point.y = seedRow;
		point.x = seedCol;
		return point;
	}

}
