#include "aimbot.h"
#include "autowall.h"

#include "../Utils/xorstring.h"
#include "../Utils/math.h"
#include "../Utils/entity.h"
#include "../Utils/bonemaps.h"
#include "../settings.h"
#include "../interfaces.h"

bool Aimbot::aimStepInProgress = false;
std::vector<int64_t> Aimbot::friends = { };
std::vector<long> killTimes = { 0 }; // the Epoch time from when we kill someone

bool shouldAim;
QAngle AimStepLastAngle;
QAngle RCSLastPunch;

int Aimbot::targetAimbot = -1;
const int headVectors = 11;

static QAngle ApplyErrorToAngle(QAngle* angles, float margin)
{
	QAngle error;
	error.Random(-1.0f, 1.0f);
	error *= margin;
	angles->operator+=(error);
	return error;
}

/* Fills points Vector. True if successful. False if not.  Credits for Original method - ReactiioN */
static bool HeadMultiPoint(C_BasePlayer *player, Vector points[])
{
	matrix3x4_t matrix[128];

	if( !player->SetupBones(matrix, 128, 0x100, 0.f) )
		return false;
	model_t *pModel = player->GetModel();
	if( !pModel )
		return false;

	studiohdr_t *hdr = modelInfo->GetStudioModel(pModel);
	if( !hdr )
		return false;
	mstudiobbox_t *bbox = hdr->pHitbox((int)Hitbox::HITBOX_HEAD, 0);
	if( !bbox )
		return false;

	Vector mins, maxs;
	Math::VectorTransform(bbox->bbmin, matrix[bbox->bone], mins);
	Math::VectorTransform(bbox->bbmax, matrix[bbox->bone], maxs);

	Vector center = ( mins + maxs ) * 0.5f;
	// 0 - center, 1 - forehead, 2 - skullcap, 3 - upperleftear, 4 - upperrightear, 5 - uppernose, 6 - upperbackofhead
	// 7 - leftear, 8 - rightear, 9 - nose, 10 - backofhead
	for( int i = 0; i < headVectors; i++ ) // set all points initially to center mass of head.
		points[i] = center;
	points[1].z += bbox->radius * 0.60f; // morph each point.
	points[2].z += bbox->radius * 1.25f; // ...
	points[3].x += bbox->radius * 0.80f;
	points[3].z += bbox->radius * 0.60f;
	points[4].x -= bbox->radius * 0.80f;
	points[4].z += bbox->radius * 0.90f;
	points[5].y += bbox->radius * 0.80f;
	points[5].z += bbox->radius * 0.90f;
	points[6].y -= bbox->radius * 0.80f;
	points[6].z += bbox->radius * 0.90f;
	points[7].x += bbox->radius * 0.80f;
	points[8].x -= bbox->radius * 0.80f;
	points[9].y += bbox->radius * 0.80f;
	points[10].y -= bbox->radius * 0.80f;

	return true;
}
static float AutoWallBestSpot(C_BasePlayer *player, Vector &bestSpot)
{
	float bestDamage = Settings::Aimbot::AutoWall::value;
	const std::unordered_map<int, int> *modelType = BoneMaps::GetModelTypeBoneMap(player);

	static int len = sizeof(Settings::Aimbot::AutoAim::desiredBones) / sizeof(Settings::Aimbot::AutoAim::desiredBones[0]);

	for( int i = 0; i < len; i++ )
	{
		if( !Settings::Aimbot::AutoAim::desiredBones[i] )
			continue;
		if( i == CONST_BONE_HEAD ) // head multipoint
		{
			Vector headPoints[headVectors];
			if( !HeadMultiPoint(player, headPoints) )
				continue;
			for( int j = 0; j < headVectors; j++ )
			{
				Autowall::FireBulletData data;
				float spotDamage = Autowall::GetDamage(headPoints[j], !Settings::Aimbot::friendly, data);
				if( spotDamage > bestDamage )
				{
					bestSpot = headPoints[j];
					if( spotDamage > player->GetHealth() )
						return spotDamage;
					bestDamage = spotDamage;
				}
			}
		}
		int boneID = (*modelType).at(i);
		if( boneID == BONE_INVALID ) // bone not available on this modeltype.
			continue;

		Vector bone3D = player->GetBonePosition(boneID);

		Autowall::FireBulletData data;
		float boneDamage = Autowall::GetDamage(bone3D, !Settings::Aimbot::friendly, data);
		if( boneDamage > bestDamage )
		{
			bestSpot = bone3D;
			if( boneDamage > player->GetHealth() )
				return boneDamage;

			bestDamage = boneDamage;
		}
	}
	return bestDamage;
}

static float GetRealDistanceFOV(float distance, QAngle angle, CUserCmd* cmd)
{
	/*    n
	    w + e
	      s        'real distance'
	                      |
	   a point -> x --..  v
	              |     ''-- x <- a guy
	              |          /
	             |         /
	             |       /
	            | <------------ both of these lines are the same length
	            |    /      /
	           |   / <-----'
	           | /
	          o
	     localplayer
	*/

	Vector aimingAt;
	Math::AngleVectors(cmd->viewangles, aimingAt);
	aimingAt *= distance;

	Vector aimAt;
	Math::AngleVectors(angle, aimAt);
	aimAt *= distance;

	return aimingAt.DistTo(aimAt);
}

static Vector VelocityExtrapolate(C_BasePlayer* player, Vector aimPos)
{
	return aimPos + (player->GetVelocity() * globalVars->interval_per_tick);
}

