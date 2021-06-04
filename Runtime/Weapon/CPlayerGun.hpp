#pragma once

#include <array>
#include <memory>
#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Character/CActorLights.hpp"
#include "Runtime/Character/CModelData.hpp"
#include "Runtime/Graphics/CRainSplashGenerator.hpp"
#include "Runtime/Graphics/Shaders/CAABoxShader.hpp"
#include "Runtime/Graphics/Shaders/CTexturedQuadFilter.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Weapon/CAuxWeapon.hpp"
#include "Runtime/Weapon/CFidget.hpp"
#include "Runtime/Weapon/CGrappleArm.hpp"
#include "Runtime/Weapon/CGunMotion.hpp"
#include "Runtime/Weapon/CIceBeam.hpp"
#include "Runtime/Weapon/CPhazonBeam.hpp"
#include "Runtime/Weapon/CPlasmaBeam.hpp"
#include "Runtime/Weapon/CPowerBeam.hpp"
#include "Runtime/Weapon/CWaveBeam.hpp"
#include "Runtime/World/CPlayerCameraBob.hpp"
#include "Runtime/World/CWorldShadow.hpp"
#include "Runtime/World/ScriptObjectSupport.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CTransform.hpp>

namespace metaforce {
struct CFinalInput;

class CPlayerGun {
public:
  static float skTractorBeamFactor;
  enum class EMissileMode { Inactive, Active };
  enum class EBWeapon { Bomb, PowerBomb };
  enum class EPhazonBeamState { Inactive, Entering, Exiting, Active };
  enum class EChargePhase {
    NotCharging,
    ChargeRequested,
    AnimAndSfx,
    FxGrowing,
    FxGrown,
    ComboXfer,
    ComboXferDone,
    ComboFire,
    ComboFireDone,
    ChargeCooldown,
    ChargeDone
  };
  enum class ENextState {
    StatusQuo,
    EnterMissile,
    ExitMissile,
    MissileShotDone,
    MissileReload,
    ChangeWeapon,
    SetupBeam,
    Seven,
    EnterPhazonBeam,
    ExitPhazonBeam
  };
  enum class EIdleState { NotIdle, Wander, Idle, Three, Four };

private:
  class CGunMorph {
  public:
    enum class EGunState { InWipeDone, OutWipeDone, InWipe, OutWipe };
    enum class EMorphEvent { None, InWipeDone, OutWipeDone };
    enum class EDir { In, Out, Done };

  private:
    float x0_yLerp = 0.f;
    float x4_gunTransformTime;
    float x8_remTime = 0.f;
    float xc_speed = 0.1f;
    float x10_holoHoldTime;
    float x14_remHoldTime = 2.f;
    float x18_transitionFactor = 1.f;
    EDir x1c_dir = EDir::Done;
    EGunState x20_gunState = EGunState::OutWipeDone;
    bool x24_24_morphing : 1 = false;
    bool x24_25_weaponChanged : 1 = false;

  public:
    CGunMorph(float gunTransformTime, float holoHoldTime)
    : x4_gunTransformTime(gunTransformTime)
    , x10_holoHoldTime(std::fabs(holoHoldTime)) {}
    float GetYLerp() const { return x0_yLerp; }
    float GetTransitionFactor() const { return x18_transitionFactor; }
    EGunState GetGunState() const { return x20_gunState; }
    void SetWeaponChanged() { x24_25_weaponChanged = true; }
    EMorphEvent Update(float inY, float outY, float dt);
    void StartWipe(EDir dir);
  };

  class CMotionState {
  public:
    enum class EMotionState { Zero, One, LockOn, CancelLockOn };
    enum class EFireState { NotFiring, StartFire, Firing };

  private:
    static float gGunExtendDistance;
    bool x0_24_extendParabola = true;
    float x4_extendParabolaDelayTimer = 0.f;
    float x8_fireTime = 0.f;
    float xc_curExtendDist = 0.f;
    float x10_curRotation = 0.f;
    float x14_rotationT = 0.f;
    float x18_startRotation = 0.f;
    float x1c_endRotation = 0.f;
    EMotionState x20_state = EMotionState::Zero;
    EFireState x24_fireState = EFireState::NotFiring;

  public:
    static void SetExtendDistance(float d) { gGunExtendDistance = d; }
    void SetState(EMotionState state) { x20_state = state; }
    void Update(bool firing, float dt, zeus::CTransform& xf, CStateManager& mgr);
  };

