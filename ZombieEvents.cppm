export module ZombieEvents;

#define STRING(str) str,sizeof(str)/sizeof(*str)

import Index;
import MyBoard;
import MyZombie;
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

class ZombieCatapultDeathEvent : public BoolDLLEventTemplate<0x52EC00, 6, 0x52ED9E, 0x24, REG_EAX>
{
public:
	ZombieCatapultDeathEvent(const char* str) : BoolDLLEventTemplate() { Init(str); };
	ZombieCatapultDeathEvent(int address) : BoolDLLEventTemplate() { Init(address); };
};

/// @param 触发事件的 Zombie
class ZombieNotWalkingEvent : public ThreeStateEventTemplate<0x52A611, 6, 0x52A7A4, 0x52A79A, REG_EAX>
{
public:
	ZombieNotWalkingEvent(const char* str) : ThreeStateEventTemplate() { Init(str); };
	ZombieNotWalkingEvent(int address) : ThreeStateEventTemplate() { Init(address); };
};

/// @param 触发事件的 Board，判定的行，僵尸类型
class IsRowCanHaveZombieTypeEvent : public ThreeStateEventTemplate<0x40DB21, 7, 0x40DC41, 0x40DB4E, 0x2C, 0x2C, REG_ESI>
{
public:
	IsRowCanHaveZombieTypeEvent(const char* str) : ThreeStateEventTemplate() { Init(str); };
	IsRowCanHaveZombieTypeEvent(int address) : ThreeStateEventTemplate() { Init(address); };
};