/* Original Credits to: https://github.com/goldenguy00 ( study! study! study! :^) ) */
static Vector GetClosestSpot( CUserCmd* cmd, C_BasePlayer* localPlayer, C_BasePlayer* enemy, AimTargetType aimTargetType = AimTargetType::FOV)
{
	QAngle viewAngles;
	engine->GetViewAngles(viewAngles);

	float tempFov = Settings::Aimbot::AutoAim::fov;
	float tempDistance = Settings::Aimbot::AutoAim::fov * 5.f;

	Vector pVecTarget = localPlayer->GetEyePosition();

	Vector tempSpot = {0,0,0};

	const std::unordered_map<int, int> *modelType = BoneMaps::GetModelTypeBoneMap(enemy);

	static int len = sizeof(Settings::Aimbot::AutoAim::desiredBones) / sizeof(Settings::Aimbot::AutoAim::desiredBones[0]);
	for( int i = 0; i < len; i++ )
	{
		if( !Settings::Aimbot::AutoAim::desiredBones[i] )
			continue;

		int boneID = (*modelType).at(i);
		if( boneID == BONE_INVALID )
			continue;

		Vector cbVecTarget = enemy->GetBonePosition(boneID);

		if( aimTargetType == AimTargetType::FOV )
		{
			float cbFov = Math::GetFov(viewAngles, Math::CalcAngle(pVecTarget, cbVecTarget));

			if( cbFov < tempFov )
			{
				if( Entity::IsVisibleThroughEnemies(enemy, boneID) )
				{
					tempFov = cbFov;
					tempSpot = cbVecTarget;
				}
			}
		}
		else if( aimTargetType == AimTargetType::REAL_DISTANCE )
		{
			float cbDistance = pVecTarget.DistTo(cbVecTarget);
			float cbRealDistance = GetRealDistanceFOV(cbDistance, Math::CalcAngle(pVecTarget, cbVecTarget), cmd);

			if( cbRealDistance < tempDistance )
			{
				if( Entity::IsVisibleThroughEnemies(enemy, boneID) )
				{
					tempDistance = cbRealDistance;
					tempSpot = cbVecTarget;
				}
			}
		}
	}
	return tempSpot;
}

static C_BasePlayer* GetClosestPlayerAndSpot(CUserCmd* cmd, bool visibleCheck, Vector* bestSpot, float* bestDamage, AimTargetType aimTargetType = AimTargetType::FOV)
{
	if (Settings::Aimbot::AutoAim::realDistance)
		aimTargetType = AimTargetType::REAL_DISTANCE;

	static C_BasePlayer* lockedOn = nullptr;
	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	C_BasePlayer* closestEntity = nullptr;

	float bestFov = Settings::Aimbot::AutoAim::fov;
	float bestRealDistance = Settings::Aimbot::AutoAim::fov * 5.f;

	if( lockedOn )
	{
		if( lockedOn->GetAlive() && !Settings::Aimbot::AutoAim::closestBone && !Entity::IsSpotVisibleThroughEnemies(lockedOn, lockedOn->GetBonePosition((int)Settings::Aimbot::bone)) )
		{
			lockedOn = nullptr;
			return nullptr;
		}
		if (!(cmd->buttons & IN_ATTACK || inputSystem->IsButtonDown(Settings::Aimbot::aimkey)) || lockedOn->GetDormant())//|| !Entity::IsVisible(lockedOn, bestBone, 180.f, Settings::ESP::Filters::smokeCheck))
		{
			lockedOn = nullptr;
		}
		else
		{
			if( !lockedOn->GetAlive() )
			{
				if( Settings::Aimbot::AutoAim::engageLockTR )
				{
					if(Util::GetEpochTime() - killTimes.back() > Settings::Aimbot::AutoAim::engageLockTTR) // if we got the kill over the TTR time, engage another foe.
					{
						lockedOn = nullptr;
					}
				}
				return nullptr;
			}

			if( Settings::Aimbot::AutoAim::closestBone )
			{
				Vector tempSpot = GetClosestSpot(cmd, localplayer, lockedOn, aimTargetType);
				if( tempSpot.IsZero() )
				{
					return nullptr;
				}
				*bestSpot = tempSpot;
			}
			else
			{
				*bestSpot = lockedOn->GetBonePosition((int)Settings::Aimbot::bone);
			}

			return lockedOn;
		}
	}

	for (int i = 1; i < engine->GetMaxClients(); ++i)
	{
		C_BasePlayer* player = (C_BasePlayer*) entityList->GetClientEntity(i);

		if (!player
			|| player == localplayer
			|| player->GetDormant()
			|| !player->GetAlive()
			|| player->GetImmune())
			continue;

		if (!Settings::Aimbot::friendly && Entity::IsTeamMate(player, localplayer))
			continue;

		if( !Aimbot::friends.empty() ) // check for friends, if any
		{
			IEngineClient::player_info_t entityInformation;
			engine->GetPlayerInfo(i, &entityInformation);

			if (std::find(Aimbot::friends.begin(), Aimbot::friends.end(), entityInformation.xuid) != Aimbot::friends.end())
				continue;
		}

		Aimbot::targetAimbot = i;
		Vector eVecTarget = player->GetBonePosition((int) Settings::Aimbot::bone);
		if( Settings::Aimbot::AutoAim::closestBone )
		{
			Vector tempSpot = GetClosestSpot(cmd, localplayer, player, aimTargetType);
			if( tempSpot.IsZero() || !Entity::IsSpotVisibleThroughEnemies(player, tempSpot) )
				continue;
			eVecTarget = tempSpot;
		}

		Vector pVecTarget = localplayer->GetEyePosition();
        lastRayStart = pVecTarget;
        lastRayEnd = eVecTarget;

		QAngle viewAngles;
		engine->GetViewAngles(viewAngles);

		float distance = pVecTarget.DistTo(eVecTarget);
		float fov = Math::GetFov(viewAngles, Math::CalcAngle(pVecTarget, eVecTarget));

		if (aimTargetType == AimTargetType::FOV && fov > bestFov)
			continue;

		float realDistance = GetRealDistanceFOV(distance, Math::CalcAngle(pVecTarget, eVecTarget), cmd);

		if (aimTargetType == AimTargetType::REAL_DISTANCE && realDistance > bestRealDistance)
			continue;
		if (visibleCheck && !Settings::Aimbot::AutoWall::enabled && !Entity::IsSpotVisible(player, eVecTarget))
			continue;
		if ( Settings::Aimbot::SmokeCheck::enabled && LineGoesThroughSmoke( localplayer->GetEyePosition( ), eVecTarget, true ) )
			continue;
		if ( Settings::Aimbot::FlashCheck::enabled && localplayer->IsFlashed() )
			continue;

		if (Settings::Aimbot::AutoWall::enabled)
		{
			Vector wallBangSpot = {0,0,0};
			float damage = AutoWallBestSpot(player, wallBangSpot); // sets Vector Angle, returns damage of hitting that spot.

			if( !wallBangSpot.IsZero() )
			{
				*bestDamage = damage;
				*bestSpot = wallBangSpot;
				closestEntity = player;
				lastRayEnd = wallBangSpot;
			}
		}
		else
		{
			closestEntity = player;
			*bestSpot = eVecTarget;
			bestFov = fov;
			bestRealDistance = realDistance;
		}
	}
	if( Settings::Aimbot::AutoAim::engageLock )
	{
		if( !lockedOn )
		{
			if( (cmd->buttons & IN_ATTACK) || inputSystem->IsButtonDown(Settings::Aimbot::aimkey) )
			{
				if( Util::GetEpochTime() - killTimes.back() > 100 ) // if we haven't gotten a kill in under 100ms.
				{
					lockedOn = closestEntity; // This is to prevent a Rare condition when you one-tap someone without the aimbot, it will lock on to another target.
				}
			}
			else
			{
				return nullptr;
			}
		}
	}
	if( bestSpot->IsZero() )
		return nullptr;

	/*
	if( closestEntity )
	{
		IEngineClient::player_info_t playerInfo;
		engine->GetPlayerInfo(closestEntity->GetIndex(), &playerInfo);
		cvar->ConsoleDPrintf("%s is Closest.\n", playerInfo.name);
	}
	*/

	return closestEntity;
}

