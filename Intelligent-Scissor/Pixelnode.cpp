#include "Pixelnode.h"
void PixelNode::GetNodeOffset(int& offsetX, int& offsetY, int linkIndex)
{
	/*
	*  321
	*  4 0
	*  567
	*/
	if (linkIndex == 0) {
		offsetX = 1;
		offsetY = 0;
	}
	else if (linkIndex == 1) {
		offsetX = 1;
		offsetY = -1;
	}
	else if (linkIndex == 2) {
		offsetX = 0;
		offsetY = -1;
	}
	else if (linkIndex == 3) {
		offsetX = -1;
		offsetY = -1;
	}
	else if (linkIndex == 4) {
		offsetX = -1;
		offsetY = 0;
	}
	else if (linkIndex == 5) {
		offsetX = -1;
		offsetY = 1;
	}
	else if (linkIndex == 6) {
		offsetX = 0;
		offsetY = 1;
	}
	else if (linkIndex == 7) {
		offsetX = 1;
		offsetY = 1;
	}
}
