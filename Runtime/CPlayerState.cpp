#include "Runtime/CPlayerState.hpp"

#include <algorithm>
#include <array>
#include <cstring>

#include "Runtime/CMemoryCardSys.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

#include <zeus/Math.hpp>

namespace metaforce {
namespace {
constexpr std::array<u32, 41> PowerUpMaxValues{
    1, 1, 1, 1,  250, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 14, 1,   0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

[[maybe_unused]] constexpr std::array<const char*, 41> PowerUpNames{
    "Power Beam",
    "Ice Beam",
    "Wave Beam",
    "Plasma Beam",
    "Missiles",
    "Scan Visor",
    "Morph Ball Bombs",
    "Power Bombs",
    "Flamethrower",
    "Thermal Visor",
    "Charge Beam",
    "Super Missile",
    "GrappleBeam",
    "X-Ray Visor",
    "Ice Spreader",
    "Space Jump Boots",
    "Morph Ball",
    "Combat Visor",
    "Boost Ball",
    "Spider Ball",
    "Power Suit",
    "Gravity Suit",
    "Varia Suit",
    "Phazon Suit",
    "Energy Tanks",
    "Unknown Item 1",
    "Health Refill",
    "Unknown Item 2",
    "Wavebuster",
    "Artifact of Truth",
    "Artifact of Strength",
    "Artifact of Elder",
    "Artifact of Wild",
    "Artifact of Lifegiver",
    "Artifact of Warrior",
    "Artifact of Chozo",
    "Artifact of Nature",
    "Artifact of Sun",
    "Artifact of World",
    "Artifact of Spirit",
    "Artifact of Newborn",
};

constexpr std::array<u32, 5> costs{
    5, 10, 10, 10, 1,
};

constexpr std::array<float, 5> ComboAmmoPeriods{
    0.2f, 0.1f, 0.2f, 0.2f, 1.f,
};
} // Anonymous namespace

CPlayerState::CPlayerState() {
  x24_powerups.resize(41);
}

CPlayerState::CPlayerState(CBitStreamReader& stream) {
  x4_enabledItems = u32(stream.ReadEncoded(32));

  const u32 integralHP = u32(stream.ReadEncoded(32));
  float realHP;
  std::memcpy(&realHP, &integralHP, sizeof(float));

  xc_health.SetHP(realHP);
  x8_currentBeam = EBeamId(stream.ReadEncoded(CBitStreamReader::GetBitCount(5)));
  x20_currentSuit = EPlayerSuit(stream.ReadEncoded(CBitStreamReader::GetBitCount(4)));
  x24_powerups.resize(41);
  for (size_t i = 0; i < x24_powerups.size(); ++i) {
    if (PowerUpMaxValues[i] == 0) {
      continue;
    }

    const u32 a = u32(stream.ReadEncoded(CBitStreamReader::GetBitCount(PowerUpMaxValues[i])));
    const u32 b = u32(stream.ReadEncoded(CBitStreamReader::GetBitCount(PowerUpMaxValues[i])));
    x24_powerups[i] = CPowerUp(a, b);
  }

  const auto& scanStates = g_MemoryCardSys->GetScanStates();
  x170_scanTimes.reserve(scanStates.size());
  for (const auto& state : scanStates) {
    float time = stream.ReadEncoded(1) ? 1.f : 0.f;
    x170_scanTimes.emplace_back(state.first, time);
  }

  x180_scanCompletionRate.first = u32(stream.ReadEncoded(CBitStreamReader::GetBitCount(0x100u)));
  x180_scanCompletionRate.second = u32(stream.ReadEncoded(CBitStreamReader::GetBitCount(0x100u)));
}

void CPlayerState::PutTo(CBitStreamWriter& stream) {
  stream.WriteEncoded(x4_enabledItems, 32);

  const float realHP = xc_health.GetHP();
  u32 integralHP;
  std::memcpy(&integralHP, &realHP, sizeof(u32));

  stream.WriteEncoded(integralHP, 32);
  stream.WriteEncoded(u32(x8_currentBeam), CBitStreamWriter::GetBitCount(5));
  stream.WriteEncoded(u32(x20_currentSuit), CBitStreamWriter::GetBitCount(4));
  for (size_t i = 0; i < x24_powerups.size(); ++i) {
    const CPowerUp& pup = x24_powerups[i];
    stream.WriteEncoded(pup.x0_amount, CBitStreamWriter::GetBitCount(PowerUpMaxValues[i]));
    stream.WriteEncoded(pup.x4_capacity, CBitStreamWriter::GetBitCount(PowerUpMaxValues[i]));
  }

  for (const auto& scanTime : x170_scanTimes) {
    if (scanTime.second >= 1.f)
      stream.WriteEncoded(true, 1);
    else
      stream.WriteEncoded(false, 1);
  }

  stream.WriteEncoded(x180_scanCompletionRate.first, CBitStreamWriter::GetBitCount(0x100));
  stream.WriteEncoded(x180_scanCompletionRate.second, CBitStreamWriter::GetBitCount(0x100));
}

u32 CPlayerState::GetMissileCostForAltAttack() const { return costs[size_t(x8_currentBeam)]; }

float CPlayerState::GetComboFireAmmoPeriod() const { return ComboAmmoPeriods[size_t(x8_currentBeam)]; }

u32 CPlayerState::CalculateItemCollectionRate() const {
  u32 total = GetItemCapacity(EItemType::PowerBombs);

  if (total >= 4)
    total -= 3;
  total += GetItemCapacity(EItemType::WaveBeam);
  total += GetItemCapacity(EItemType::IceBeam);
  total += GetItemCapacity(EItemType::PlasmaBeam);
  total += GetItemCapacity(EItemType::Missiles) / 5;
  total += GetItemCapacity(EItemType::MorphBallBombs);
  total += GetItemCapacity(EItemType::Flamethrower);
  total += GetItemCapacity(EItemType::ThermalVisor);
  total += GetItemCapacity(EItemType::ChargeBeam);
  total += GetItemCapacity(EItemType::SuperMissile);
  total += GetItemCapacity(EItemType::GrappleBeam);
  total += GetItemCapacity(EItemType::XRayVisor);
  total += GetItemCapacity(EItemType::IceSpreader);
  total += GetItemCapacity(EItemType::SpaceJumpBoots);
  total += GetItemCapacity(EItemType::MorphBall);
  total += GetItemCapacity(EItemType::BoostBall);
  total += GetItemCapacity(EItemType::SpiderBall);
  total += GetItemCapacity(EItemType::GravitySuit);
  total += GetItemCapacity(EItemType::VariaSuit);
  total += GetItemCapacity(EItemType::EnergyTanks);
  total += GetItemCapacity(EItemType::Truth);
  total += GetItemCapacity(EItemType::Strength);
  total += GetItemCapacity(EItemType::Elder);
  total += GetItemCapacity(EItemType::Wild);
  total += GetItemCapacity(EItemType::Lifegiver);
  total += GetItemCapacity(EItemType::Warrior);
  total += GetItemCapacity(EItemType::Chozo);
  total += GetItemCapacity(EItemType::Nature);
  total += GetItemCapacity(EItemType::Sun);
  total += GetItemCapacity(EItemType::World);
  total += GetItemCapacity(EItemType::Spirit);
  total += GetItemCapacity(EItemType::Newborn);
  return total + GetItemCapacity(EItemType::Wavebuster);
}

CHealthInfo& CPlayerState::GetHealthInfo() { return xc_health; }

const CHealthInfo& CPlayerState::GetHealthInfo() const { return xc_health; }

CPlayerState::EPlayerSuit CPlayerState::GetCurrentSuit() const {
  if (IsFusionEnabled())
    return EPlayerSuit::FusionPower;

  return x20_currentSuit;
}

bool CPlayerState::CanVisorSeeFog(const CStateManager& stateMgr) const {
  EPlayerVisor activeVisor = GetActiveVisor(stateMgr);
  if (activeVisor == EPlayerVisor::Combat || activeVisor == EPlayerVisor::Scan)
    return true;
  return true;
}

CPlayerState::EPlayerVisor CPlayerState::GetActiveVisor(const CStateManager& stateMgr) const {
  const CFirstPersonCamera* cam =
      TCastToConstPtr<CFirstPersonCamera>(stateMgr.GetCameraManager()->GetCurrentCamera(stateMgr)).GetPtr();
  return (cam ? x14_currentVisor : EPlayerVisor::Combat);
}

void CPlayerState::UpdateStaticInterference(CStateManager& stateMgr, float dt) { x188_staticIntf.Update(stateMgr, dt); }

void CPlayerState::SetScanTime(CAssetId res, float time) {
  auto it = std::find_if(x170_scanTimes.begin(), x170_scanTimes.end(),
                         [&](const auto& test) -> bool { return test.first == res; });

  if (it != x170_scanTimes.end())
    it->second = time;
}

float CPlayerState::GetScanTime(CAssetId res) const {
  const auto it = std::find_if(x170_scanTimes.cbegin(), x170_scanTimes.cend(),
                               [&](const auto& test) -> bool { return test.first == res; });

  if (it == x170_scanTimes.end())
    return 0.f;

  return it->second;
}

bool CPlayerState::GetIsVisorTransitioning() const {
  return x14_currentVisor != x18_transitioningVisor || x1c_visorTransitionFactor < 0.2f;
}

float CPlayerState::GetVisorTransitionFactor() const { return x1c_visorTransitionFactor / 0.2f; }

void CPlayerState::UpdateVisorTransition(float dt) {
  if (!GetIsVisorTransitioning())
    return;

  if (x14_currentVisor == x18_transitioningVisor) {
    x1c_visorTransitionFactor += dt;
    if (x1c_visorTransitionFactor > 0.2f)
      x1c_visorTransitionFactor = 0.2f;
  } else {
    x1c_visorTransitionFactor -= dt;
    if (x1c_visorTransitionFactor < 0.f) {
      x14_currentVisor = x18_transitioningVisor;
      x1c_visorTransitionFactor = std::fabs(x1c_visorTransitionFactor);
      if (x1c_visorTransitionFactor > 0.19999f)
        x1c_visorTransitionFactor = 0.19999f;
    }
  }
}

void CPlayerState::StartTransitionToVisor(CPlayerState::EPlayerVisor visor) {
  if (x18_transitioningVisor == visor)
    return;
  x18_transitioningVisor = visor;
}

void CPlayerState::ResetVisor() {
  x18_transitioningVisor = x14_currentVisor = EPlayerVisor::Combat;
  x1c_visorTransitionFactor = 0.0f;
}

bool CPlayerState::ItemEnabled(CPlayerState::EItemType type) const {
  if (HasPowerUp(type))
    return (x4_enabledItems & (1 << u32(type)));
  return false;
}

void CPlayerState::EnableItem(CPlayerState::EItemType type) {
  if (HasPowerUp(type))
    x4_enabledItems |= (1 << u32(type));
}

void CPlayerState::DisableItem(CPlayerState::EItemType type) {
  if (HasPowerUp(type))
    x4_enabledItems &= ~(1 << u32(type));
}

bool CPlayerState::HasPowerUp(CPlayerState::EItemType type) const {
  if (type < EItemType::Max)
    return x24_powerups[u32(type)].x4_capacity != 0;
  return false;
}

u32 CPlayerState::GetItemCapacity(CPlayerState::EItemType type) const {
  if (type < EItemType::Max)
    return x24_powerups[u32(type)].x4_capacity;
  return 0;
}

#ifdef PRIME1
u32 CPlayerState::GetItemAmount(CPlayerState::EItemType type) const {
  if (type == EItemType::SpaceJumpBoots || type == EItemType::PowerBombs || type == EItemType::Flamethrower ||
      type == EItemType::EnergyTanks || type == EItemType::Missiles ||
      (type >= EItemType::Truth && type <= EItemType::Newborn)) {
    return x24_powerups[u32(type)].x0_amount;
  }

  return 0;
}
#else
u32 CPlayerState::GetItemAmount(EItemType type, bool respectFieldToQuery = true) const {
  if (int(type) < 0 || type >= EItemType::Max) {
    return 0;
  }
  auto query = EPowerUpFieldToQuery::Actual;
  if (respectFieldToQuery) {
    query = GetPowerUpFieldToQuery(type);
  }
  switch (query) {
  case EPowerUpFieldToQuery::Actual:
    return x24_powerups[int(type)].x0_amount;
  case EPowerUpFieldToQuery::Maximum:
    return GetPowerUpMaxValue(type);
  case EPowerUpFieldToQuery::Minimum:
    return 0;
  }
}
#endif


void CPlayerState::DecrPickup(CPlayerState::EItemType type, u32 amount) {
#ifdef PRIME1
  if (type >= EItemType::Max)
    return;

  if ((type == EItemType::Missiles || type >= EItemType::PowerBombs) && type < EItemType::ThermalVisor)
    x24_powerups[u32(type)].x0_amount -= amount;

#else
  if (int(type) < 0 || type >= EItemType::Max || GetPowerUpFieldToQuery(type) == EPowerUpFieldToQuery::Maximum) {
    return;
  }
  auto& powerup = x24_powerups[u32(type)];
  if (powerup.x0_amount > amount) {
    powerup.x0_amount -= amount;
  } else {
    powerup.x0_amount = 0;
  }
  if (type == EItemType::EnergyTanks) {
    IncrementHealth(0.0f);
  }
#endif
}

void CPlayerState::IncrPickup(EItemType type, u32 amount) {
  if (type >= EItemType::Max)
    return;

  switch (type) {
#ifdef PRIME1
  default:
    break;
  case EItemType::Missiles:
  case EItemType::PowerBombs:
  case EItemType::ChargeBeam:
  case EItemType::SpaceJumpBoots:
  case EItemType::EnergyTanks:
  case EItemType::Truth:
  case EItemType::Strength:
  case EItemType::Elder:
  case EItemType::Wild:
  case EItemType::Lifegiver:
  case EItemType::Warrior:
  case EItemType::Chozo:
  case EItemType::Nature:
  case EItemType::Sun:
  case EItemType::World:
  case EItemType::Spirit:
  case EItemType::Newborn: 
#else
  default:
#endif
  {
    CPowerUp& pup = x24_powerups[u32(type)];
    pup.x0_amount = std::min(pup.x0_amount + u32(amount), pup.x4_capacity);

    if (type == EItemType::EnergyTanks)
      IncrPickup(EItemType::HealthRefill, 9999);
    break;
  }
  case EItemType::HealthRefill:
    xc_health.SetHP(std::min(amount + xc_health.GetHP(), CalculateHealth()));
    break;
  }
}

void CPlayerState::ResetAndIncrPickUp(CPlayerState::EItemType type, u32 amount) {
  x24_powerups[u32(type)].x0_amount = 0;
  IncrPickup(type, amount);
}

float CPlayerState::CalculateHealth() {
  return (GetEnergyTankCapacity() * x24_powerups[u32(EItemType::EnergyTanks)].x0_amount) + GetBaseHealthCapacity();
}

void CPlayerState::AddPowerUp(CPlayerState::EItemType type, u32 capacity) {
  if (type >= EItemType::Max)
    return;

  CPowerUp& pup = x24_powerups[u32(type)];
  pup.x4_capacity = zeus::clamp(0u, pup.x4_capacity + capacity, PowerUpMaxValues[u32(type)]);
  pup.x0_amount = std::min(pup.x0_amount, pup.x4_capacity);
  if (type >= EItemType::PowerSuit && type <= EItemType::PhazonSuit) {
    if (HasPowerUp(EItemType::PhazonSuit))
      x20_currentSuit = EPlayerSuit::Phazon;
    else if (HasPowerUp(EItemType::GravitySuit))
      x20_currentSuit = EPlayerSuit::Gravity;
    else if (HasPowerUp(EItemType::VariaSuit))
      x20_currentSuit = EPlayerSuit::Varia;
    else
      x20_currentSuit = EPlayerSuit::Power;
  }
}

void CPlayerState::ReInitializePowerUp(CPlayerState::EItemType type, u32 capacity) {
  x24_powerups[u32(type)].x4_capacity = 0;
  AddPowerUp(type, capacity);
}

void CPlayerState::InitializeScanTimes() {
  if (x170_scanTimes.size())
    return;

  const auto& scanStates = g_MemoryCardSys->GetScanStates();
  x170_scanTimes.reserve(scanStates.size());
  for (const auto& state : scanStates)
    x170_scanTimes.emplace_back(state.first, 0.f);
}

u32 CPlayerState::GetPowerUpMaxValue(EItemType type) { return PowerUpMaxValues[size_t(type)]; }

CPlayerState::EItemType CPlayerState::ItemNameToType(std::string_view name) {
  static constexpr std::array<std::pair<std::string_view, EItemType>, 46> typeNameMap{{
      {"powerbeam"sv, EItemType::PowerBeam},
      {"icebeam"sv, EItemType::IceBeam},
      {"wavebeam"sv, EItemType::WaveBeam},
      {"plasmabeam"sv, EItemType::PlasmaBeam},
      {"missiles"sv, EItemType::Missiles},
      {"scanvisor"sv, EItemType::ScanVisor},
      {"bombs"sv, EItemType::MorphBallBombs},
      {"ballbombs"sv, EItemType::MorphBallBombs},
      {"morphballbombs"sv, EItemType::MorphBallBombs},
      {"powerbombs"sv, EItemType::PowerBombs},
      {"flamethrower"sv, EItemType::Flamethrower},
      {"thermalvisor"sv, EItemType::ThermalVisor},
      {"chargebeam"sv, EItemType::ChargeBeam},
      {"supermissile"sv, EItemType::SuperMissile},
      {"grapple"sv, EItemType::GrappleBeam},
      {"grapplebeam"sv, EItemType::GrappleBeam},
      {"xrayvisor"sv, EItemType::XRayVisor},
      {"icespreader"sv, EItemType::IceSpreader},
      {"spacejump"sv, EItemType::SpaceJumpBoots},
      {"spacejumpboots"sv, EItemType::SpaceJumpBoots},
      {"morphball"sv, EItemType::MorphBall},
      {"combatvisor"sv, EItemType::CombatVisor},
      {"boostball"sv, EItemType::BoostBall},
      {"spiderball"sv, EItemType::SpiderBall},
      {"powersuit"sv, EItemType::PowerSuit},
      {"gravitysuit"sv, EItemType::GravitySuit},
      {"variasuit"sv, EItemType::VariaSuit},
      {"phazonsuit"sv, EItemType::PhazonSuit},
      {"energytanks"sv, EItemType::EnergyTanks},
      {"unknownitem1"sv, EItemType::UnknownItem1},
      {"healthrefill"sv, EItemType::HealthRefill},
      {"health"sv, EItemType::HealthRefill},
      {"unknownitem2"sv, EItemType::UnknownItem2},
      {"wavebuster"sv, EItemType::Wavebuster},
      {"truth"sv, EItemType::Truth},
      {"strength"sv, EItemType::Strength},
      {"elder"sv, EItemType::Elder},
      {"wild"sv, EItemType::Wild},
      {"lifegiver"sv, EItemType::Lifegiver},
      {"warrior"sv, EItemType::Warrior},
      {"chozo"sv, EItemType::Chozo},
      {"nature"sv, EItemType::Nature},
      {"sun"sv, EItemType::Sun},
      {"world"sv, EItemType::World},
      {"spirit"sv, EItemType::Spirit},
      {"newborn"sv, EItemType::Newborn},
  }};

  std::string lowName{name};
  athena::utility::tolower(lowName);

  const auto iter = std::find_if(typeNameMap.cbegin(), typeNameMap.cend(),
                                 [&lowName](const auto& entry) { return entry.first == lowName; });
  if (iter == typeNameMap.cend()) {
    return EItemType::Invalid;
  }

  return iter->second;
}

std::string_view CPlayerState::ItemTypeToName(CPlayerState::EItemType type) {
  switch (type) {
  case EItemType::PowerBeam:
    return "Power Beam"sv;
  case EItemType::IceBeam:
    return "Ice Beam"sv;
  case EItemType::WaveBeam:
    return "Wave Beam"sv;
  case EItemType::PlasmaBeam:
    return "Plasma Beam"sv;
  case EItemType::Missiles:
    return "Missiles"sv;
  case EItemType::ScanVisor:
    return "Scan Visor"sv;
  case EItemType::MorphBallBombs:
    return "Morph Ball Bombs"sv;
  case EItemType::PowerBombs:
    return "Power Bombs"sv;
  case EItemType::Flamethrower:
    return "Flamethrower"sv;
  case EItemType::ThermalVisor:
    return "Thermal Visor"sv;
  case EItemType::ChargeBeam:
    return "Charge Beam"sv;
  case EItemType::SuperMissile:
    return "Super Missile"sv;
  case EItemType::GrappleBeam:
    return "Grapple Beam"sv;
  case EItemType::XRayVisor:
    return "X-Ray Visor"sv;
  case EItemType::IceSpreader:
    return "Ice Spreader"sv;
  case EItemType::SpaceJumpBoots:
    return "Space Jump Boots"sv;
  case EItemType::MorphBall:
    return "Morph Ball"sv;
  case EItemType::CombatVisor:
    return "Combat Visor"sv;
  case EItemType::BoostBall:
    return "Boost Ball"sv;
  case EItemType::SpiderBall:
    return "Spider Ball"sv;
  case EItemType::PowerSuit:
    return "Power Suit"sv;
  case EItemType::GravitySuit:
    return "Gravity Suit"sv;
  case EItemType::VariaSuit:
    return "Varia Suit"sv;
  case EItemType::PhazonSuit:
    return "Phazon Suit"sv;
  case EItemType::EnergyTanks:
    return "Energy Tanks"sv;
  case EItemType::HealthRefill:
    return "Health Refill"sv;
  case EItemType::Wavebuster:
    return "Wavebuster"sv;
  case EItemType::Truth:
    return "Artifact of Truth"sv;
  case EItemType::Strength:
    return "Artifact of Strength"sv;
  case EItemType::Elder:
    return "Artifact of Elder"sv;
  case EItemType::Wild:
    return "Artifact of Wild"sv;
  case EItemType::Lifegiver:
    return "Artifact of Lifegiver"sv;
  case EItemType::Warrior:
    return "Artifact of Warrior"sv;
  case EItemType::Chozo:
    return "Artifact of Chozo"sv;
  case EItemType::Nature:
    return "Artifact of Nature"sv;
  case EItemType::Sun:
    return "Artifact of Sun"sv;
  case EItemType::World:
    return "Artifact of World"sv;
  case EItemType::Spirit:
    return "Artifact of Spirit"sv;
  case EItemType::Newborn:
    return "Artifact of Newborn"sv;
  default:
    return "[unknown]"sv;
  }
}


#ifdef PRIME2

void CPlayerState::IncrementHealth(float delta) {
  xc_health.SetHP(zeus::clamp(0.0f, xc_health.GetHP() + delta, CalculateHealth()));
}

CPlayerState::EPowerUpFieldToQuery CPlayerState::GetPowerUpFieldToQuery(EItemType type) const {
  switch (type) {
  case EItemType::MorphBall:
    if (x24_powerups[int(EItemType::DeathBall)].x0_amount != 0) {
      return EPowerUpFieldToQuery::Minimum;
    }
    if (x24_powerups[int(EItemType::DisableBall)].x0_amount != 0) {
      return EPowerUpFieldToQuery::Minimum;
    }
    if (x24_powerups[int(EItemType::Unknown_91)].x0_amount != 0) {
      return EPowerUpFieldToQuery::Minimum;
    }
    break;
  case EItemType::BoostBall:
  case EItemType::SpiderBall:
  case EItemType::MorphBallBombs:
  case EItemType::PowerBombs:
    if (x24_powerups[int(EItemType::DeathBall)].x0_amount != 0) {
      return EPowerUpFieldToQuery::Minimum;
    }
    break;
  case EItemType::SpaceJumpBoots:
    if (x24_powerups[int(EItemType::DisableSpaceJump)].x0_amount != 0) {
      return EPowerUpFieldToQuery::Minimum;
    }
    break;
  case EItemType::Missiles:
    if (x24_powerups[int(EItemType::DisableMissiles)].x0_amount != 0) {
      return EPowerUpFieldToQuery::Minimum;
    }
    if (x24_powerups[int(EItemType::UnlimitedMissiles)].x0_amount != 0) {
      return EPowerUpFieldToQuery::Maximum;
    }
    break;
  case EItemType::DarkAmmo:
  case EItemType::LightAmmo:
    if (x24_powerups[int(EItemType::DisableBeamAmmo)].x0_amount != 0) {
      return EPowerUpFieldToQuery::Minimum;
    }
    if (x24_powerups[int(EItemType::UnlimitedBeamAmmo)].x0_amount != 0) {
      return EPowerUpFieldToQuery::Maximum;
    }
  }
  return EPowerUpFieldToQuery::Actual;
}
#endif

} // namespace metaforce
