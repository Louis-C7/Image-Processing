#include "Scissor.h"
#include "CostMap.h"
using namespace std;
using namespace cv;
// ����������Ȩֵ
float Scissor::Wz = 0.3f;
float Scissor::Wg = 0.3f;
float Scissor::Wd = 0.1f;
// ������˹����
Mat Scissor::Laplace_kernel = (Mat_<char>(3, 3) <<
	0, -1, 0,
	-1, 4, -1,
	0, -1, 0);
// Sobel����
Mat Scissor::Sobel_x = (Mat_<char>(3, 3) <<
	-1, 0, 1,
	-2, 0, 2,
	-1, 0, 1);
Mat Scissor::Sobel_y = (Mat_<char>(3, 3) <<
	-1, -2, -1,
	0, 0, 0,
	1, 2, 1);

const double SQRT2 = 1.4142135623730950488016887242097;     //����2
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
	//����������������ͬ�ĵ�Nodes�����Ҵ���ӳ��
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
	filter2D(OriginImage, FzCostMap, CV_32F, Laplace_kernel);  // ��������˹���Ӿ��
	//	Function:
	//	1	if(Lz(p) == 0)
	//	0	if(lz(p) != 0)
	// ��������ǵ�ͨ����ͼ��
	if (Channels == 1)
	{
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				// ����ָ��������ͼ��
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
	// �����������ͨ��RGBͼ��
	else
	{
		// ����һ���������ݴ�����
		Mat NewFzCostMap;
		NewFzCostMap.create(FzCostMap.size(), CV_32FC1);

		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				Gray_img* Newptr = NewFzCostMap.ptr<Gray_img>(i, j);
				RGB_img* Fzptr = FzCostMap.ptr<RGB_img>(i, j);
				// ������ͨ���ֱ����
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
				// ȡ��ͨ���е����ֵ
				*Newptr = max({ Fzptr->x, Fzptr->y, Fzptr->z });
			}
		}
		swap(FzCostMap, NewFzCostMap);
	}

	for (int i = 1; i < Rows - 1; ++i)
	{
		for (int j = 1; j < Cols - 1; ++j)
		{
			// ָ���(i, j)�����ص�node����
			PixelNode& node = GetPixelNode(i, j, Cols);
			// �԰˸�����ֱ����
			for (int k = 0; k < 8; k++)
			{
				int offsetX, offsetY;
				// ��ȡ��k�������λ��ƫ����
				node.GetNodeOffset(offsetX, offsetY, k);
				// �ۼ�LinkCost
				CumulateLinkCost(&node, k, node.row + offsetY, node.column + offsetX, FzCostMap, 1);
			}
		}
	}
}


void Scissor::ComputeFgCostMap()
{
	// �ֱ���������x��y������ݶ�Ix��Iy
	filter2D(OriginImage, Ix, CV_32F, Sobel_x);
	filter2D(OriginImage, Iy, CV_32F, Sobel_y);
	FgCostMap.create(Ix.size(), CV_32FC1);
	// ���ڵ�ͨ��ͼ��
	if (Channels == 1)
	{
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				Gray_img* ix = Ix.ptr<Gray_img>(i, j);
				Gray_img* iy = Iy.ptr<Gray_img>(i, j);
				Gray_img* out = FgCostMap.ptr<Gray_img>(i, j);

				*out = sqrt(*ix * *ix + *iy * *iy);             // �ݶ�G = sqrt��Ix^2 + Iy^2)
			}
		}
	}
	// ����ͨ��RGBͼ��
	else
	{
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Cols; j++)
			{
				RGB_img* ix = Ix.ptr<RGB_img>(i, j);
				RGB_img* iy = Iy.ptr<RGB_img>(i, j);
				Gray_img* out = FgCostMap.ptr<Gray_img>(i, j);
				// �ֱ�������ͨ�����ݶ�G
				Gray_img X = sqrt(ix->x * ix->x + iy->x * iy->x);
				Gray_img Y = sqrt(ix->y * ix->y + iy->y * iy->y);
				Gray_img Z = sqrt(ix->z * ix->z + iy->z * iy->z);
				// ȡ���ֵ
				*out = max({ X, Y, Z });
			}
		}
	}

	float max_G = 0.0f;
	float min_G = -1.0f;
	// Ѱ��G�����ֵ����Сֵ
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
			// ��̬�������ۺ���fg = 1 - ��fg - min(fg)�� / (max(fg) - min(fg))
			// WgΪ����Ȩ��
			*fg = (1.0f - (*fg - min_G) / (max_G - min_G)) * Wg;
		}
	}

	for (int i = 1; i < Rows - 1; ++i)
	{
		for (int j = 1; j < Cols - 1; ++j)
		{
			// ָ���(i, j)�����ص�node����
			PixelNode& node = GetPixelNode(i, j, Cols);
			// �԰˸�����ֱ����
			for (int k = 0; k < 8; k++)
			{
				int offsetX, offsetY;
				// ��ȡ��k�������λ��ƫ����
				node.GetNodeOffset(offsetX, offsetY, k);
				// �ۼ�LinkCost
				// Ϊ��ʹ����ݶȱ�����һ����λ������ŷ����þ�����ݶȴ�С���۽������š�
				// ���q��p�ĶԽ����ھӣ�fg(q)������1
				// ���q��ˮƽ��ֱ�ھӣ�������1/��2
				CumulateLinkCost(&node, k, node.row + offsetY, node.column + offsetX,
					FgCostMap, (i % 2 == 0) ? SQINV : 1.0f);
			}
		}
	}
}

