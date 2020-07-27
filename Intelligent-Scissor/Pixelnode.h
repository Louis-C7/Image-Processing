#pragma once
#define INITIAL 0
#define ACTIVE  1
#define EXPANDED 2
class PixelNode
{
public:
	int column, row;//��ʾ������ھ����е�������
	double linkCost[8];//�����Ȩֵ
	int state;
	double totalCost;
	PixelNode* prevNode;//�����γ���С·��
	PixelNode() : linkCost{ 0,0,0,0,0,0,0,0 },
		prevNode(nullptr),
		column(0),
		row(0),
		state(INITIAL),
		totalCost(0)
	{}
	void GetNodeOffset(int& offsetX, int& offsetY, int linkIndex);//������0,1,2,3,4,5,6,7�л�ȡƫ����
	int pqIndex;
	int Index(void) const {
		return pqIndex;
	}
	int& Index(void) {
		return pqIndex;
	}
	int count = 0;//��¼��ѡ�еĴ���������·����ȴ
};
inline int operator < (const PixelNode& a, const PixelNode& b)
{
	return a.totalCost < b.totalCost;
}