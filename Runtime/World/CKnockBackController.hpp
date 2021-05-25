#pragma once

#include <bitset>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Character/CharacterCommon.hpp"
#include "Runtime/Weapon/WeaponCommon.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class CDamageInfo;
class CPatterned;

enum class EKnockBackType { Radius, Direct };

enum class EKnockBackVariant { Small, Medium, Large };

enum class EKnockBackWeaponType {
  Invalid = -1,
  Power,
  PowerCharged,
  PowerComboed,
  PowerComboedDirect,
  Wave,
  WaveCharged,
  WaveComboed,
  WaveComboedDirect,
  Ice,
  IceCharged,
  IceComboed,
  IceComboedDirect,
  Plasma,
  PlasmaCharged,
  PlasmaComboed,
  Missile,
  Bomb,
  PowerBomb,
  Phazon
};

enum class EKnockBackCharacterState { Alive, Dead, FrozenAlive, FrozenDead };

enum class EKnockBackAnimationState { Invalid = -1, None, Flinch, KnockBack, Hurled, Fall };

enum class EKnockBackAnimationFollowUp {
  Invalid = -1,
  None,
  Freeze,
  Shock,
  Burn,
  PhazeOut,
  Death,
  ExplodeDeath,
  IceDeath,
  BurnDeath,
  LaggedBurnDeath
};

class CKnockBackController {
public:
  struct KnockBackParms {
    EKnockBackAnimationState x0_animState = EKnockBackAnimationState::None;
    EKnockBackAnimationFollowUp x4_animFollowup = EKnockBackAnimationFollowUp::None;
    float x8_followupDuration = 0.f;
    float xc_intoFreezeDur = 0.f;
  };

private:
  friend class CPatterned;
  EKnockBackVariant x0_variant;
  KnockBackParms x4_activeParms{};
  EWeaponType x14_deferWeaponType = EWeaponType::None;
  EKnockBackAnimationState x18_minAnimState = EKnockBackAnimationState::None;
  EKnockBackAnimationState x1c_maxAnimState = EKnockBackAnimationState::Fall;
  u32 x20_impulseDurationIdx = 0;
  rstl::reserved_vector<std::pair<float, float>, 5> x24_;
  zeus::CVector3f x50_impulseDir;
  float x5c_impulseMag = 0.f;
  float x60_impulseRemTime = 0.f;
  float x64_flinchRemTime = 0.f;
  float x68_deferRemTime = 0.f;
  u32 x6c_ = 0;
  u32 x70_ = 0;
  u32 x74_ = 0;
  pas::ESeverity x7c_severity = pas::ESeverity::One;
  std::bitset<5> x80_availableStates{0b11111};
  bool x81_24_autoResetImpulse : 1 = true;
  bool x81_25_enableFreeze : 1 = true;
  bool x81_26_enableShock : 1 = false;
  bool x81_27_enableBurn : 1 = true;
  bool x81_28_enableBurnDeath : 1 = true;
  bool x81_29_enableExplodeDeath : 1 = true;
  bool x81_30_enableLaggedBurnDeath : 1 = true;
  bool x81_31_ : 1 = true;
  bool x82_24_ : 1 = true;
  bool x82_25_inDeferredKnockBack : 1 = false;
  bool x82_26_locomotionDuringElectrocution : 1 = false;
  void ApplyImpulse(float dt, CPatterned& parent);
  bool TickDeferredTimer(float dt);
  EKnockBackCharacterState GetKnockBackCharacterState(const CPatterned& parent) const;
  void ValidateState(const CPatterned& parent);
  float CalculateExtraHurlVelocity(CStateManager& mgr, float magnitude, float kbResistance) const;
  void DoKnockBackAnimation(const zeus::CVector3f& backVec, CStateManager& mgr, CPatterned& parent, float magnitude);
  void ResetKnockBackImpulse(const CPatterned& parent, const zeus::CVector3f& backVec, float magnitude);
  void DoDeferredKnockBack(CStateManager& mgr, CPatterned& parent);
  EKnockBackWeaponType GetKnockBackWeaponType(const CDamageInfo& info, EWeaponType wType, EKnockBackType type);
  void SelectDamageState(const CPatterned& parent, const CDamageInfo& info, EWeaponType wType, EKnockBackType type);

public:
  explicit CKnockBackController(EKnockBackVariant variant);
  void SetKnockBackVariant(EKnockBackVariant v) { x0_variant = v; }
  void DeferKnockBack(EWeaponType tp) {
    x14_deferWeaponType = tp;
    x68_deferRemTime = 0.05f;
  }
  void sub80233d40(int i, float f1, float f2);
  void SetAutoResetImpulse(bool b);
  void SetImpulseDurationIdx(u32 i) { x20_impulseDurationIdx = i; }
  void SetAnimationStateRange(EKnockBackAnimationState a, EKnockBackAnimationState b) {
    x18_minAnimState = a;
    x1c_maxAnimState = b;
  }
  void Update(float dt, CStateManager& mgr, CPatterned& parent);
  void KnockBack(const zeus::CVector3f& backVec, CStateManager& mgr, CPatterned& parent, const CDamageInfo& info,
                 EKnockBackType type, float magnitude);
  void SetSeverity(pas::ESeverity v) { x7c_severity = v; }
  void SetEnableFreeze(bool b) { x81_25_enableFreeze = b; }
  bool GetEnableFreeze() const { return x81_25_enableFreeze; }
  void SetEnableShock(bool b) { x81_26_enableShock = b; }
  void SetEnableBurn(bool b) { x81_27_enableBurn = b; }
  void SetEnableBurnDeath(bool b) { x81_28_enableBurnDeath = b; }
  void SetEnableExplodeDeath(bool b) { x81_29_enableExplodeDeath = b; }
  void SetEnableLaggedBurnDeath(bool b) { x81_30_enableLaggedBurnDeath = b; }
  void SetX81_31(bool b) { x81_31_ = b; }
  void SetX82_24(bool b) { x82_24_ = b; }
  void SetLocomotionDuringElectrocution(bool b) { x82_26_locomotionDuringElectrocution = b; }
  const KnockBackParms& GetActiveParms() const { return x4_activeParms; }
  EKnockBackVariant GetVariant() const { return x0_variant; }
  float GetFlinchRemTime() const { return x64_flinchRemTime; }
  void SetAvailableState(EKnockBackAnimationState s, bool b) { x80_availableStates.set(size_t(s), b); }
  bool TestAvailableState(EKnockBackAnimationState s) const { return x80_availableStates.test(size_t(s)); }
};

} // namespace metaforce