/// @param 触发事件的子弹，子弹判定的僵尸
/// @return 若为负数，则按原版处理；若为 0，则为不可选中；若为正数，则为强制可选。
class ProjectileFindTargetRTEvent_ts : public ThreeStateEventTemplate<0x46CD95, 6, 0x46CDAD, 0x46CE58, REG_ESI, REG_EDI>
{
public:
	ProjectileFindTargetRTEvent_ts(const char* str) : ThreeStateEventTemplate() { Init(str); };
	ProjectileFindTargetRTEvent_ts(int address) : ThreeStateEventTemplate() { Init(address); };
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

PVZ::PVZString wait_countdown, unbox_countdown, basketball_countdown;
char idle1_anim_name[] = "anim_idle1";
char idle2_anim_name[] = "anim_idle2";
char retreat_anim_name[] = "anim_retreat";
bool onZombieUpdateAbility(MyZombie zombie)
{
	if (zombie.Type == ZombieType::CatapultZombie)
	{
		auto board = MyBoard(zombie.GetBoard());
		if (!board.garageEpisode1Enabled)
			return true;

		switch (zombie.State)
		{
		case ZombieState::WALKING:
			if (zombie.X <= 730)
			{
				zombie.State = ZombieState::BACK_CAR_SUMMON;
				zombie.PlayZombieReanimation(0x6580D4, PVZEnum::REANIM_LOOP, 10, 12.0f); // anim_idle
				zombie.AttributeCountdown = zombie.GetLawnApp().GetInteger(wait_countdown, 0);
			}
			break;
		case ZombieState::BACK_CAR_SUMMON:
			if (zombie.AttributeCountdown == zombie.GetLawnApp().GetInteger(unbox_countdown, 0))
			{
				zombie.PlayZombieReanimation(DWORD(idle1_anim_name), PVZEnum::REANIM_LOOP, 5, 12.0f); // anim_idle
			}
			if (zombie.AttributeCountdown <= 0)
			{
				zombie.State = ZombieState::BACK_CAR_RETREAT;
				zombie.PlayZombieReanimation(DWORD(idle2_anim_name), PVZEnum::REANIM_PLAY_ONCE_AND_HOLD, 10, 12.0f);

				auto spawn_pogo = [&](int row, float x_offset) {
					auto creep = Creator::CreateZombie(ZombieType::PogoZombie, row, 9);
					creep.X += x_offset;
					creep.Layer += 5;
				};

				spawn_pogo(zombie.Row - 1, 0);
				spawn_pogo(zombie.Row - 1, 15);
				spawn_pogo(zombie.Row, 0);
				spawn_pogo(zombie.Row, 15);
			}
			break;
		case ZombieState::BACK_CAR_RETREAT:
			if (zombie.GetAnimation().CycleCount >= 1 && zombie.Speed > 0)
			{
				zombie.PlayZombieReanimation(DWORD(retreat_anim_name), PVZEnum::REANIM_LOOP, 10, 12.0f);
				zombie.SetSpeed(-zombie.Speed);
			}
			break;
		case ZombieState::BACK_CAR_DYING:
			auto proj = PVZ::GetByID<PVZ::Projectile>(zombie.BasketballID);
			if (!proj.NotExist)
			{
				if (proj.X < 520 && proj.XSpeed != 0)
				{
					proj.XSpeed = 0;
					zombie.AttributeCountdown = zombie.GetLawnApp().GetInteger(basketball_countdown, 151);
				}
			}
			if (zombie.AttributeCountdown == 1)
			{
				PVZ::Rect blast_range = PVZ::Rect(proj.ImageX - 60, zombie.ImageY, 200, 80);
				for (auto plant : zombie.GetBoard().GetAllPlants())
				{
					if (plant.Row != zombie.Row)
						continue;

					auto rect = plant.GetPlantRect();
					if (PVZ::GetXOverlap(rect, blast_range) > 0)
						plant.Remove();
				}

				PVZ::CreateParticleSystem(proj.X, proj.Y, 400000, EffectType::JACK_BOX_EXPLODED);
				zombie.GetBoard().Earthquake(4, -6, 12);
				proj.Remove();

				PVZ::CreateParticleSystem(zombie.X + 80.0f, zombie.Y + 60.0f, zombie.Layer + 1, EffectType::ZOMBIE_BASKBALL_EXPLODED);
				zombie.RemoveWithLoot();
				zombie.GetLawnApp().PlayFoley(PVZEnum::FOLEY_EXPLOSION);
			}
			break;
		}
		return false;
	}
	else
		return true;
}

bool onCatapultDeath(MyZombie zombie, PVZ::DamageFlags flags)
{
	if (flags & 0x20)
		return true;

	auto basketball = Creator::CreateProjectile(ProjectileType::Basketball, zombie.Row, zombie.ImageX);
	basketball.DamageAbility = 0;
	basketball.XSpeed = -3.33f;
	basketball.Motion = MotionType::Float;
	basketball.RotationSpeed = 0;
	zombie.BasketballID = basketball.Id;
	zombie.State = ZombieState::BACK_CAR_DYING;
	zombie.AttributeCountdown = 0;
	zombie.BodyHealth = zombie.BodyMaxHealth;
	zombie.PlayZombieReanimation(DWORD("anim_bounce"), PVZEnum::REANIM_PLAY_ONCE_AND_HOLD, 2, 12.0f);

	return false;
}

int onZombieTakeDamage(PVZ::Zombie zombie, int& damageType, int damage)
{
	if (zombie.Type == ZombieType::CatapultZombie
			&& (zombie.State == ZombieState::BACK_CAR_RETREAT || zombie.State == ZombieState::BACK_CAR_DYING))
		return 0;

	return damage;
}

int onZombieNotWalking(PVZ::Zombie zombie)
{
	if (zombie.State == ZombieState::BACK_CAR_SUMMON || zombie.State == ZombieState::BACK_CAR_DYING
			|| (zombie.State == ZombieState::BACK_CAR_RETREAT && zombie.Speed > 0))
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

int onProjectileFindTargetRT(PVZ::Projectile projectile, PVZ::Zombie zombie)
{
	if (zombie.Type == ZombieType::CatapultZombie && projectile.Row - zombie.Row == -1)
		return ThreeState::Enable;

	return ThreeState::None;
}

class SquashDamageRTEvent_ts : public ThreeStateEventTemplate<0x460739, 6, 0x460749, 0x4607B7, REG_ESI, REG_EDI>
{
public:
	SquashDamageRTEvent_ts(const char* str) : ThreeStateEventTemplate() { Init(str); };
	SquashDamageRTEvent_ts(int address) : ThreeStateEventTemplate() { Init(address); };
};

export void InitZombieEvents()
{
	wait_countdown = PVZ::PVZString::Make("BackCarWaitCountdown");
	unbox_countdown = PVZ::PVZString::Make("BackCarUnboxCountdown");
	basketball_countdown = PVZ::PVZString::Make("BackCarExplodeCountdown");

	ZombieWalkEvent((int)GetZombieWalkDist);
	ZombieUpdateAbilityEvent((int)onZombieUpdateAbility);
	ZombieCatapultDeathEvent((int)onCatapultDeath);
	ZombieTakeDmgEvent((int)onZombieTakeDamage);
	ZombieNotWalkingEvent((int)onZombieNotWalking);
	IsRowCanHaveZombieTypeEvent((int)IsRowCanHaveZombieType);
	PVZEvent::PlantFindTargetRTEvent_ts((int)onPlantFindTargetRT);
	PVZEvent::SquashFindTargetRTEvent_ts((int)onSquashFindTargetRT);
	SquashDamageRTEvent_ts((int)onSquashFindTargetRT);
	ProjectileFindTargetRTEvent_ts((int)onProjectileFindTargetRT);
}