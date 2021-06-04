#pragma once

#include <string_view>
#include <vector>

#include "Runtime/CStaticInterference.hpp"
#include "Runtime/IOStreams.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/World/CHealthInfo.hpp"

namespace metaforce {

class CPlayerState {
  friend class CWorldTransManager;

public:
  enum class EItemType : s32 {
    Invalid = -1,
    PowerBeam = 0,
    IceBeam = 1,
    WaveBeam = 2,
    PlasmaBeam = 3,
    Missiles = 4,
    ScanVisor = 5,
    MorphBallBombs = 6,
    PowerBombs = 7,
    Flamethrower = 8,
    ThermalVisor = 9,
    ChargeBeam = 10,
    SuperMissile = 11,
    GrappleBeam = 12,
    XRayVisor = 13,
    IceSpreader = 14,
    SpaceJumpBoots = 15,
    MorphBall = 16,
    CombatVisor = 17,
    BoostBall = 18,
    SpiderBall = 19,
    PowerSuit = 20,
    GravitySuit = 21,
    VariaSuit = 22,
    PhazonSuit = 23,
    EnergyTanks = 24,
    UnknownItem1 = 25,
    HealthRefill = 26,
    UnknownItem2 = 27,
    Wavebuster = 28,
    Truth = 29,
    Strength = 30,
    Elder = 31,
    Wild = 32,
    Lifegiver = 33,
    Warrior = 34,
    Chozo = 35,
    Nature = 36,
    Sun = 37,
    World = 38,
    Spirit = 39,
    Newborn = 40,

    #ifdef PRIME2
    //PowerBeam,
    DarkBeam,
    LightBeam,
    AnnihilatorBeam,
    //SuperMissile,
    Darkburst,
    Sunburst,
    SonicBoom,
    //CombatVisor,
    //ScanVisor,
    DarkVisor,
    EchoVisor,
    //VariaSuit,
    DarkSuit,
    LightSuit,
    //MorphBall,
    //BoostBall,
    //SpiderBall,
    //MorphBallBombs,
    Unknown01,
    Unknown02,
    Unknown03,
    //ChargeBeam,
    //GrappleBeam,
    //SpaceJumpBoots,
    GravityBoost,
    SeekerLauncher,
    ScrewAttack,
    Unknown04,
    TempleKey1,
    TempleKey2,
    TempleKey3,
    AgonKey1,
    AgonKey2,
    AgonKey3,
    TorvusKey1,
    TorvusKey2,
    TorvusKey3,
    HiveKey1,
    HiveKey2,
    HiveKey3,
    //HealthRefill,
    //EnergyTank,
    //Powerbomb,
    //Missile,
    DarkAmmo,
    LightAmmo,
    ItemPercentage,
    Unknown_48,
    Unknown_49,
    Unknown_50,
    Unknown_51,
    Unknown_52,
    Unknown_53,
    Unknown_54,
    Unknown_55,
    Unknown_56,
    Invisibility,
    DoubleDamage,
    Invincibility,
    Unknown_60,
    Unknown_61,
    Unknown_62,
    Unknown_63,
    Unknown_64,
    Unknown_65,
    Unknown_66,
    Unknown_67,
    Unknown_68,
    Unknown_69,
    Unknown_70,
    Unused1,
    Unused2,
    Unused3,
    Unused4,
    Unknown_76,
    Unknown_77,
    Unknown_78,
    Unknown_79,
    Unknown_80,
    Unknown_81,
    UnlimitedMissiles,
    UnlimitedBeamAmmo,
    DarkShield,
    LightShield,
    AbsorbAttack,
    DeathBall,
    ScanVirus,
    Unknown_88,
    DisableBeamAmmo,
    DisableMissiles,
    Unknown_91,
    DisableBall,
    DisableSpaceJump,
    Unknown_94,
    HackedEffect,
    CannonBall,
    VioletTranslator,
    AmberTranslator,
    EmeraldTranslator,
    CobaltTranslator,
    TempleKey4,
    TempleKey5,
    TempleKey6,
    TempleKey7,
    TempleKey8,
    TempleKey9,
    EnergyTransferModule,
    ChargeCombo,
    #endif

    /* This must remain at the end of the list */
    Max
  };

  enum class EPlayerVisor : u32 {
    Combat,
    XRay,
    Scan,
    Thermal,

    /* This must remain at the end of the list */
    Max
  };

  enum class EPlayerSuit : s32 {
    Invalid = -1,
    Power,
    Gravity,
    Varia,
    Phazon,
    FusionPower,
    FusionGravity,
    FusionVaria,
    FusionPhazon
  };

  enum class EBeamId : s32 { Invalid = -1, Power, Ice, Wave, Plasma, Phazon, Phazon2 = 27 };