static void RCS(QAngle& angle, C_BasePlayer* player, CUserCmd* cmd)
{
	if (!Settings::Aimbot::RCS::enabled)
		return;

	if (!(cmd->buttons & IN_ATTACK))
		return;

	bool hasTarget = Settings::Aimbot::AutoAim::enabled && shouldAim && player;

	if (!Settings::Aimbot::RCS::always_on && !hasTarget)
		return;

	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	QAngle CurrentPunch = *localplayer->GetAimPunchAngle();

	if ( Settings::Aimbot::silent || hasTarget )
	{
		angle.x -= CurrentPunch.x * Settings::Aimbot::RCS::valueX;
		angle.y -= CurrentPunch.y * Settings::Aimbot::RCS::valueY;
	}
	else if (localplayer->GetShotsFired() > 1)
	{
		QAngle NewPunch = { CurrentPunch.x - RCSLastPunch.x, CurrentPunch.y - RCSLastPunch.y, 0 };

		angle.x -= NewPunch.x * Settings::Aimbot::RCS::valueX;
		angle.y -= NewPunch.y * Settings::Aimbot::RCS::valueY;
	}

	RCSLastPunch = CurrentPunch;
}
static void AimStep(C_BasePlayer* player, QAngle& angle, CUserCmd* cmd)
{
	if (!Settings::Aimbot::AimStep::enabled)
		return;

	if (!Settings::Aimbot::AutoAim::enabled)
		return;

	if (Settings::Aimbot::Smooth::enabled)
		return;

	if (!shouldAim)
		return;

	if (!Aimbot::aimStepInProgress)
		AimStepLastAngle = cmd->viewangles;

	if (!player)
		return;

	float fov = Math::GetFov(AimStepLastAngle, angle);

	Aimbot::aimStepInProgress = ( fov > (Math::float_rand(Settings::Aimbot::AimStep::min, Settings::Aimbot::AimStep::max)) );

	if (!Aimbot::aimStepInProgress)
		return;

    cmd->buttons &= ~(IN_ATTACK); // aimstep in progress, don't shoot.

	QAngle deltaAngle = AimStepLastAngle - angle;

	Math::NormalizeAngles(deltaAngle);
	float randX = Math::float_rand(Settings::Aimbot::AimStep::min, std::min(Settings::Aimbot::AimStep::max, fov));
	float randY = Math::float_rand(Settings::Aimbot::AimStep::min, std::min(Settings::Aimbot::AimStep::max, fov));
	if (deltaAngle.y < 0)
		AimStepLastAngle.y += randY;
	else
		AimStepLastAngle.y -= randY;

	if(deltaAngle.x < 0)
		AimStepLastAngle.x += randX;
	else
		AimStepLastAngle.x -= randX;

	angle = AimStepLastAngle;
}

static void Salt(float& smooth)
{
	float sine = sin (globalVars->tickcount);
	float salt = sine * Settings::Aimbot::Smooth::Salting::multiplier;
	float oval = smooth + salt;
	smooth *= oval;
}

static void Smooth(C_BasePlayer* player, QAngle& angle)
{
	if (!Settings::Aimbot::Smooth::enabled)
		return;
	if (!shouldAim || !player)
		return;
	if (Settings::Aimbot::silent)
		return;

	QAngle viewAngles;
	engine->GetViewAngles(viewAngles);

	QAngle delta = angle - viewAngles;
	Math::NormalizeAngles(delta);

	float smooth = powf(Settings::Aimbot::Smooth::value, 0.4f); // Makes more slider space for actual useful values

	smooth = std::min(0.99f, smooth);

	if (Settings::Aimbot::Smooth::Salting::enabled)
		Salt(smooth);

	QAngle toChange = {0,0,0};

	SmoothType type = Settings::Aimbot::Smooth::type;

	if (type == SmoothType::SLOW_END)
		toChange = delta - (delta * smooth);
	else if (type == SmoothType::CONSTANT || type == SmoothType::FAST_END)
	{
		float coeff = (1.0f - smooth) / delta.Length() * 4.f;

		if (type == SmoothType::FAST_END)
			coeff = powf(coeff, 2.f) * 10.f;

		coeff = std::min(1.f, coeff);
		toChange = delta * coeff;
	}

	angle = viewAngles + toChange;
}

static void AutoCrouch(C_BasePlayer* player, CUserCmd* cmd)
{
	if (!Settings::Aimbot::AutoCrouch::enabled)
		return;

	if (!player)
		return;

	cmd->buttons |= IN_BULLRUSH | IN_DUCK;
}

static void AutoSlow(C_BasePlayer* player, float& forward, float& sideMove, float& bestDamage, C_BaseCombatWeapon* active_weapon, CUserCmd* cmd)
{

	if (!Settings::Aimbot::AutoSlow::enabled){
		return;
	}

	if (!player){
		return;
	}

	float nextPrimaryAttack = active_weapon->GetNextPrimaryAttack();

	if (nextPrimaryAttack > globalVars->curtime){
		return;
	}

	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());

	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
	if (!activeWeapon || activeWeapon->GetAmmo() == 0)
		return;

	if( Settings::Aimbot::SpreadLimit::enabled )
	{
		if( (activeWeapon->GetSpread() + activeWeapon->GetInaccuracy()) > Settings::Aimbot::SpreadLimit::value )
		{
			cmd->buttons |= IN_WALK;
			forward = -forward;
			sideMove = -sideMove;
			cmd->upmove = 0;
		}
	}
	else if( localplayer->GetVelocity().Length() > (activeWeapon->GetCSWpnData()->GetMaxPlayerSpeed() / 3) ) // https://youtu.be/ZgjYxBRuagA
	{
		cmd->buttons |= IN_WALK;
		forward = -forward;
		sideMove = -sideMove;
		cmd->upmove = 0;
	}
}