  CActorLights x0_lights;
  CSfxHandle x2e0_chargeSfx;
  CSfxHandle x2e4_invalidSfx;
  CSfxHandle x2e8_phazonBeamSfx;
  // 0x1: FireOrBomb, 0x2: MissileOrPowerBomb
  u32 x2ec_lastFireButtonStates = 0;
  u32 x2f0_pressedFireButtonStates = 0;
  u32 x2f4_fireButtonStates = 0;
  // 0x1: beam mode, 0x2: missile mode, 0x4: missile ready, 0x8: morphing, 0x10: combo fire
  u32 x2f8_stateFlags = 0x1;
  u32 x2fc_fidgetAnimBits = 0;
  u32 x300_remainingMissiles = 0;
  u32 x304_ = 0;
  u32 x308_bombCount = 3;
  u32 x30c_rapidFireShots = 0;
  CPlayerState::EBeamId x310_currentBeam = CPlayerState::EBeamId::Power;
  CPlayerState::EBeamId x314_nextBeam = CPlayerState::EBeamId::Power;
  u32 x318_comboAmmoIdx = 0;
  EMissileMode x31c_missileMode = EMissileMode::Inactive;
  CPlayerState::EBeamId x320_currentAuxBeam = CPlayerState::EBeamId::Power;
  EIdleState x324_idleState = EIdleState::Four;
  float x328_animSfxPitch = 0.f;
  EChargePhase x32c_chargePhase = EChargePhase::NotCharging;
  EChargeState x330_chargeState = EChargeState::Normal;
  u32 x334_ = 0;
  ENextState x338_nextState = ENextState::StatusQuo;
  EPhazonBeamState x33c_phazonBeamState = EPhazonBeamState::Inactive;
  float x340_chargeBeamFactor = 0.f;
  float x344_comboXferTimer = 0.f;
  float x348_chargeCooldownTimer = 0.f;
  float x34c_shakeX = 0.f;
  float x350_shakeZ = 0.f;
  float x354_bombFuseTime;
  float x358_bombDropDelayTime;
  float x35c_bombTime = 0.f;
  float x360_ = 0.f;
  float x364_gunStrikeCoolTimer = 0.f;
  float x368_idleWanderDelayTimer = 0.f;
  float x36c_ = 1.f;
  float x370_gunMotionSpeedMult = 1.f;
  float x374_ = 0.f;
  float x378_shotSmokeStartTimer = 0.f;
  float x37c_rapidFireShotsDecayTimer = 0.f;
  float x380_shotSmokeTimer = 0.f;
  float x384_gunStrikeDelayTimer = 0.f;
  float x388_enterFreeLookDelayTimer = 0.f;
  float x38c_muzzleEffectVisTimer = 0.f;
  float x390_cooldown = 0.f;
  float x394_damageTimer = 0.f;
  float x398_damageAmt = 0.f;
  float x39c_phazonMorphT = 0.f;
  float x3a0_missileExitTimer = 0.f;
  CFidget x3a4_fidget;
  zeus::CVector3f x3dc_damageLocation;
  zeus::CTransform x3e8_xf;
  zeus::CTransform x418_beamLocalXf;
  zeus::CTransform x448_elbowWorldXf;
  zeus::CTransform x478_assistAimXf;
  zeus::CTransform x4a8_gunWorldXf;
  zeus::CTransform x4d8_gunLocalXf;
  zeus::CTransform x508_elbowLocalXf;
  TUniqueId x538_playerId;
  TUniqueId x53a_powerBomb = kInvalidUniqueId;
  TUniqueId x53c_lightId = kInvalidUniqueId;
  std::vector<CToken> x540_handAnimTokens;
  CPlayerCameraBob x550_camBob;
  u32 x658_ = 1;
  float x65c_ = 0.f;
  float x660_ = 0.f;
  float x664_ = 0.f;
  float x668_aimVerticalSpeed;
  float x66c_aimHorizontalSpeed;
  std::pair<u16, CSfxHandle> x670_animSfx{0xffff, {}};
  CGunMorph x678_morph;
  CMotionState x6a0_motionState;
  zeus::CAABox x6c8_hologramClipCube;
  CModelData x6e0_rightHandModel;
  CGunWeapon* x72c_currentBeam = nullptr;
  CGunWeapon* x730_outgoingBeam = nullptr;
  CGunWeapon* x734_loadingBeam = nullptr;
  CGunWeapon* x738_nextBeam = nullptr;
  std::unique_ptr<CGunMotion> x73c_gunMotion;
  std::unique_ptr<CGrappleArm> x740_grappleArm;
  std::unique_ptr<CAuxWeapon> x744_auxWeapon;
  std::unique_ptr<CRainSplashGenerator> x748_rainSplashGenerator;
  std::unique_ptr<CPowerBeam> x74c_powerBeam;
  std::unique_ptr<CIceBeam> x750_iceBeam;
  std::unique_ptr<CWaveBeam> x754_waveBeam;
  std::unique_ptr<CPlasmaBeam> x758_plasmaBeam;
  std::unique_ptr<CPhazonBeam> x75c_phazonBeam;
  std::array<CGunWeapon*, 4> x760_selectableBeams{}; // Used to be reserved_vector
  std::unique_ptr<CElementGen> x774_holoTransitionGen;
  std::unique_ptr<CElementGen> x77c_comboXferGen;
  rstl::reserved_vector<rstl::reserved_vector<TLockedToken<CGenDescription>, 2>, 2> x784_bombEffects;
  rstl::reserved_vector<TLockedToken<CGenDescription>, 5> x7c0_auxMuzzleEffects;
  rstl::reserved_vector<std::unique_ptr<CElementGen>, 5> x800_auxMuzzleGenerators;
  std::unique_ptr<CWorldShadow> x82c_shadow;
  s16 x830_chargeRumbleHandle = -1;