  #ifdef PRIME2
  enum class EPowerUpFieldToQuery : u32 {
    Actual,
    Minimum,
    Maximum
  };
  #endif

private:
  struct CPowerUp {
    u32 x0_amount = 0;
    u32 x4_capacity = 0;
    constexpr CPowerUp() = default;
    constexpr CPowerUp(u32 amount, u32 capacity) : x0_amount(amount), x4_capacity(capacity) {}
  };
  bool x0_24_alive : 1 = true;
  bool x0_25_firingComboBeam : 1 = false;
  bool x0_26_fusion : 1 = false;
  u32 x4_enabledItems = 0;
  EBeamId x8_currentBeam = EBeamId::Power;
  CHealthInfo xc_health = {99.f, 50.f};
  EPlayerVisor x14_currentVisor = EPlayerVisor::Combat;
  EPlayerVisor x18_transitioningVisor = x14_currentVisor;
  float x1c_visorTransitionFactor = 0.2f;
  EPlayerSuit x20_currentSuit = EPlayerSuit::Power;
  rstl::reserved_vector<CPowerUp, 41> x24_powerups;
  std::vector<std::pair<CAssetId, float>> x170_scanTimes;
  std::pair<u32, u32> x180_scanCompletionRate = {};
  CStaticInterference x188_staticIntf{5};

  bool m_canTakeDamage = true;

public:
  u32 GetMissileCostForAltAttack() const;
  float GetComboFireAmmoPeriod() const;
  static constexpr float GetMissileComboChargeFactor() { return 1.8f; }
  u32 CalculateItemCollectionRate() const;

  CHealthInfo& GetHealthInfo();
  const CHealthInfo& GetHealthInfo() const;
  u32 GetPickupTotal() const { return 99; }
  void SetIsFusionEnabled(bool val) { x0_26_fusion = val; }
  bool IsFusionEnabled() const { return x0_26_fusion; }
  EPlayerSuit GetCurrentSuit() const;
  EPlayerSuit GetCurrentSuitRaw() const { return x20_currentSuit; }
  EBeamId GetCurrentBeam() const { return x8_currentBeam; }
  void SetCurrentBeam(EBeamId beam) { x8_currentBeam = beam; }
  bool CanVisorSeeFog(const CStateManager& stateMgr) const;
  EPlayerVisor GetCurrentVisor() const { return x14_currentVisor; }
  EPlayerVisor GetTransitioningVisor() const { return x18_transitioningVisor; }
  EPlayerVisor GetActiveVisor(const CStateManager& stateMgr) const;
  void UpdateStaticInterference(CStateManager& stateMgr, float dt);
  void IncreaseScanTime(u32 time, float val);
  void SetScanTime(CAssetId res, float time);
  float GetScanTime(CAssetId time) const;
  bool GetIsVisorTransitioning() const;
  float GetVisorTransitionFactor() const;
  void UpdateVisorTransition(float dt);
  void StartTransitionToVisor(EPlayerVisor visor);
  void ResetVisor();
  bool ItemEnabled(EItemType type) const;
  void DisableItem(EItemType type);
  void EnableItem(EItemType type);
  bool HasPowerUp(EItemType type) const;
  u32 GetItemCapacity(EItemType type) const;
#ifdef PRIME1
  u32 GetItemAmount(EItemType type) const;
#else
  u32 GetItemAmount(EItemType type, bool respectFieldToQuery = true) const;
#endif
  void DecrPickup(EItemType type, u32 amount);
  void IncrPickup(EItemType type, u32 amount);
  void ResetAndIncrPickUp(EItemType type, u32 amount);
  static float GetEnergyTankCapacity() { return 100.f; }
  static float GetBaseHealthCapacity() { return 99.f; }
  float CalculateHealth();
  void ReInitializePowerUp(EItemType type, u32 capacity);
  void AddPowerUp(EItemType type, u32 capacity);
  u32 GetLogScans() const { return x180_scanCompletionRate.first; }
  u32 GetTotalLogScans() const { return x180_scanCompletionRate.second; }
  void SetScanCompletionRate(const std::pair<u32, u32>& p) { x180_scanCompletionRate = p; }
  bool IsPlayerAlive() const { return x0_24_alive; }
  void SetPlayerAlive(bool alive) { x0_24_alive = alive; }
  bool IsFiringComboBeam() const { return x0_25_firingComboBeam; }
  void SetFiringComboBeam(bool f) { x0_25_firingComboBeam = f; }
  void InitializeScanTimes();
  CStaticInterference& GetStaticInterference() { return x188_staticIntf; }
  const std::vector<std::pair<CAssetId, float>>& GetScanTimes() const { return x170_scanTimes; }
  CPlayerState();
  explicit CPlayerState(CBitStreamReader& stream);
  void PutTo(CBitStreamWriter& stream);
  static u32 GetPowerUpMaxValue(EItemType type);
  static EItemType ItemNameToType(std::string_view name);
  static std::string_view ItemTypeToName(EItemType type);
  bool CanTakeDamage() const { return m_canTakeDamage; }
  void SetCanTakeDamage(bool c) { m_canTakeDamage = c; }

  #ifdef PRIME2
  void IncrementHealth(float delta);
  EPowerUpFieldToQuery GetPowerUpFieldToQuery(EItemType type) const;
  #endif
};
} // namespace metaforce
