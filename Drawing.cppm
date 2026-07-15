export module Drawing;

import Index;
import MyBoard;
import GarageResources;

void onDrawUITop(MyBoard board, PVZ::Graphics g)
{
	g.TodDrawImageScaledF(IMAGE_GARAGE_DOOR, -77, 26, 1, 1);
}

export void InitDrawing()
{
	PVZEvent::DrawUICoinBankEvent((int)onDrawUITop);
}