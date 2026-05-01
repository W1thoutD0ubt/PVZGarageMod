module;

#include "pvzclass/pvzclass/pvzclass.h"

export module MyBoard;

export class MyBoard : public PVZ::Board
{
public:
	MyBoard(uint32_t address) : PVZ::Board(address) {};
	MyBoard(const PVZ::Board& board) : PVZ::Board(board.GetBaseAddress()) {};

	/// @brief 是否启用车库第一章特性
	T_SIMPLE_PROPERTY(uint8_t, garageEpisode1Enabled, 0x55EA);
};