#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include "Scissor.h"
#include <iostream>
using namespace cv;
using namespace std;
Scissor scissor;//智能剪刀对象
int originSeedR, originSeedC;//第一次的种子点
vector<Point2i> paths;//储存最小路径
bool isClosed = false;//检测路径是否闭合
Mat img_Drawing, img;
int judge = 0;//是否使用Cursor Snapping
int judge1 = 0;//是否使用Path Cooling
void ShowResult(void);//根据路径完成抠图
void on_mouse(int EVENT, int x, int y, int flags, void* userdata)
{
	static std::vector<std::pair<Point2i, Vec4b>> lastPathAlpha;//第一个分量保存点，第二个分量保存rgba四通道，便于恢复像素点
	Mat* pOriginImage = reinterpret_cast<Mat*>(userdata);
	switch (EVENT)
	{
	case EVENT_MOUSEMOVE:
	{
		if (!scissor.IsSetSeed())
			break;
		//恢复之前的alpha值
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
			lastPathAlpha.push_back({ Point2i(node->row,node->column) ,(*ptr) });//储存原来的alpha值
			(*ptr) = { 0,255,0,255 };//在图像中以绿色点表示path
			if (judge1) node->count++;
		}
		);
		if (judge1) {
			//开始路径冷却
			PixelNode* thisnode;//用于保存当前处理的PixelNode
			CTypedPtrDblElement <PixelNode>* tail = path.GetTailPtr();//反向遍历
			CTypedPtrDblElement <PixelNode>* mouse = tail;//用于对路径进行反向遍历
			while (mouse->Data() != nullptr) {
				thisnode = mouse->Data();
				if (thisnode->count > 250) {
					on_mouse(EVENT_LBUTTONUP, thisnode->column, thisnode->row, NULL, userdata);//设置为种子点
					cout << "Using Path Cooling (" << thisnode->column << " , " << thisnode->row << ") has been set to seed point" << endl;
					mouse = tail;//反向清除count
					while (mouse->Data() != nullptr)
					{
						thisnode = mouse->Data();
						thisnode->count = 0;
						mouse = mouse->Prev();
					}
					on_mouse(EVENT_LBUTTONUP, x, y, NULL, userdata);
					break;
				}
				mouse = mouse->Prev();//遍历，找出距离种子点最远的点设为种子点
			}
		}
		break;
	}
	break;
	case EVENT_LBUTTONUP:
	{
		if (!scissor.IsSetSeed())//记录第一次设置的seed，调用一次s.LiveWireDP后IsSetSeed==True
		{
			originSeedC = x;
			originSeedR = y;
		}
		//将path的终点设为cursor snap给出的点
		//Using cursor snap
		int CURSORSNAP_WIDTH = 7;
		Point2i cs_point(x, y);
		if (judge) {
			cs_point = scissor.CursorSnap(y, x, CURSORSNAP_WIDTH);
			//这里暂时不使用Path Cooling
			int temp = judge1;//
			judge1 = 0;
			//Update path to display intact green lines
			on_mouse(EVENT_MOUSEMOVE, cs_point.x, cs_point.y, NULL, userdata);
			judge1 = temp;//恢复
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
		cout << "是否已经闭合？" << (isClosed ? "是" : "否") << endl;
		break;
	}
	break;
	case EVENT_RBUTTONUP://自动闭合路径
	{
		on_mouse(EVENT_LBUTTONUP, x, y, NULL, userdata);
		on_mouse(EVENT_MOUSEMOVE, originSeedC, originSeedR, NULL, userdata);
		on_mouse(EVENT_LBUTTONUP, originSeedC, originSeedR, NULL, userdata);//从当前点运动到初始点，并闭合路径
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
	//计算代价图
	scissor = Scissor(img);
	scissor.ComputeFzCostMap();
	scissor.ComputeFgCostMap();
	scissor.ComputeFdCostMap();
	//展示代价图
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
	//imshow("Cost Picture",combine);//展示三个代价
	//imshow("costmap", costmap);//展示最终合成的代价图
	//开始抠图
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
		cout << "抠图尚且未闭合，自动闭合路径中...." << endl;
		on_mouse(EVENT_MOUSEMOVE, originSeedC, originSeedR, NULL, &img_Drawing);
		on_mouse(EVENT_LBUTTONUP, originSeedC, originSeedR, NULL, &img_Drawing);
		cout << "闭合成功，正在抠图...." << endl;
	}
	//先对所有顶点进行排序操作，然后取x,y的最大值最小值，形成举行区域，然后按扫描线算法将图像抠出来就行了
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