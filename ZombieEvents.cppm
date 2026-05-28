export module ZombieEvents;

#define STRING(str) str,sizeof(str)/sizeof(*str)

import Index;
import MyBoard;
import Const;
import <cmath>;
import <utility>;

class ZombieWalkEvent : public FloatDLLEventTemplate<0x52AB18, 5, 0x28, false, 0x28, REG_ESI>
{
public:
	ZombieWalkEvent(const char* str) : FloatDLLEventTemplate() { Init(str); };
	ZombieWalkEvent(int address) : FloatDLLEventTemplate() { Init(address); };
	ZombieWalkEvent() : FloatDLLEventTemplate() { Init("GetZombieWalkDis"); };
};

/// @brief Zombie 行为动作的更新。
/// @note 时机上先于原版的更新。
/// @note 该事件与 ZombieUpdateActionEvent 不同，只影响僵尸自身的技能。
/// @param 更新的 Zombie
/// @return 是否更新原版行为动作
/// @retval false 完全跳过原版的任何行为动作，这会导致原生技能失效。
class ZombieUpdateAbilityEvent : public DLLEventTemplate<0x52B174, 6, REG_ESI>
{
public:
	ZombieUpdateAbilityEvent() : DLLEventTemplate() { Init("onZombieUpdateAbility"); };
	ZombieUpdateAbilityEvent(const char* str) : DLLEventTemplate() { Init(str); };
	ZombieUpdateAbilityEvent(int address) : DLLEventTemplate() { Init(address); };
protected:
	void InitExtra(AsmBuilder& builder)
	{
		static constexpr byte after[] =
		{
			0x84, 0xC0,
			0x75, 7,

			0x61,
			0x68, (byte)(0x52B278 & 0xFF), (byte)((0x52B278 & 0xFF00) >> 8), (byte)((0x52B278 & 0xFF0000) >> 16), (byte)((0x52B278 & 0xFF000000) >> 24),
			0xC3,

			0x61,
			0x83, 0x78 + (REG_ESI), 0x24, 3,
			0x75, 6,

			0x68, (byte)(0x52B17A & 0xFF), (byte)((0x52B17A & 0xFF00) >> 8), (byte)((0x52B17A & 0xFF0000) >> 16), (byte)((0x52B17A & 0xFF000000) >> 24),
			0xC3,
			0x68, (byte)(0x52B180 & 0xFF), (byte)((0x52B180 & 0xFF00) >> 8), (byte)((0x52B180 & 0xFF0000) >> 16), (byte)((0x52B180 & 0xFF000000) >> 24),
			0xC3,
		};
		builder.add_bytes(STRING(after));
	}
};

class ZombieCatapultDeathEvent : public DLLEventTemplate<0x52EC00, 6, 0x24, REG_EAX>
{
public:
	ZombieCatapultDeathEvent(const char* str) : DLLEventTemplate() { Init(str); };
	ZombieCatapultDeathEvent(int address) : DLLEventTemplate() { Init(address); };
};

/// @param 触发事件的 Zombie
class ZombieNotWalkingEvent : public ThreeStateEventTemplate<0x52A611, 6, 0x52A7A4, 0x52A79A, REG_EAX>
{
public:
	ZombieNotWalkingEvent(const char* str) : ThreeStateEventTemplate() { Init(str); };
	ZombieNotWalkingEvent(int address) : ThreeStateEventTemplate() { Init(address); };
};

/// @param 触发事件的 Zombie
class ZombieIsBackwardEvent : public ThreeStateEventTemplate<0x52BEE0, 7, 0x52BEE9, 0x52BF5A, REG_ECX>
{
public:
	ZombieIsBackwardEvent(const char* str) : ThreeStateEventTemplate() { Init(str); };
	ZombieIsBackwardEvent(int address) : ThreeStateEventTemplate() { Init(address); };
};

/// @param 触发事件的 Board，判定的行，僵尸类型
class IsRowCanHaveZombieTypeEvent : public ThreeStateEventTemplate<0x40DB21, 7, 0x40DB4E, 0x40DC41, 0x2C, 0x2C, REG_ESI>
{
public:
	IsRowCanHaveZombieTypeEvent(const char* str) : ThreeStateEventTemplate() { Init(str); };
	IsRowCanHaveZombieTypeEvent(int address) : ThreeStateEventTemplate() { Init(address); };
};

inline constexpr auto Yvelocity = 0.05f;

float GetZombieWalkDist(PVZ::Zombie zombie, float dist)
{
	MyBoard board = zombie.GetBoard();
	if (board.garageEpisode1Enabled)
		if (350 < zombie.X && zombie.X < 370)
		{
			zombie.Row = 2;
			if (std::fabs(zombie.Y - board.GridToYPixel(2, 4)) < 1.0f)
				return 0;
		}
		
	return dist;
}