void Scissor::ComputeFdCostMap()
{
	// D(p)Ϊ��p���ݶȷ���ĵ�λ����������D(p)����Ϊ��D(p)��ֱ(˳ʱ����ת90��)�ĵ�λ����
	FdCostMap.create(OriginImage.size(), CV_32FC1);
	Mat RotateDMat(OriginImage.size(), CV_32FC2);
	// ������������D'(p)
	// ���ڵ�ͨ��ͼ��
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
	// ������ͨ��RGBͼ��
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
			auto& node = GetPixelNode(r, c, Cols);                                     // ָ���(r, c)�����ص�node����
			// ��8������ֱ����
			for (int i = 0; i < 8; i++)
			{
				Gray_img* fd = FdCostMap.ptr<Gray_img>(r, c);
				int offsetX, offsetY;
				node.GetNodeOffset(offsetX, offsetY, i);                               // ��ȡ��i�������λ��ƫ����
				auto Index = cv::Vec2f(offsetX, offsetY);                              // Index = q - p
				auto RotateDq = *(RotateDMat.ptr<Vec2f>(r + offsetY, c + offsetX));    // ��D'(q)
				float k = 1.0f / sqrt(Index.dot(Index));
				if (RotateDp.dot(Index) >= 0.0f)                             // ��D'(p) * (q - p) >= 0
				{
					auto L_pq = k * Index;                                  // L(p, q)������p��q֮��ĵ�λ����
					float Dp = RotateDp.dot(L_pq);                          // Dp(p, q) = D'(p) * L(p, q)
					float Dq = L_pq.dot(RotateDq);                          // Dq(p, q) = L(p, q) * D'(q)
					*fd = ((_2DIVI3PI * PI * (acos(Dp) + acos(Dq))) * Wd);  // fd(p, q) = 2/3 * Pi * {acos[Dp(p, q)] + acos[Dq(p, q)]}
					node.linkCost[i] += *fd;
				}
				else                                                         // ��D'(p) * (q - p) < 0
				{
					auto L_pq = -k * Index;                                  // ͬ��
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
	auto InitialStateFunctor = [&]()//��ʼ���������������нڵ�״̬Ϊinitial
	{
		int NodeSizes = Rows * Cols;
		int index = 0;
		while (index < NodeSizes)
		{
			(nodes + index)->state = INITIAL;
			index++;
		}
	};
	//������״̬����ΪINITIAL
	InitialStateFunctor();
	PixelNode* seed = &GetPixelNode(seedRow, seedCol, Cols);//������ӽڵ�ĵ�ַ
	seed->totalCost = 0.0f;//��ʼ�����ӽڵ���ܴ���
	//ʹ�����ȶ���
	CTypedPtrHeap<PixelNode> pq;//ʹ�����ȶ���
	pq.Insert(seed);//�������ӽڵ�
	while (!pq.IsEmpty())//�����ȶ��в�Ϊ��ʱѭ��
	{
		PixelNode* q = pq.ExtractMin();//��ȡ������С�ڵ�ĵ�ַq
		q->state = EXPANDED;//������״̬Ϊ����չ
		for (int i = 0; i < 8; i++)//��չ8����
		{
			int nbrCol, nbrRow;//8����ƫ����
			q->GetNodeOffset(nbrCol, nbrRow, i);//����i�õ�8����ƫ����
			nbrCol += q->column;//���ϵ�ǰ�ڵ��λ�ã����8����ڵ������λ��
			nbrRow += q->row;
			//Boarder Checking
			if (nbrRow <= 1 || nbrRow >= (Rows - 1) || nbrCol <= 1 || nbrCol >= Cols - 1)
				continue;
			PixelNode* r = &GetPixelNode(nbrRow, nbrCol, Cols);//��ȡ������ַ
			if (r->state != EXPANDED)//�ж��Ƿ�δ��չ
			{
				if (r->state == INITIAL)
				{
					r->totalCost = q->totalCost + q->linkCost[i];//�ܴ����ۼ�
					r->state = ACTIVE;//����״̬Ϊ����
					r->prevNode = q;//������ǰ��ڵ�Ϊq��r��q��8����ڵ㣩
					pq.Insert(r);//���ȶ����в��������ڵ�
				}
				else if (r->state == ACTIVE)
				{
					double totalTempCost = q->totalCost + q->linkCost[i]; //�������·��q���ӵ��ܴ���
					if (totalTempCost < r->totalCost)//����µ��ܴ���С��ԭ�ܴ���
					{
						r->totalCost = totalTempCost;//�������ܴ���Ϊ��·���ܴ���
						r->prevNode = q;//������·��
						pq.Update(r);//�������ȶ���
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
	if (freePtRow <= 1 || freePtRow >= Rows - 1 || freePtCol <= 1 || freePtCol >= Cols - 1)//���߽�����,������ɵ㳬��ͼ��߽磬�򷵻�
		return;
	PixelNode* freePtNode = &GetPixelNode(freePtRow, freePtCol, Cols);//��ȡ���ɵ�ĵ�ַ
	while (freePtNode != nullptr)
	{
		path.AddHead(freePtNode);//����ڵ�
		freePtNode = freePtNode->prevNode;//ѭ���������ɽڵ��ǰһ���ڵ�
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
