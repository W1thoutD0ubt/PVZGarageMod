module;

#include "pvzclass/pvzclass/pvzclass.h"

export module MyZombie;

export class MyZombie : public PVZ::Zombie
{
public:
	MyZombie(uint32_t address) : PVZ::Zombie(address) {};
	MyZombie(const PVZ::Zombie& zombie) : PVZ::Zombie(zombie.GetBaseAddress()) {};

	/// @brief 篮球子弹识别ID
	INT_SIMPLE_PROPERTY(BasketballID, 0x148);
};