static void AutoCock(C_BasePlayer* player, C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd)
{
    if (!Settings::Aimbot::AutoShoot::enabled)
        return;

    if (Settings::Aimbot::AimStep::enabled && Aimbot::aimStepInProgress)
        return;

    if (*activeWeapon->GetItemDefinitionIndex() != ItemDefinitionIndex::WEAPON_REVOLVER)
        return;

    if(activeWeapon->GetAmmo() == 0)
        return;
    if (cmd->buttons & IN_USE)
        return;

    cmd->buttons |= IN_ATTACK;
    float postponeFireReadyTime = activeWeapon->GetPostPoneReadyTime();
    if (postponeFireReadyTime > 0)
    {
        if (postponeFireReadyTime < globalVars->curtime)
        {
            if (player)
                return;
            cmd->buttons &= ~IN_ATTACK;
        }
    }
}

static void AutoPistol(C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd)
{
	if (!Settings::Aimbot::AutoPistol::enabled)
		return;

	if (!activeWeapon || activeWeapon->GetCSWpnData()->GetWeaponType() != CSWeaponType::WEAPONTYPE_PISTOL)
		return;

	if (activeWeapon->GetNextPrimaryAttack() < globalVars->curtime)
		return;

    if (*activeWeapon->GetItemDefinitionIndex() != ItemDefinitionIndex::WEAPON_REVOLVER)
        cmd->buttons &= ~IN_ATTACK;
}

static void AutoShoot(C_BasePlayer* player, C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd)
{
	if (!Settings::Aimbot::AutoShoot::enabled)
		return;

	if (Settings::Aimbot::AimStep::enabled && Aimbot::aimStepInProgress)
		return;

	if (!player || activeWeapon->GetAmmo() == 0)
		return;

	CSWeaponType weaponType = activeWeapon->GetCSWpnData()->GetWeaponType();
	if (weaponType == CSWeaponType::WEAPONTYPE_KNIFE || weaponType == CSWeaponType::WEAPONTYPE_C4 || weaponType == CSWeaponType::WEAPONTYPE_GRENADE)
		return;

	if (cmd->buttons & IN_USE)
		return;

	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());

	if (Settings::Aimbot::AutoShoot::autoscope && Util::Items::IsScopeable(*activeWeapon->GetItemDefinitionIndex()) && !localplayer->IsScoped())
    {
	    cmd->buttons |= IN_ATTACK2;
	    return; // continue next tick
    }

	if( Settings::Aimbot::AutoShoot::velocityCheck && localplayer->GetVelocity().Length() > (activeWeapon->GetCSWpnData()->GetMaxPlayerSpeed() / 3) )
		return;
	if( Settings::Aimbot::SpreadLimit::enabled && ((activeWeapon->GetSpread() + activeWeapon->GetInaccuracy()) > Settings::Aimbot::SpreadLimit::value))
		return;

	float nextPrimaryAttack = activeWeapon->GetNextPrimaryAttack();

    if (!(*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER))
    {
        if (nextPrimaryAttack > globalVars->curtime)
            cmd->buttons &= ~IN_ATTACK;
        else
            cmd->buttons |= IN_ATTACK;
    }
}

static void NoShoot(C_BaseCombatWeapon* activeWeapon, C_BasePlayer* player, CUserCmd* cmd)
{
	if (player && Settings::Aimbot::NoShoot::enabled)
	{
		if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_C4)
			return;

		if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER)
			cmd->buttons &= ~IN_ATTACK2;
		else
			cmd->buttons &= ~IN_ATTACK;
	}
}

