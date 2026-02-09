export module BoardEvents;

#include "pvzclass/pvzclass/Memory.hpp"

import Index;
import <utility>;

class MyBoard : public PVZ::Board
{
public:
	MyBoard(uint32_t address) : PVZ::Board(address) {};
	MyBoard(const PVZ::Board& board) : PVZ::Board(board.GetBaseAddress()) {};

	/// @brief 对局是否正在进行
	T_SIMPLE_PROPERTY(uint8_t, MatchRunning, 0x55EA);
};

class BoardPickBackgroundAfterEvent : public DLLEventTemplate<0x40A7F1, 5, REG_EDX>
{
public:
	BoardPickBackgroundAfterEvent(const char* str) : DLLEventTemplate() { Init(str); };
	BoardPickBackgroundAfterEvent(int address) : DLLEventTemplate() { Init(address); };
	BoardPickBackgroundAfterEvent() : DLLEventTemplate() { Init("onBoardPickBackgroundAfter"); };
};

PVZ::PVZString prop_name;

void InitBackground(MyBoard board)
{
	auto app = board.GetPVZApp();

	if (!prop_name.isValid())
		prop_name = PVZ::PVZString::Make("Enabled");

	board.MatchRunning = app.GetBoolean(prop_name, false);

	if (board.MatchRunning)
	{
		auto lawn = board.GetLawn();
		lawn.SetRouteType(0, RouteType::NoZombie);

		for (int i = 0; i < 9; i++)
			lawn.SetGridType(0, i, LawnType::Unsodded);

		std::pair<int, int> disable_pos[]{ {1,1}, {1,2}, {1,3}, {2,1}, {2,3}, {3,1}, {3,2}, {4,0}, {4,1}, {4,2} };
		for (auto [row, col] : disable_pos)
			lawn.SetGridType(row, col, LawnType::Unsodded);
	}
}

export void InitBoardEvents()
{
	BoardPickBackgroundAfterEvent((int)InitBackground);
}