  bool x832_24_coolingCharge : 1 = false;
  bool x832_25_chargeEffectVisible : 1 = false;
  bool x832_26_comboFiring : 1 = false;
  bool x832_27_chargeAnimStarted : 1 = false;
  bool x832_28_readyForShot : 1 = false;
  bool x832_29_lockedOn : 1 = false;
  bool x832_30_requestReturnToDefault : 1 = false;
  bool x832_31_inRestPose : 1 = true;

  bool x833_24_notFidgeting : 1 = true;
  bool x833_25_ : 1 = false;
  bool x833_26_ : 1 = false;
  bool x833_27_ : 1 = false;
  bool x833_28_phazonBeamActive : 1 = false;
  bool x833_29_pointBlankWorldSurface : 1 = false;
  bool x833_30_canShowAuxMuzzleEffect : 1 = true;
  bool x833_31_inFreeLook : 1 = false;

  bool x834_24_charging : 1 = false;
  bool x834_25_gunMotionFidgeting : 1 = false;
  bool x834_26_animPlaying : 1 = false;
  bool x834_27_underwater : 1 = false;
  bool x834_28_requestImmediateRecharge : 1 = false;
  bool x834_29_frozen : 1 = false;
  bool x834_30_inBigStrike : 1 = false;
  bool x834_31_gunMotionInFidgetBasePosition : 1 = false;

  bool x835_24_canFirePhazon : 1 = false;
  bool x835_25_inPhazonBeam : 1 = false;
  bool x835_26_phazonBeamMorphing : 1 = false;
  bool x835_27_intoPhazonBeam : 1 = false;
  bool x835_28_bombReady : 1 = false;
  bool x835_29_powerBombReady : 1 = false;
  bool x835_30_inPhazonPool : 1 = false;
  bool x835_31_actorAttached : 1 = false;

  CTexturedQuadFilter m_screenQuad{EFilterType::Blend, CGraphics::g_SpareTexture.get(),
                                   CTexturedQuadFilter::ZTest::GEqualZWrite};
  CAABoxShader m_aaboxShader{true};