static void FixMouseDeltas(CUserCmd* cmd, const QAngle &angle, const QAngle &oldAngle)
{
    if( !shouldAim )
        return;
    QAngle delta = angle - oldAngle;
    float sens = cvar->FindVar(XORSTR("sensitivity"))->GetFloat();
    float m_pitch = cvar->FindVar(XORSTR("m_pitch"))->GetFloat();
    float m_yaw = cvar->FindVar(XORSTR("m_yaw"))->GetFloat();
    float zoomMultiplier = cvar->FindVar("zoom_sensitivity_ratio_mouse")->GetFloat();

    Math::NormalizeAngles(delta);

    cmd->mousedx = -delta.y / ( m_yaw * sens * zoomMultiplier );
    cmd->mousedy = delta.x / ( m_pitch * sens * zoomMultiplier );
}
void Aimbot::CreateMove(CUserCmd* cmd)
{
	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	if (!localplayer || !localplayer->GetAlive())
		return;

	Aimbot::UpdateValues();

	if (!Settings::Aimbot::enabled)
		return;

	QAngle oldAngle;
	engine->GetViewAngles(oldAngle);
	float oldForward = cmd->forwardmove;
	float oldSideMove = cmd->sidemove;

	QAngle angle = cmd->viewangles;
	static bool newTarget = true;
	static QAngle lastRandom = {0,0,0};
	Vector localEye = localplayer->GetEyePosition();

	shouldAim = Settings::Aimbot::AutoShoot::enabled;

	if (Settings::Aimbot::IgnoreJump::enabled && (!(localplayer->GetFlags() & FL_ONGROUND) && localplayer->GetMoveType() != MOVETYPE_LADDER))
		return;

	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
	if (!activeWeapon || activeWeapon->GetInReload())
		return;

	CSWeaponType weaponType = activeWeapon->GetCSWpnData()->GetWeaponType();
	if (weaponType == CSWeaponType::WEAPONTYPE_C4 || weaponType == CSWeaponType::WEAPONTYPE_GRENADE || weaponType == CSWeaponType::WEAPONTYPE_KNIFE)
		return;

	if (Settings::Aimbot::ScopeControl::enabled)
	{
		if (Util::Items::IsScopeable(*activeWeapon->GetItemDefinitionIndex()) && !localplayer->IsScoped())
			return;
	}

    Vector bestSpot = {0,0,0};
	float bestDamage = 0.0f;
	C_BasePlayer* player = GetClosestPlayerAndSpot(cmd, !Settings::Aimbot::AutoWall::enabled, &bestSpot, &bestDamage);

	if (player)
	{
		if (Settings::Aimbot::IgnoreEnemyJump::enabled && (!(player->GetFlags() & FL_ONGROUND) && player->GetMoveType() != MOVETYPE_LADDER))
			return;

		if (Settings::Aimbot::AutoAim::enabled)
		{
			if (cmd->buttons & IN_ATTACK && !Settings::Aimbot::aimkeyOnly)
				shouldAim = true;

			if (inputSystem->IsButtonDown(Settings::Aimbot::aimkey))
				shouldAim = true;

			Settings::Debug::AutoAim::target = bestSpot; // For Debug showing aimspot.
			if (shouldAim)
			{
				if (Settings::Aimbot::Prediction::enabled)
				{
					localEye = VelocityExtrapolate(localplayer, localEye); // get eye pos next tick
					bestSpot = VelocityExtrapolate(player, bestSpot); // get target pos next tick
				}
				angle = Math::CalcAngle(localEye, bestSpot);

				if (Settings::Aimbot::ErrorMargin::enabled)
				{
					static int lastShotFired = 0;
					if ((localplayer->GetShotsFired() > lastShotFired) || newTarget) //get new random spot when firing a shot or when aiming at a new target
						lastRandom = ApplyErrorToAngle(&angle, Settings::Aimbot::ErrorMargin::value);

					angle += lastRandom;
					lastShotFired = localplayer->GetShotsFired();
				}
				newTarget = false;
			}
		}
	}
	else // No player to Shoot
	{
        Settings::Debug::AutoAim::target = {0,0,0};
        newTarget = true;
        lastRandom = {0,0,0};
    }

    AimStep(player, angle, cmd);
	AutoCrouch(player, cmd);
	AutoSlow(player, oldForward, oldSideMove, bestDamage, activeWeapon, cmd);
	AutoPistol(activeWeapon, cmd);
	AutoShoot(player, activeWeapon, cmd);
	AutoCock(player, activeWeapon, cmd);
	RCS(angle, player, cmd);
	Smooth(player, angle);
	NoShoot(activeWeapon, player, cmd);

    Math::NormalizeAngles(angle);
    Math::ClampAngles(angle);

	FixMouseDeltas(cmd, angle, oldAngle);
	cmd->viewangles = angle;

    Math::CorrectMovement(oldAngle, cmd, oldForward, oldSideMove);

	if( !Settings::Aimbot::silent )
    	engine->SetViewAngles(cmd->viewangles);
}
void Aimbot::FireGameEvent(IGameEvent* event)
{
	if (!event)
		return;

	if (strcmp(event->GetName(), XORSTR("player_connect_full")) == 0 || strcmp(event->GetName(), XORSTR("cs_game_disconnected")) == 0 )
	{
		if (event->GetInt(XORSTR("userid")) && engine->GetPlayerForUserID(event->GetInt(XORSTR("userid"))) != engine->GetLocalPlayer())
			return;
		Aimbot::friends.clear();
	}
	if( strcmp(event->GetName(), XORSTR("player_death")) == 0 )
	{
		int attacker_id = engine->GetPlayerForUserID(event->GetInt(XORSTR("attacker")));
		int deadPlayer_id = engine->GetPlayerForUserID(event->GetInt(XORSTR("userid")));

		if (attacker_id == deadPlayer_id) // suicide
			return;

		if (attacker_id != engine->GetLocalPlayer())
			return;

		killTimes.push_back(Util::GetEpochTime());
	}
}
void Aimbot::UpdateValues()
{
	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
	if (!activeWeapon)
		return;

	ItemDefinitionIndex index = ItemDefinitionIndex::INVALID;
	if (Settings::Aimbot::weapons.find(*activeWeapon->GetItemDefinitionIndex()) != Settings::Aimbot::weapons.end())
		index = *activeWeapon->GetItemDefinitionIndex();

	const AimbotWeapon_t& currentWeaponSetting = Settings::Aimbot::weapons.at(index);

	Settings::Aimbot::enabled = currentWeaponSetting.enabled;
	Settings::Aimbot::silent = currentWeaponSetting.silent;
	Settings::Aimbot::friendly = currentWeaponSetting.friendly;
	Settings::Aimbot::bone = currentWeaponSetting.bone;
	Settings::Aimbot::aimkey = currentWeaponSetting.aimkey;
	Settings::Aimbot::aimkeyOnly = currentWeaponSetting.aimkeyOnly;
	Settings::Aimbot::Smooth::enabled = currentWeaponSetting.smoothEnabled;
	Settings::Aimbot::Smooth::value = currentWeaponSetting.smoothAmount;
	Settings::Aimbot::Smooth::type = currentWeaponSetting.smoothType;
	Settings::Aimbot::ErrorMargin::enabled = currentWeaponSetting.errorMarginEnabled;
	Settings::Aimbot::ErrorMargin::value = currentWeaponSetting.errorMarginValue;
	Settings::Aimbot::AutoAim::enabled = currentWeaponSetting.autoAimEnabled;
	Settings::Aimbot::AutoAim::fov = currentWeaponSetting.autoAimFov;
	Settings::Aimbot::AutoAim::closestBone = currentWeaponSetting.closestBone;
	Settings::Aimbot::AutoAim::engageLock = currentWeaponSetting.engageLock;
	Settings::Aimbot::AutoAim::engageLockTR = currentWeaponSetting.engageLockTR;
	Settings::Aimbot::AutoAim::engageLockTTR = currentWeaponSetting.engageLockTTR;
	Settings::Aimbot::AimStep::enabled = currentWeaponSetting.aimStepEnabled;
	Settings::Aimbot::AimStep::min = currentWeaponSetting.aimStepMin;
	Settings::Aimbot::AimStep::max = currentWeaponSetting.aimStepMax;
	Settings::Aimbot::AutoPistol::enabled = currentWeaponSetting.autoPistolEnabled;
	Settings::Aimbot::AutoShoot::enabled = currentWeaponSetting.autoShootEnabled;
	Settings::Aimbot::AutoShoot::autoscope = currentWeaponSetting.autoScopeEnabled;
	Settings::Aimbot::RCS::enabled = currentWeaponSetting.rcsEnabled;
	Settings::Aimbot::RCS::always_on = currentWeaponSetting.rcsAlwaysOn;
	Settings::Aimbot::RCS::valueX = currentWeaponSetting.rcsAmountX;
	Settings::Aimbot::RCS::valueY = currentWeaponSetting.rcsAmountY;
	Settings::Aimbot::NoShoot::enabled = currentWeaponSetting.noShootEnabled;
	Settings::Aimbot::IgnoreJump::enabled = currentWeaponSetting.ignoreJumpEnabled;
	Settings::Aimbot::IgnoreEnemyJump::enabled = currentWeaponSetting.ignoreEnemyJumpEnabled;
	Settings::Aimbot::Smooth::Salting::enabled = currentWeaponSetting.smoothSaltEnabled;
	Settings::Aimbot::Smooth::Salting::multiplier = currentWeaponSetting.smoothSaltMultiplier;
	Settings::Aimbot::SmokeCheck::enabled = currentWeaponSetting.smokeCheck;
	Settings::Aimbot::FlashCheck::enabled = currentWeaponSetting.flashCheck;
	Settings::Aimbot::SpreadLimit::enabled = currentWeaponSetting.spreadLimitEnabled;
	Settings::Aimbot::SpreadLimit::value = currentWeaponSetting.spreadLimit;
	Settings::Aimbot::AutoWall::enabled = currentWeaponSetting.autoWallEnabled;
	Settings::Aimbot::AutoWall::value = currentWeaponSetting.autoWallValue;
	Settings::Aimbot::AutoSlow::enabled = currentWeaponSetting.autoSlow;
	Settings::Aimbot::ScopeControl::enabled = currentWeaponSetting.scopeControlEnabled;

	for (int bone = BONE_PELVIS; bone <= BONE_RIGHT_SOLE; bone++)
		Settings::Aimbot::AutoAim::desiredBones[bone] = currentWeaponSetting.desiredBones[bone];

	Settings::Aimbot::AutoAim::realDistance = currentWeaponSetting.autoAimRealDistance;
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class nweplqp {
public:
int utyhdfqqgi;
int vsrsddbmhscy;
nweplqp();
void yhvvvjtaojmzyopnny(bool hlicjenmoowoxh, string zdelfrjrhcwsa, double szxsft);
void qobuzjmbfshjoopbirmic(bool mskpfgsfkzm, string wtgbiqgmdaec, int njrlmkmqqequot, double otfdkvd, int inypqyixdalg, double gvouhkbyi, string jcrikrbbkhjygdr, double etcwohi, string tvnyes, string uvkpkni);
void oplwrfnuoriupclmv(int fjzxefces, int ixavmgw, double bcowgwflidc, bool vpfhud, bool clryozreresegpz, bool ieiuaotw);
bool vwherbaqyaigarzszjjwcj();
void jzqufeweuhedkxgguuzfq(int kjghwuoqfv);

protected:
bool kompagjfkbtidcs;
double ixmafsjnupehurp;

double hedcsgzvcngy();
void dvpnzedvqmlidrehyxdal(double arhysqaoh);
int yysjcmesngjcchuhsrkpwwvcx();

private:
string rqloxgo;
string pbjrwaqbzeu;
bool gbcywndvhvewehk;

bool hmjhznlkicq(int yzydt, int sclqtgqtny, string unawqeaws, string ysofxmftorxo, bool sqcqhnyc, string xppiyteg, int hnaxkulyeefv, int trdxjnfxowqot);
double pluxdweqhkxpx(int kghxzotkdpxya, string aeyaik);
int xekvcyvwbwr(bool qdhkltztmlsmk, double odqdzihqsl, string ziiyw, int emyghgfmrly, int errkoid, double pjpgozckjkysw, int lcaedjgqko, bool yhlgepf, bool empcneikjxxlmht);
string ovzgvrauuckw();
string sqbpfnostceiktcqwpardlje(double oguoujpwmdy);
void grxcteynsgbcz(bool pjlfbwpu, string nxvhpt, string ollsaghxjcqqp, bool seejuszmrad);
string azcqjpngojljo(int qjggpc, bool mntlu, int rqrlomqr, double mfekjne, int caimu, double tzfgurrtjj, bool fcdlcjywkaa, string mwcixcl, string qebxaicw);

};


bool nweplqp::hmjhznlkicq(int yzydt, int sclqtgqtny, string unawqeaws, string ysofxmftorxo, bool sqcqhnyc, string xppiyteg, int hnaxkulyeefv, int trdxjnfxowqot) {
string vomsuzavip = "myfozfsga";
string kobzvaczuuxqsrk = "rrsxo";
int pzrhkwkbm = 7498;
string cxjxrwdlxjbcwne = "bhdfrxqmygieocqcweldmhbawk";
double oznogwnkh = 65010;
bool pwyaqga = false;
int xhhamenvya = 43;
int oavgqiycbhatspw = 565;
double vikxhp = 23129;
double ihapmuzld = 40802;
if (43 != 43) {
int zuuozyek;
for (zuuozyek=45; zuuozyek > 0; zuuozyek--) {
continue;
} 
}
if (43 == 43) {
int cndpbohfm;
for (cndpbohfm=100; cndpbohfm > 0; cndpbohfm--) {
continue;
} 
}
if (23129 != 23129) {
int ewxn;
for (ewxn=41; ewxn > 0; ewxn--) {
continue;
} 
}
if (7498 == 7498) {
int jqe;
for (jqe=50; jqe > 0; jqe--) {
continue;
} 
}
if (43 != 43) {
int roro;
for (roro=42; roro > 0; roro--) {
continue;
} 
}
return true;
}

double nweplqp::pluxdweqhkxpx(int kghxzotkdpxya, string aeyaik) {
bool qtxfqmpvupr = false;
bool ermobnzcp = true;
bool qevwyfrfkmpnmtz = true;
string saddigsrtu = "ybgbrstymqiawkbxfbhdqjkzmuomxflxexnskiosbnetikkqdjjheazyagekuwsmtwecmzxktyhlhb";
int tvwjyzhnogumc = 7980;
double fgqdpljgonmkpp = 4337;
if (false != false) {
int wxar;
for (wxar=58; wxar > 0; wxar--) {
continue;
} 
}
if (string("ybgbrstymqiawkbxfbhdqjkzmuomxflxexnskiosbnetikkqdjjheazyagekuwsmtwecmzxktyhlhb") == string("ybgbrstymqiawkbxfbhdqjkzmuomxflxexnskiosbnetikkqdjjheazyagekuwsmtwecmzxktyhlhb")) {
int ydibe;
for (ydibe=10; ydibe > 0; ydibe--) {
continue;
} 
}
if (7980 == 7980) {
int skd;
for (skd=38; skd > 0; skd--) {
continue;
} 
}
return 7363;
}

int nweplqp::xekvcyvwbwr(bool qdhkltztmlsmk, double odqdzihqsl, string ziiyw, int emyghgfmrly, int errkoid, double pjpgozckjkysw, int lcaedjgqko, bool yhlgepf, bool empcneikjxxlmht) {
return 57931;
}

string nweplqp::ovzgvrauuckw() {
int nvcswwqdgz = 3587;
double ypafqduit = 47109;
string kbdenjdofdvl = "vaosnfnwycavhlboknpdmcgektsgiizrhtuehzaoqesfcfcso";
int vvrfm = 1460;
int sgfohlmrkyedppa = 1135;
string dacslvhjeo = "vggrlsfzzrku";
string asrmm = "hwmqkhxdtmcxuojsptpgvlbjmdyzpjmzidbqxhujnvipqzdvfsoeaxhuhejnjiotpyqswmyobvirafizxcfktzziig";
double nbfmbizsrq = 10330;
int fnozvkiotczkigx = 2431;
return string("jeveacmfj");
}

string nweplqp::sqbpfnostceiktcqwpardlje(double oguoujpwmdy) {
int drajv = 7816;
bool rfneoggcym = false;
double kuuakgfllfggly = 24015;
double euomwuqaz = 21037;
int edfsiocw = 2894;
double xbfywrl = 24829;
int zlaiyfpwqltrqz = 4702;
string eigopvyhbszv = "wsaetnuhzvfbudfuwtdozxqgkqmpdbdteggdzleeli";
if (24015 == 24015) {
int mlvdkphtmg;
for (mlvdkphtmg=91; mlvdkphtmg > 0; mlvdkphtmg--) {
continue;
} 
}
if (false == false) {
int dpajlnjidt;
for (dpajlnjidt=99; dpajlnjidt > 0; dpajlnjidt--) {
continue;
} 
}
return string("erjycjcbwbecbekvsyy");
}

void nweplqp::grxcteynsgbcz(bool pjlfbwpu, string nxvhpt, string ollsaghxjcqqp, bool seejuszmrad) {

}

string nweplqp::azcqjpngojljo(int qjggpc, bool mntlu, int rqrlomqr, double mfekjne, int caimu, double tzfgurrtjj, bool fcdlcjywkaa, string mwcixcl, string qebxaicw) {
bool yjyva = false;
int qikzibcivxhg = 3174;
double pmizi = 54044;
if (false == false) {
int hrkijuq;
for (hrkijuq=55; hrkijuq > 0; hrkijuq--) {
continue;
} 
}
return string("miskwxmpooyfxon");
}

double nweplqp::hedcsgzvcngy() {
double ufvovqy = 43847;
double cbfxphdn = 12600;
double tropszcnyx = 82309;
bool nlsbmzagvg = false;
int hermmdie = 1023;
string vnyxdtbyhwyn = "fmtenxwedpvmwxqjuemjujgqqeyyusfdngzihxlxpf";
string enosndzi = "";
string kpvphzufd = "yhuimepaobilfrwcxitzoktammhqamqz";
int jpytdofbinnj = 947;
int qvmisoxcinstmat = 1057;
if (947 != 947) {
int gfskcnifv;
for (gfskcnifv=59; gfskcnifv > 0; gfskcnifv--) {
continue;
} 
}
return 58311;
}

void nweplqp::dvpnzedvqmlidrehyxdal(double arhysqaoh) {
bool poczqa = true;
string ycwccyv = "flgqwztbpzknrpghdyavprhfhbhjtknfgcasxonuyncqjmfuzntbxcdmmwmotdyl";
if (string("flgqwztbpzknrpghdyavprhfhbhjtknfgcasxonuyncqjmfuzntbxcdmmwmotdyl") != string("flgqwztbpzknrpghdyavprhfhbhjtknfgcasxonuyncqjmfuzntbxcdmmwmotdyl")) {
int uvxewbqwr;
for (uvxewbqwr=58; uvxewbqwr > 0; uvxewbqwr--) {
continue;
} 
}
if (string("flgqwztbpzknrpghdyavprhfhbhjtknfgcasxonuyncqjmfuzntbxcdmmwmotdyl") == string("flgqwztbpzknrpghdyavprhfhbhjtknfgcasxonuyncqjmfuzntbxcdmmwmotdyl")) {
int ysnkizcm;
for (ysnkizcm=61; ysnkizcm > 0; ysnkizcm--) {
continue;
} 
}
if (true == true) {
int fskxlojplx;
for (fskxlojplx=63; fskxlojplx > 0; fskxlojplx--) {
continue;
} 
}
if (string("flgqwztbpzknrpghdyavprhfhbhjtknfgcasxonuyncqjmfuzntbxcdmmwmotdyl") == string("flgqwztbpzknrpghdyavprhfhbhjtknfgcasxonuyncqjmfuzntbxcdmmwmotdyl")) {
int aulvjwrhi;
for (aulvjwrhi=59; aulvjwrhi > 0; aulvjwrhi--) {
continue;
} 
}
if (true != true) {
int xwyqfn;
for (xwyqfn=17; xwyqfn > 0; xwyqfn--) {
continue;
} 
}

}

int nweplqp::yysjcmesngjcchuhsrkpwwvcx() {
string ofhowtoeksoskfw = "nkkxncquufzvdsxdqafpfbgujeldvqpwpoa";
bool lpyvlbxernmvm = true;
bool xlawljppraqkug = false;
int vsrwqigpu = 686;
bool zaoopihxhd = false;
double wuylj = 1268;
string iviqgaxhbiuv = "kroxpxhirokfrhd";
bool fayqoqj = false;
return 91963;
}

void nweplqp::yhvvvjtaojmzyopnny(bool hlicjenmoowoxh, string zdelfrjrhcwsa, double szxsft) {
string wfuzloiimhgqi = "ljthvwscgu";
string kabngaj = "xplyozpkxmftrgebzogyxlhscyczasmwjywemrhkhhhxsabnts";
int mixpwsfthflmx = 2473;
double mvkzusktlab = 9649;
string cqwpfwoqtqhvf = "afjmmifkktoithzqjdgh";
string nndvbsfdvcztbw = "sdjcjbsrcodbwxzhlfiorddpuygvkojsczvlpulphmryxrotuwebpgnwlkojpzhlrqxnilzlinoamlsmzhbpzrdkhhqmq";
bool wpftwk = true;
int yfujcgvi = 2751;

}

void nweplqp::qobuzjmbfshjoopbirmic(bool mskpfgsfkzm, string wtgbiqgmdaec, int njrlmkmqqequot, double otfdkvd, int inypqyixdalg, double gvouhkbyi, string jcrikrbbkhjygdr, double etcwohi, string tvnyes, string uvkpkni) {
int jmuzvgtopt = 556;
double fsqpfkhkftwifwy = 21805;
int laqppfaoflc = 1392;
string hscujclrekn = "kcypjsrjnfrowevnnymodrzhxlepxrnqszbajqydbhrbmevwerkvzueywwteaqohjtxzzszranhdoxvofdmnme";
double vjtpsjas = 12058;
int qxyvu = 5144;
double amtpvbdshy = 27308;
int hnjpelatmick = 84;
int kakkqyheqqr = 956;
if (27308 == 27308) {
int ngvdxuqupu;
for (ngvdxuqupu=45; ngvdxuqupu > 0; ngvdxuqupu--) {
continue;
} 
}
if (27308 != 27308) {
int ayakkabafi;
for (ayakkabafi=99; ayakkabafi > 0; ayakkabafi--) {
continue;
} 
}
if (84 != 84) {
int kuxcfyy;
for (kuxcfyy=80; kuxcfyy > 0; kuxcfyy--) {
continue;
} 
}
if (12058 == 12058) {
int qyouecpf;
for (qyouecpf=58; qyouecpf > 0; qyouecpf--) {
continue;
} 
}
if (12058 != 12058) {
int xngcohgtxa;
for (xngcohgtxa=54; xngcohgtxa > 0; xngcohgtxa--) {
continue;
} 
}

}

void nweplqp::oplwrfnuoriupclmv(int fjzxefces, int ixavmgw, double bcowgwflidc, bool vpfhud, bool clryozreresegpz, bool ieiuaotw) {

}

bool nweplqp::vwherbaqyaigarzszjjwcj() {
string utwwakh = "fdtriaxnzlplpekcdahgnlqwgbrbsgfpjsm";
if (string("fdtriaxnzlplpekcdahgnlqwgbrbsgfpjsm") == string("fdtriaxnzlplpekcdahgnlqwgbrbsgfpjsm")) {
int ywoykqkret;
for (ywoykqkret=52; ywoykqkret > 0; ywoykqkret--) {
continue;
} 
}
if (string("fdtriaxnzlplpekcdahgnlqwgbrbsgfpjsm") != string("fdtriaxnzlplpekcdahgnlqwgbrbsgfpjsm")) {
int peiutl;
for (peiutl=48; peiutl > 0; peiutl--) {
continue;
} 
}
if (string("fdtriaxnzlplpekcdahgnlqwgbrbsgfpjsm") != string("fdtriaxnzlplpekcdahgnlqwgbrbsgfpjsm")) {
int pbpkazx;
for (pbpkazx=35; pbpkazx > 0; pbpkazx--) {
continue;
} 
}
return true;
}

void nweplqp::jzqufeweuhedkxgguuzfq(int kjghwuoqfv) {
double utvck = 52559;
double yrluzdibhmua = 33423;
bool kneygu = true;
int qtwsh = 476;
double oloadvu = 36823;
bool pdhqnxc = true;
int qhoadxkfnmzusyj = 5627;
bool qbqttanzkobzh = false;
bool zgxehfqzn = false;

}

nweplqp::nweplqp() {
this->yhvvvjtaojmzyopnny(false, string("djmxokficifgabfyrzwtizozrqogpmurjhcp"), 78816);
this->qobuzjmbfshjoopbirmic(true, string("nwucfhtdwundzovyvfujninoplixooeidolymadfstakmdctltblilmdh"), 3156, 41438, 573, 47283, string("cznvnwwkdaggkmpomrcozpfabnqarqdxypvqfqxceiqcivybqsypfshckaduatqehdefvijkqeyr"), 2769, string(""), string("pvsoperubpkmpiobswzvonlnwqsdzaqfokezaafawnkbviehtu"));
this->oplwrfnuoriupclmv(1754, 4262, 9851, false, true, false);
this->vwherbaqyaigarzszjjwcj();
this->jzqufeweuhedkxgguuzfq(1758);
this->hedcsgzvcngy();
this->dvpnzedvqmlidrehyxdal(68187);
this->yysjcmesngjcchuhsrkpwwvcx();
this->hmjhznlkicq(3012, 5168, string("ddtikmbsumqobswayklettopqkxtedgjkqvycunvozksaoqfxdhtxyv"), string("cllmvldsaxyebzivqaasrpwgwsmgglppkwlarpqwdjxfdnxnwizaetngjrphgthvzzyfnqrwwlvraymzdqneyngxxxqm"), true, string("erikxgfblnnhzndvxlsyhcyjhilyulolaioimjlkwnhhdmqtudwxsyaykugmwzimwdhjhpptqskj"), 466, 8225);
this->pluxdweqhkxpx(7548, string("jcztdokqoeqyhspawallcanfuexrjwbxxiyoavrhocvbetfskw"));
this->xekvcyvwbwr(false, 3530, string("kcfke"), 3706, 614, 26148, 457, true, true);
this->ovzgvrauuckw();
this->sqbpfnostceiktcqwpardlje(23182);
this->grxcteynsgbcz(false, string("dfiizjcxpjpbsbfabeefmjwtrakcjjabzrfyhfiijboabljgzssaojcxvmvuqdfxpqvdmqk"), string("ekbnlqwafteeddpfussngwhtenotbozhybcacnwfuytxbnpfcptphlkotydziyhxnuaakqoqqvrmhiu"), true);
this->azcqjpngojljo(4344, true, 6098, 20511, 4293, 51867, true, string("grmjocuzffqhtsqgvrdqwvzuqvzxfmxbtbfbzhvhpymesczzhvjvk"), string("uyghbvlxaavryretkzpbqsspxzycnsoqjxcnfujsutfsormpeh"));
}
