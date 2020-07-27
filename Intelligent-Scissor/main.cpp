#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include "Scissor.h"
#include <iostream>
using namespace cv;
using namespace std;
Scissor scissor;//���ܼ�������
int originSeedR, originSeedC;//��һ�ε����ӵ�
vector<Point2i> paths;//������С·��
bool isClosed = false;//���·���Ƿ�պ�
Mat img_Drawing, img;
int judge = 0;//�Ƿ�ʹ��Cursor Snapping
int judge1 = 0;//�Ƿ�ʹ��Path Cooling
void ShowResult(void);//����·����ɿ�ͼ
void on_mouse(int EVENT, int x, int y, int flags, void* userdata)
{
	static std::vector<std::pair<Point2i, Vec4b>> lastPathAlpha;//��һ����������㣬�ڶ�����������rgba��ͨ�������ڻָ����ص�
	Mat* pOriginImage = reinterpret_cast<Mat*>(userdata);
	switch (EVENT)
	{
	case EVENT_MOUSEMOVE:
	{
		if (!scissor.IsSetSeed())
			break;
		//�ָ�֮ǰ��alphaֵ
		for (auto p : lastPathAlpha) {
			Vec4b* ptr = pOriginImage->ptr<Vec4b>(p.first.x, p.first.y);
			(*ptr) = p.second;
		}
		lastPathAlpha.clear();
		CTypedPtrDblList<PixelNode> path;
		scissor.CalculateMininumPath(path, y, x);
		path.Do(
			[&](PixelNode* node)->void {
			Vec4b* ptr = pOriginImage->ptr<Vec4b>(node->row, node->column);
			lastPathAlpha.push_back({ Point2i(node->row,node->column) ,(*ptr) });//����ԭ����alphaֵ
			(*ptr) = { 0,255,0,255 };//��ͼ��������ɫ���ʾpath
			if (judge1) node->count++;
		}
		);
		if (judge1) {
			//��ʼ·����ȴ
			PixelNode* thisnode;//���ڱ��浱ǰ�����PixelNode
			CTypedPtrDblElement <PixelNode>* tail = path.GetTailPtr();//�������
			CTypedPtrDblElement <PixelNode>* mouse = tail;//���ڶ�·�����з������
			while (mouse->Data() != nullptr) {
				thisnode = mouse->Data();
				if (thisnode->count > 250) {
					on_mouse(EVENT_LBUTTONUP, thisnode->column, thisnode->row, NULL, userdata);//����Ϊ���ӵ�
					cout << "Using Path Cooling (" << thisnode->column << " , " << thisnode->row << ") has been set to seed point" << endl;
					mouse = tail;//�������count
					while (mouse->Data() != nullptr)
					{
						thisnode = mouse->Data();
						thisnode->count = 0;
						mouse = mouse->Prev();
					}
					on_mouse(EVENT_LBUTTONUP, x, y, NULL, userdata);
					break;
				}
				mouse = mouse->Prev();//�������ҳ��������ӵ���Զ�ĵ���Ϊ���ӵ�
			}
		}
		break;
	}
	break;
	case EVENT_LBUTTONUP:
	{
		if (!scissor.IsSetSeed())//��¼��һ�����õ�seed������һ��s.LiveWireDP��IsSetSeed==True
		{
			originSeedC = x;
			originSeedR = y;
		}
		//��path���յ���Ϊcursor snap�����ĵ�
		//Using cursor snap
		int CURSORSNAP_WIDTH = 7;
		Point2i cs_point(x, y);
		if (judge) {
			cs_point = scissor.CursorSnap(y, x, CURSORSNAP_WIDTH);
			//������ʱ��ʹ��Path Cooling
			int temp = judge1;//
			judge1 = 0;
			//Update path to display intact green lines
			on_mouse(EVENT_MOUSEMOVE, cs_point.x, cs_point.y, NULL, userdata);
			judge1 = temp;//�ָ�
		}
		for (auto p : lastPathAlpha)
		{
			if (!paths.empty() && p.first == paths[0])
			{
				cout << paths[0] << "  " << p.first << endl;
				isClosed = true;
			}
			paths.push_back(p.first);
		}
		lastPathAlpha.clear();
		scissor.LiveWireDP(cs_point.y, cs_point.x);
		cout << "�Ƿ��Ѿ��պϣ�" << (isClosed ? "��" : "��") << endl;
		break;
	}
	break;
	case EVENT_RBUTTONUP://�Զ��պ�·��
	{
		on_mouse(EVENT_LBUTTONUP, x, y, NULL, userdata);
		on_mouse(EVENT_MOUSEMOVE, originSeedC, originSeedR, NULL, userdata);
		on_mouse(EVENT_LBUTTONUP, originSeedC, originSeedR, NULL, userdata);//�ӵ�ǰ���˶�����ʼ�㣬���պ�·��
		std::sort(paths.begin(), paths.end(), [](const Point2i& lhs, const Point2i& rhs)
		{ return lhs.x > rhs.x || (lhs.x == rhs.x && lhs.y > rhs.y); });
	}
	break;
	default:
		break;
	}
	imshow("Drawing Picture", img_Drawing);
}
int main(int argc, char* argv[])
{
	img = imread("human.jpg");
	int rows = img.rows, cols = img.cols;
	cvtColor(img, img_Drawing, COLOR_BGR2BGRA);
	//�������ͼ
	scissor = Scissor(img);
	scissor.ComputeFzCostMap();
	scissor.ComputeFgCostMap();
	scissor.ComputeFdCostMap();
	//չʾ����ͼ
	Mat fz, fg, fd, costmap;
	cvtColor(scissor.FzCostMap, fz, COLOR_GRAY2BGRA);
	cvtColor(scissor.FgCostMap, fg, COLOR_GRAY2BGRA);
	cvtColor(scissor.FdCostMap, fd, COLOR_GRAY2BGRA);
	costmap = scissor.MakeCostImage();
	Mat img_showed;
	img_Drawing.convertTo(img_showed, CV_32FC4);
	normalize(img_Drawing, img_showed, 1, 0, NORM_MINMAX, CV_32FC4);
	Mat combine1, combine2, combine;
	hconcat(img_showed, fz, combine1);
	hconcat(fg, fd, combine2);
	vconcat(combine1, combine2, combine);
	imshow("Drawing Picture", img_Drawing);
	//imshow("Cost Picture",combine);//չʾ��������
	//imshow("costmap", costmap);//չʾ���պϳɵĴ���ͼ
	//��ʼ��ͼ
	setMouseCallback("Drawing Picture", on_mouse, &img_Drawing);
	createTrackbar("Cooling", "Drawing Picture", &judge1, 1);
	createTrackbar("Snapping", "Drawing Picture", &judge, 1);

	waitKey(0);
	ShowResult();
	return 0;
}
void ShowResult(void)
{
	if (isClosed)
	{
		cout << "��ͼ����δ�պϣ��Զ��պ�·����...." << endl;
		on_mouse(EVENT_MOUSEMOVE, originSeedC, originSeedR, NULL, &img_Drawing);
		on_mouse(EVENT_LBUTTONUP, originSeedC, originSeedR, NULL, &img_Drawing);
		cout << "�պϳɹ������ڿ�ͼ...." << endl;
	}
	//�ȶ����ж���������������Ȼ��ȡx,y�����ֵ��Сֵ���γɾ�������Ȼ��ɨ�����㷨��ͼ��ٳ���������
	std::sort(paths.begin(), paths.end(), [](const Point2i& lhs, const Point2i& rhs) ->bool
	{ return lhs.x > rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y); });
	auto MaxRow = paths.front().x;
	auto MinRow = paths.back().x;
	int MaxCol = INT_MIN;
	int MinCol = INT_MAX;
	for (auto p : paths)
	{
		if (p.y > MaxCol)
			MaxCol = p.y;
		if (p.y < MinCol)
			MinCol = p.y;
	}
	Mat cutImage;
	cutImage.create(MaxRow - MinRow + 10, MaxCol - MinCol + 10, CV_8UC3);

	int rowIndex = MaxRow;
	int colBegin = paths.front().y;
	int colEnd;
	auto NextRowIndexOp = [&](const Point2i& p) -> bool { return p.x != rowIndex; };
	auto iter = find_if(paths.begin(), paths.end(), NextRowIndexOp);

	while (iter != paths.cend())
	{
		colEnd = (iter - 1)->y;
		using RGB = Scissor::RGB_img;
		for (; colBegin <= colEnd; colBegin++)
		{
			*(cutImage.ptr<RGB>(rowIndex - MinRow, colBegin - MinCol))
				= *img.ptr<RGB>(rowIndex, colBegin);
		}
		rowIndex = iter->x;
		colBegin = iter->y;
		iter = find_if(iter, paths.end(), NextRowIndexOp);
	}
	imshow("Cut Image", cutImage);
	waitKey(0);
}