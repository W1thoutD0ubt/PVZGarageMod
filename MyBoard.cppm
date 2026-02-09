module;

#include "pvzclass/pvzclass/pvzclass.h"

export module MyBoard;

export class MyBoard : public PVZ::Board
{
public:
	MyBoard(uint32_t address) : PVZ::Board(address) {};
	MyBoard(const PVZ::Board& board) : PVZ::Board(board.GetBaseAddress()) {};

	/// @brief 对局是否正在进行
	T_SIMPLE_PROPERTY(uint8_t, garageEnabled, 0x55EA);
};