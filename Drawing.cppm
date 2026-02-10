export module Drawing;

import Index;
import MyBoard;
import GarageResources;

void onDrawUITop(MyBoard board, PVZ::Graphics g)
{
	g.DrawImage(IMAGE_GARAGE_DOOR, 0, 0);
}

export void InitDrawing()
{
	PVZEvent::DrawUICoinBankEvent((int)onDrawUITop);
}