  void InitBeamData();
  void InitBombData();
  void InitMuzzleData();
  void InitCTData();
  void LoadHandAnimTokens();
  void CreateGunLight(CStateManager& mgr);
  void DeleteGunLight(CStateManager& mgr);
  void UpdateGunLight(const zeus::CTransform& xf, CStateManager& mgr);
  void SetGunLightActive(bool active, CStateManager& mgr);
  void SetPhazonBeamMorph(bool intoPhazonBeam);
  void Reset(CStateManager& mgr, bool b1);
  void ResetBeamParams(CStateManager& mgr, const CPlayerState& playerState, bool playSelectionSfx);
  void PlayAnim(NWeaponTypes::EGunAnimType type, bool loop);
  void CancelCharge(CStateManager& mgr, bool withEffect);
  bool ExitMissile();
  void StopChargeSound(CStateManager& mgr);
  void UnLoadFidget();
  void ReturnArmAndGunToDefault(CStateManager& mgr, bool returnToDefault);
  void ReturnToRestPose();
  void ChangeWeapon(const CPlayerState& playerState, CStateManager& mgr);
  void GetLctrWithShake(zeus::CTransform& xfOut, const CModelData& mData, std::string_view lctrName, bool shake,
                        bool dyn) const;
  void UpdateLeftArmTransform(const CModelData& mData, const CStateManager& mgr);
  void ProcessGunMorph(float dt, CStateManager& mgr);
  void SetPhazonBeamFeedback(bool active);
  void StartPhazonBeamTransition(bool active, CStateManager& mgr, CPlayerState& playerState);
  void ProcessPhazonGunMorph(float dt, CStateManager& mgr);
  void EnableChargeFx(EChargeState state, CStateManager& mgr);
  void UpdateChargeState(float dt, CStateManager& mgr);
  void UpdateAuxWeapons(float dt, const zeus::CTransform& targetXf, CStateManager& mgr);
  void DoUserAnimEvent(float dt, CStateManager& mgr, const CInt32POINode& node, EUserEventType type);
  void DoUserAnimEvents(float dt, CStateManager& mgr);
  TUniqueId GetTargetId(CStateManager& mgr) const;
  void CancelLockOn();
  void FireSecondary(float dt, CStateManager& mgr);
  void ResetCharged(float dt, CStateManager& mgr);
  void ActivateCombo(CStateManager& mgr);
  void ProcessChargeState(u32 releasedStates, u32 pressedStates, CStateManager& mgr, float dt);
  void ResetNormal(CStateManager& mgr);
  void UpdateNormalShotCycle(float dt, CStateManager& mgr);
  void ProcessNormalState(u32 releasedStates, u32 pressedStates, CStateManager& mgr, float dt);
  void UpdateWeaponFire(float dt, const CPlayerState& playerState, CStateManager& mgr);
  void EnterFreeLook(CStateManager& mgr);
  void SetFidgetAnimBits(int animSet, bool beamOnly);
  void AsyncLoadFidget(CStateManager& mgr);
  bool IsFidgetLoaded() const;
  void EnterFidget(CStateManager& mgr);
  void UpdateGunIdle(bool inStrikeCooldown, float camBobT, float dt, CStateManager& mgr);
  void RenderEnergyDrainEffects(const CStateManager& mgr) const;
  void DrawArm(const CStateManager& mgr, const zeus::CVector3f& pos, const CModelFlags& flags) const;
  zeus::CVector3f ConvertToScreenSpace(const zeus::CVector3f& pos, const CGameCamera& cam) const;
  static void CopyScreenTex();
  void DrawScreenTex(float z);
  void DrawClipCube(const zeus::CAABox& aabb);

public:
  explicit CPlayerGun(TUniqueId playerId);
  void TakeDamage(bool bigStrike, bool notFromMetroid, CStateManager& mgr);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void AsyncLoadSuit(CStateManager& mgr);
  void TouchModel(const CStateManager& stateMgr);
  EMissileMode GetMissleMode() const { return x31c_missileMode; }
  bool IsFidgeting() const { return x833_24_notFidgeting; }
  bool IsCharging() const { return x834_24_charging; }
  float GetChargeBeamFactor() const { return x340_chargeBeamFactor; }
  bool IsBombReady() const { return x835_28_bombReady; }
  u32 GetBombCount() const { return x308_bombCount; }
  bool IsPowerBombReady() const { return x835_29_powerBombReady; }
  CPlayerState::EBeamId GetCurrentBeam() const { return x310_currentBeam; }
  CPlayerState::EBeamId GetNextBeam() const { return x314_nextBeam; }
  const CGunMorph& GetGunMorph() const { return x678_morph; }
  float GetHoloTransitionFactor() const { return x678_morph.GetTransitionFactor(); }
  void SetTransform(const zeus::CTransform& xf) { x3e8_xf = xf; }
  void SetAssistAimTransform(const zeus::CTransform& xf) { x478_assistAimXf = xf; }
  CGrappleArm& GetGrappleArm() { return *x740_grappleArm; }
  const CGrappleArm& GetGrappleArm() const { return *x740_grappleArm; }
  void DamageRumble(const zeus::CVector3f& location, float damage, const CStateManager& mgr);
  void ResetCharge(CStateManager& mgr, bool resetBeam);
  void HandleBeamChange(const CFinalInput& input, CStateManager& mgr);
  void HandlePhazonBeamChange(CStateManager& mgr);
  void HandleWeaponChange(const CFinalInput& input, CStateManager& mgr);
  void ProcessInput(const CFinalInput& input, CStateManager& mgr);
  void ResetIdle(CStateManager& mgr);
  void CancelFiring(CStateManager& mgr);
  float GetBeamVelocity() const;
  void StopContinuousBeam(CStateManager& mgr, bool b1);
  void Update(float grappleSwingT, float cameraBobT, float dt, CStateManager& mgr);
  void PreRender(const CStateManager& mgr, const zeus::CFrustum& frustum, const zeus::CVector3f& camPos);
  void Render(const CStateManager& mgr, const zeus::CVector3f& pos, const CModelFlags& flags);
  void AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const;
  u32 GetLastFireButtonStates() const { return x2ec_lastFireButtonStates; }
  void DropBomb(EBWeapon weapon, CStateManager& mgr);
  TUniqueId DropPowerBomb(CStateManager& mgr);
  void SetActorAttached(bool b) { x835_31_actorAttached = b; }
  CAuxWeapon& GetAuxWeapon() const { return *x744_auxWeapon; }

  #ifdef PRIME2
  zeus::CVector3f GetVector_801c6df8() {
      return zeus::CVector3f(); 
  };
  #endif
};

} // namespace metaforce