PVZ::PVZString wait_countdown, unbox_countdown;
bool onZombieUpdateAbility(PVZ::Zombie zombie)
{
	if (zombie.Type == ZombieType::CatapultZombie)
	{
		auto board = MyBoard(zombie.GetBoard());
		if (!board.garageEpisode2Enabled)
			return true;

		switch (zombie.State)
		{
		case ZombieState::WALKING:
			if (zombie.X <= 730)
			{
				zombie.State = ZombieState::BACK_CAR_SUMMON;
				zombie.AttributeCountdown = zombie.GetLawnApp().GetInteger(wait_countdown, 0);
			}
			break;
		case ZombieState::BACK_CAR_SUMMON:
			if (zombie.AttributeCountdown < zombie.GetLawnApp().GetInteger(unbox_countdown, 0))
			{
				zombie.State = ZombieState::BACK_CAR_RETREAT;
				Creator::CreateZombie(ZombieType::PogoZombie, zombie.Row - 1, 9);
				auto creep = Creator::CreateZombie(ZombieType::PogoZombie, zombie.Row - 1, 9);
				creep.X += 15;
				Creator::CreateZombie(ZombieType::PogoZombie, zombie.Row, 9);
				creep = Creator::CreateZombie(ZombieType::PogoZombie, zombie.Row, 9);
				creep.X += 15;
			}
			break;
		}
		return false;
	}
	else
		return true;
}

void onCatapultDeath(PVZ::Zombie zombie, PVZ::DamageFlags flags)
{
	if (flags & 0x20)
		return;

	PVZ::Rect blast_range = PVZ::Rect(zombie.ImageX - 240, zombie.ImageY, 240, 80);

	for (auto plant : zombie.GetBoard().GetAllPlants())
	{
		if (plant.Row != zombie.Row)
			continue;

		auto rect = plant.GetPlantRect();
		if (PVZ::GetXOverlap(rect, blast_range) > 0)
			plant.Remove();
	}
}

int onZombieTakeDamage(PVZ::Zombie zombie, int& damageType, int damage)
{
	if (zombie.Type == ZombieType::CatapultZombie
			&& zombie.State == ZombieState::BACK_CAR_RETREAT)
		return 0;

	return damage;
}

int onZombieNotWalking(PVZ::Zombie zombie)
{
	if (zombie.State == ZombieState::BACK_CAR_SUMMON)
		return ThreeState::Enable;

	return ThreeState::None;
}

int onZombieIsBackward(PVZ::Zombie zombie)
{
	if (zombie.State == ZombieState::BACK_CAR_RETREAT)
		return ThreeState::Enable;

	return ThreeState::None;
}

int IsRowCanHaveZombieType(PVZ::Board board, int row, ZombieType::ZombieType type)
{
	if (type == ZombieType::CatapultZombie)
	{
		if (row > 1)
		{
			auto lawn = board.GetLawn();
			if (lawn.GetRouteType(row) == RouteType::Land
					&& lawn.GetRouteType(row - 1) == RouteType::Land)
				return ThreeState::None;
		}
		return ThreeState::Disable;
	}
	return ThreeState::None;
}

int onPlantFindTargetRT(PVZ::Plant plant, PVZ::Zombie zombie, int targetRow)
{
	if (zombie.Type == ZombieType::CatapultZombie && targetRow - zombie.Row == -1)
		return ThreeState::Enable;

	return ThreeState::None;
}

int onSquashFindTargetRT(PVZ::Plant plant, PVZ::Zombie zombie)
{
	if (zombie.Type == ZombieType::CatapultZombie && plant.Row - zombie.Row == -1)
		return ThreeState::Enable;

	return ThreeState::None;
}

export void InitZombieEvents()
{
	wait_countdown = PVZ::PVZString::Make("BackCarWaitCountdown");
	unbox_countdown = PVZ::PVZString::Make("BackCarUnboxCountdown");

	ZombieWalkEvent((int)GetZombieWalkDist);
	ZombieUpdateAbilityEvent((int)onZombieUpdateAbility);
	ZombieCatapultDeathEvent((int)onCatapultDeath);
	ZombieTakeDmgEvent((int)onZombieTakeDamage);
	ZombieNotWalkingEvent((int)onZombieNotWalking);
	ZombieIsBackwardEvent((int)onZombieIsBackward);
	IsRowCanHaveZombieTypeEvent((int)IsRowCanHaveZombieType);
	PVZEvent::PlantFindTargetRTEvent_ts((int)onPlantFindTargetRT);
	PVZEvent::SquashFindTargetRTEvent_ts((int)onSquashFindTargetRT);
}