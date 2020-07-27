#pragma once
#define INITIAL 0
#define ACTIVE  1
#define EXPANDED 2
class PixelNode
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
	int pqIndex;
	int Index(void) const {
		return pqIndex;
	}
	int& Index(void) {
		return pqIndex;
	}
	int count = 0;//记录被选中的次数，用于路径冷却
};
inline int operator < (const PixelNode& a, const PixelNode& b)
{
	return a.totalCost < b.totalCost;
}