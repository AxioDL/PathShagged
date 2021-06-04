#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "Runtime/Weapon/CPlayerGun.hpp"
#include "Runtime/Weapon/CWeaponMgr.hpp"
#include "Runtime/World/CMorphBall.hpp"
#include "Runtime/World/CPhysicsActor.hpp"
#include "Runtime/World/CPlayerEnergyDrain.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

#ifdef PRIME2
#include "Runtime/GameGlobalObjects.hpp"
#endif

namespace metaforce {
class CCollidableSphere;
class CDamageInfo;
class CFirstPersonCamera;
class CMaterialList;
class CMorphBall;
class CPlayerCameraBob;
class CPlayerGun;
class CScriptPlayerHint;
class CScriptWater;
class IVisitor;

struct CFinalInput;

class CPlayer : public CPhysicsActor {
  friend class CFirstPersonCamera;
  friend class CGroundMovement;
  friend class CMorphBall;
  friend class CPlayerCameraBob;
  friend class CStateManager;

public:
  enum class EPlayerScanState { NotScanning, Scanning, ScanComplete };

  enum class EPlayerOrbitType { Close, Far, Default };

  enum class EPlayerOrbitState {
    NoOrbit,
    OrbitObject,
    OrbitPoint,
    OrbitCarcass,
    ForcedOrbitObject, // For CMetroidBeta attack
    Grapple
  };

  enum class EPlayerOrbitRequest {
    StopOrbit,
    Respawn,
    EnterMorphBall,
    Default,
    Four,
    Five,
    InvalidateTarget,
    BadVerticalAngle,
    ActivateOrbitSource,
    ProjectileCollide,
    Freeze,
    DamageOnGrapple,
    LostGrappleLineOfSight
  };

  enum class EOrbitValidationResult {
    OK,
    InvalidTarget,
    PlayerNotReadyToTarget,
    NonTargetableTarget,
    ExtremeHorizonAngle,
    BrokenLookAngle,
    TargetingThroughDoor
  };

  enum class EPlayerZoneInfo { Targeting, Scan };

  enum class EPlayerZoneType { Always = -1, Box = 0, Ellipse };

  enum class EPlayerMovementState { OnGround, Jump, ApplyJump, Falling, FallingMorphed };

  enum class EPlayerMorphBallState { Unmorphed, Morphed, Morphing, Unmorphing };

  enum class EPlayerCameraState { FirstPerson, Ball, Two, Transitioning, Spawned };

  enum class ESurfaceRestraints { Normal, Air, Ice, Organic, Water, Lava, Phazon, Shrubbery };

  enum class EFootstepSfx { None, Left, Right };

  enum class EGrappleState { None, Firing, Pull, Swinging, JumpOff };

  enum class EGunHolsterState { Holstered, Drawing, Drawn, Holstering };

private:
  struct CVisorSteam {
    float x0_curTargetAlpha;
    float x4_curAlphaInDur;
    float x8_curAlphaOutDur;
    CAssetId xc_tex;
    float x10_nextTargetAlpha = 0.f;
    float x14_nextAlphaInDur = 0.f;
    float x18_nextAlphaOutDur = 0.f;
    CAssetId x1c_txtr;
    float x20_alpha = 0.f;
    float x24_delayTimer = 0.f;
    bool x28_affectsThermal = false;

  public:
    CVisorSteam(float targetAlpha, float alphaInDur, float alphaOutDur, CAssetId tex)
    : x0_curTargetAlpha(targetAlpha), x4_curAlphaInDur(alphaInDur), x8_curAlphaOutDur(alphaOutDur), xc_tex(tex) {}
    CAssetId GetTextureId() const;
    void SetSteam(float targetAlpha, float alphaInDur, float alphaOutDur, CAssetId txtr, bool affectsThermal);
    void Update(float dt);
    float GetAlpha() const { return x20_alpha; }
    bool AffectsThermal() const { return x28_affectsThermal; }
  };

  class CFailsafeTest {
  public:
    enum class EInputState { Jump, StartingJump, Moving };

  private:
    rstl::reserved_vector<EInputState, 20> x0_stateSamples;
    rstl::reserved_vector<zeus::CVector3f, 20> x54_posSamples;
    rstl::reserved_vector<zeus::CVector3f, 20> x148_velSamples;
    rstl::reserved_vector<zeus::CVector2f, 20> x23c_inputSamples;

  public:
    void Reset();
    void AddSample(EInputState state, const zeus::CVector3f& pos, const zeus::CVector3f& vel,
                   const zeus::CVector2f& input);
    bool Passes() const;
  };

  EPlayerMovementState x258_movementState = EPlayerMovementState::OnGround;
  std::vector<CToken> x25c_ballTransitionsRes;
  TUniqueId x26c_attachedActor = kInvalidUniqueId;
  float x270_attachedActorTime = 0.f;
  CPlayerEnergyDrain x274_energyDrain{4};
  float x288_startingJumpTimeout = 0.f;
  float x28c_sjTimer = 0.f;
  float x290_minJumpTimeout = 0.f;
  float x294_jumpCameraTimer = 0.f;
  u32 x298_jumpPresses = 0;
  float x29c_fallCameraTimer = 0.f;
  float x2a0_ = 0.f;
  bool x2a4_cancelCameraPitch = false;
  float x2a8_timeSinceJump = 1000.f;
  ESurfaceRestraints x2ac_surfaceRestraint = ESurfaceRestraints::Normal;
  u32 x2b0_outOfWaterTicks = 2;
  rstl::reserved_vector<float, 6> x2b4_accelerationTable;
  u32 x2d0_curAcceleration = 3;
  float x2d4_accelerationChangeTimer = 0.f;
  zeus::CAABox x2d8_fpBounds;
  float x2f0_ballTransHeight = 0.f;
  EPlayerCameraState x2f4_cameraState = EPlayerCameraState::FirstPerson;
  EPlayerMorphBallState x2f8_morphBallState = EPlayerMorphBallState::Unmorphed;
  EPlayerMorphBallState x2fc_spawnedMorphBallState = EPlayerMorphBallState::Unmorphed;
  float x300_fallingTime = 0.f;
  EPlayerOrbitState x304_orbitState = EPlayerOrbitState::NoOrbit;
  EPlayerOrbitType x308_orbitType = EPlayerOrbitType::Close;
  EPlayerOrbitRequest x30c_orbitRequest = EPlayerOrbitRequest::Default;
  TUniqueId x310_orbitTargetId = kInvalidUniqueId;
  zeus::CVector3f x314_orbitPoint;
  zeus::CVector3f x320_orbitVector;
  float x32c_orbitModeTimer = 0.f;
  EPlayerZoneInfo x330_orbitZoneMode = EPlayerZoneInfo::Targeting;
  EPlayerZoneType x334_orbitType = EPlayerZoneType::Ellipse;
  u32 x338_ = 1;
  TUniqueId x33c_orbitNextTargetId = kInvalidUniqueId;
  bool m_deferredOrbitObject = false;
  float x340_ = 0.f;
  std::vector<TUniqueId> x344_nearbyOrbitObjects;
  std::vector<TUniqueId> x354_onScreenOrbitObjects;
  std::vector<TUniqueId> x364_offScreenOrbitObjects;
  bool x374_orbitLockEstablished = false;
  float x378_orbitPreventionTimer = 0.f;
  bool x37c_sidewaysDashing = false;
  float x380_strafeInputAtDash = 0.f;
  float x384_dashTimer = 0.f;
  float x388_dashButtonHoldTime = 0.f;
  bool x38c_doneSidewaysDashing = false;
  u32 x390_orbitSource = 2;
  bool x394_orbitingEnemy = false;
  float x398_dashSpeedMultiplier = 1.5f;
  bool x39c_noStrafeDashBlend = false;
  float x3a0_dashDuration = 0.5f;
  float x3a4_strafeDashBlendDuration = 0.449f;
  EPlayerScanState x3a8_scanState = EPlayerScanState::NotScanning;
  float x3ac_scanningTime = 0.f;
  float x3b0_curScanTime = 0.f;
  TUniqueId x3b4_scanningObject = kInvalidUniqueId;
  EGrappleState x3b8_grappleState = EGrappleState::None;
  float x3bc_grappleSwingTimer = 0.f;
  zeus::CVector3f x3c0_grappleSwingAxis = zeus::skRight;
  float x3cc_ = 0.f;
  float x3d0_ = 0.f;
  float x3d4_ = 0.f;
  float x3d8_grappleJumpTimeout = 0.f;
  bool x3dc_inFreeLook = false;
  bool x3dd_lookButtonHeld = false;
  bool x3de_lookAnalogHeld = false;
  float x3e0_curFreeLookCenteredTime = 0.f;
  float x3e4_freeLookYawAngle = 0.f;
  float x3e8_horizFreeLookAngleVel = 0.f;
  float x3ec_freeLookPitchAngle = 0.f;
  float x3f0_vertFreeLookAngleVel = 0.f;
  TUniqueId x3f4_aimTarget = kInvalidUniqueId;
  zeus::CVector3f x3f8_targetAimPosition = zeus::skZero3f;
  TReservedAverage<zeus::CVector3f, 10> x404_aimTargetAverage;
  zeus::CVector3f x480_assistedTargetAim = zeus::skZero3f;
  float x48c_aimTargetTimer = 0.f;
  std::unique_ptr<CPlayerGun> x490_gun;
  float x494_gunAlpha = 1.f;
  EGunHolsterState x498_gunHolsterState = EGunHolsterState::Drawn;
  float x49c_gunHolsterRemTime;
  std::unique_ptr<CFailsafeTest> x4a0_failsafeTest;
  TReservedAverage<float, 20> x4a4_moveSpeedAvg;
  float x4f8_moveSpeed = 0.f;
  float x4fc_flatMoveSpeed = 0.f;
  zeus::CVector3f x500_lookDir = x34_transform.basis[1];
  zeus::CVector3f x50c_moveDir = x34_transform.basis[1];
  zeus::CVector3f x518_leaveMorphDir = x34_transform.basis[1];
  zeus::CVector3f x524_lastPosForDirCalc = x34_transform.basis[1];
  zeus::CVector3f x530_gunDir = x34_transform.basis[1];
  float x53c_timeMoving = 0.f;
  zeus::CVector3f x540_controlDir = x34_transform.basis[1];
  zeus::CVector3f x54c_controlDirFlat = x34_transform.basis[1];
  bool x558_wasDamaged = false;
  float x55c_damageAmt = 0.f;
  float x560_prevDamageAmt = 0.f;
  zeus::CVector3f x564_damageLocation;
  float x570_immuneTimer = 0.f;
  float x574_morphTime = 0.f;
  float x578_morphDuration = 0.f;
  u32 x57c_ = 0;
  u32 x580_ = 0;
  int x584_ballTransitionAnim = -1;
  float x588_alpha = 1.f;
  float x58c_transitionVel = 0.f;
  bool x590_leaveMorphballAllowed = true;
  TReservedAverage<zeus::CTransform, 4> x594_transisionBeamXfs;
  TReservedAverage<zeus::CTransform, 4> x658_transitionModelXfs;
  TReservedAverage<float, 4> x71c_transitionModelAlphas;
  std::vector<std::unique_ptr<CModelData>> x730_transitionModels;
  float x740_staticTimer = 0.f;
  float x744_staticOutSpeed = 0.f;
  float x748_staticInSpeed = 0.f;
  float x74c_visorStaticAlpha = 1.f;
  float x750_frozenTimeout = 0.f;
  s32 x754_iceBreakJumps = 0;
  float x758_frozenTimeoutBias = 0.f;
  s32 x75c_additionalIceBreakJumps = 0;
  bool x760_controlsFrozen = false;
  float x764_controlsFrozenTimeout = 0.f;
  std::unique_ptr<CMorphBall> x768_morphball;
  std::unique_ptr<CPlayerCameraBob> x76c_cameraBob;
  CSfxHandle x770_damageLoopSfx;
  float x774_samusVoiceTimeout = 0.f;
  CSfxHandle x778_dashSfx;
  // Prime: x77c; Echoes: x119c
  CSfxHandle x77c_samusVoiceSfx;
  int x780_samusVoicePriority = 0;
  // Prime: x784; Echoes: x11a0
  float x784_damageSfxTimer = 0.f;
  // Prime: x788; Echoes: x11a4
  u16 x788_damageLoopSfxId = 0;
  float x78c_footstepSfxTimer = 0.f;
  EFootstepSfx x790_footstepSfxSel = EFootstepSfx::None;
  zeus::CVector3f x794_lastVelocity;
  CVisorSteam x7a0_visorSteam = CVisorSteam(0.f, 0.f, 0.f, CAssetId() /*kInvalidAssetId*/);
  CPlayerState::EPlayerSuit x7cc_transitionSuit = CPlayerState::EPlayerSuit::Invalid;
  CAnimRes x7d0_animRes;
  zeus::CVector3f x7d8_beamScale;
  bool x7e4_ = true;
  u32 x7e8_ = 0;
  CPlayerState::EBeamId x7ec_beam = CPlayerState::EBeamId::Power;
  std::unique_ptr<CModelData> x7f0_ballTransitionBeamModel;
  zeus::CTransform x7f4_gunWorldXf;
  float x824_transitionFilterTimer = 0.f;
  float x828_distanceUnderWater = 0.f;
  bool x82c_inLava = false;
  TUniqueId x82e_ridingPlatform = kInvalidUniqueId;
  TUniqueId x830_playerHint = kInvalidUniqueId;
  u32 x834_playerHintPriority = 1000;
  rstl::reserved_vector<std::pair<u32, TUniqueId>, 32> x838_playerHints;
  rstl::reserved_vector<TUniqueId, 32> x93c_playerHintsToRemove;
  rstl::reserved_vector<TUniqueId, 32> x980_playerHintsToAdd;
  bool x9c4_24_visorChangeRequested : 1 = false;
  bool x9c4_25_showCrosshairs : 1 = false;
  bool x9c4_26_ : 1 = true;
  bool x9c4_27_canEnterMorphBall : 1 = true;
  bool x9c4_28_canLeaveMorphBall : 1 = true;
  bool x9c4_29_spiderBallControlXY : 1 = false;
  bool x9c4_30_controlDirOverride : 1 = false;
  bool x9c4_31_inWaterMovement : 1 = false;
  bool x9c5_24_ : 1 = false;
  bool x9c5_25_splashUpdated : 1 = false;
  bool x9c5_26_ : 1 = false;
  bool x9c5_27_camSubmerged : 1 = false;
  bool x9c5_28_slidingOnWall : 1 = false;
  bool x9c5_29_hitWall : 1 = false;
  bool x9c5_30_selectFluidBallSound : 1 = false;
  bool x9c5_31_stepCameraZBiasDirty : 1 = true;
  bool x9c6_24_extendTargetDistance : 1 = false;
  bool x9c6_25_interpolatingControlDir : 1 = false;
  bool x9c6_26_outOfBallLookAtHint : 1 = false;
  bool x9c6_27_aimingAtProjectile : 1 = false;
  bool x9c6_28_aligningGrappleSwingTurn : 1 = false;
  bool x9c6_29_disableInput : 1 = false;
  bool x9c6_30_newScanScanning : 1 = false;
  bool x9c6_31_overrideRadarRadius : 1 = false;
  bool x9c7_24_noDamageLoopSfx : 1 = false;
  bool x9c7_25_outOfBallLookAtHintActor : 1 = false;
  float x9c8_eyeZBias = 0.f;
  float x9cc_stepCameraZBias = 0.f;
  u32 x9d0_bombJumpCount = 0;
  s32 x9d4_bombJumpCheckDelayFrames = 0;
  zeus::CVector3f x9d8_controlDirOverrideDir = zeus::skForward;
  rstl::reserved_vector<TUniqueId, 5> x9e4_orbitDisableList;

  float x9f4_deathTime = 0.f;
  float x9f8_controlDirInterpTime = 0.f;
  float x9fc_controlDirInterpDur = 0.f;
  TUniqueId xa00_deathPowerBomb = kInvalidUniqueId;
  float xa04_preThinkDt = 0.f;
  CAssetId xa08_steamTextureId;
  CAssetId xa0c_iceTextureId;
  u32 xa10_envDmgCounter = 0;
  float xa14_envDmgCameraShakeTimer = 0.f;
  float xa18_phazonDamageLag = 0.f;
  float xa1c_threatOverride = 0.f;
  float xa20_radarXYRadiusOverride = 1.f;
  float xa24_radarZRadiusOverride = 1.f;
  float xa28_attachedActorStruggle = 0.f;
  int xa2c_damageLoopSfxDelayTicks = 2;
  float xa30_samusExhaustedVoiceTimer = 4.f;

#ifdef PRIME2
  // Echoes
  std::array<short, 4> field_0x130; // TODO: unknown actual size

  // x1268
  bool echoesFlagsA_31_inSafeZone;
  // x1269
  bool echoesFlagsB_31;
  // x126a
  bool echoesFlagsC_24;
  bool echoesFlagsC_30;

  int field_0x12dc;
  // x1314
  CPlayerState* playerState = nullptr;
  // x1318
  CCameraManager* cameraManager;
  // x135c
  float periodForHealSfx;
  // x1360
  float lastDarkAetherDamage;
  // x1364
  float darkAetherTimeForUnk;
  // x136c
  std::pair<TToken<CGenDescription>, TToken<CGenDescription>>* darkAetherElementGenDesc;
  // x1370
  std::unique_ptr<CElementGen> darkAetherDamageParticleA;
  // x1374
  std::unique_ptr<CElementGen> darkAetherDamageParticleB;

  // x1058
  CDamageVulnerability vulnerabilityDarkSuit;
  // x1088
  CDamageVulnerability vulnerabilityLightSuit;
  // x10e8
  CDamageVulnerability vulnerabilityScrewAttack;
#endif

  void StartLandingControlFreeze();
  void EndLandingControlFreeze();
  void ProcessFrozenInput(float dt, CStateManager& mgr);
  bool CheckSubmerged() const;
  void UpdateSubmerged(CStateManager& mgr);
  void InitializeBallTransition();
  float UpdateCameraBob(float dt, CStateManager& mgr);
  float GetAcceleration() const;
  float CalculateOrbitMinDistance(EPlayerOrbitType type) const;

public:
  DEFINE_ENTITY
  CPlayer(TUniqueId uid, const zeus::CTransform& xf, const zeus::CAABox& aabb, CAssetId resId,
          const zeus::CVector3f& playerScale, float mass, float stepUp, float stepDown, float ballRadius,
          const CMaterialList& ml);

  bool IsTransparent() const;
  bool GetControlsFrozen() const { return x760_controlsFrozen; }
  float GetTransitionAlpha(const zeus::CVector3f& camPos, float zNear) const;
  s32 ChooseTransitionToAnimation(float dt, CStateManager& mgr) const;
  void TransitionToMorphBallState(float dt, CStateManager& mgr);
  void TransitionFromMorphBallState(CStateManager& mgr);
  s32 GetNextBallTransitionAnim(float dt, bool& loopOut, CStateManager& mgr);
  void UpdateMorphBallTransition(float dt, CStateManager& mgr);
  void UpdateGunAlpha();
  void UpdatePlayerSounds(float dt);
  void Update(float dt, CStateManager& mgr);
  void PostUpdate(float dt, CStateManager& mgr);
  bool StartSamusVoiceSfx(u16 sfx, float vol, int prio);
  bool IsPlayerDeadEnough() const;
  void AsyncLoadSuit(CStateManager& mgr);
  void LoadAnimationTokens();
  bool HasTransitionBeamModel() const;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f& v1, const zeus::CVector3f& v2,
                                                     const CDamageInfo& info) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  zeus::CVector3f GetHomingPosition(const CStateManager& mgr, float dt) const override;
  zeus::CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;
  void FluidFXThink(EFluidState state, CScriptWater& water, CStateManager& mgr) override;
  zeus::CVector3f GetDamageLocationWR() const { return x564_damageLocation; }
  float GetPrevDamageAmount() const { return x560_prevDamageAmt; }
  float GetDamageAmount() const { return x55c_damageAmt; }
  bool WasDamaged() const { return x558_wasDamaged; }
  void TakeDamage(bool significant, const zeus::CVector3f& location, float dam, EWeaponType type, CStateManager& mgr);
  void Accept(IVisitor& visitor) override;
  CHealthInfo* HealthInfo(CStateManager& mgr) override;
  bool IsUnderBetaMetroidAttack(const CStateManager& mgr) const;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  void DoPreThink(float dt, CStateManager& mgr);
  void DoThink(float dt, CStateManager& mgr);
  void UpdateScanningState(const CFinalInput& input, CStateManager& mgr, float dt);
  bool ValidateScanning(const CFinalInput& input, const CStateManager& mgr) const;
  void FinishNewScan(CStateManager& mgr);
  void SetScanningState(EPlayerScanState state, CStateManager& mgr);
  void SetSpawnedMorphBallState(EPlayerMorphBallState state, CStateManager& mgr);
  bool GetExplorationMode() const;
  bool GetCombatMode() const;
  void RenderGun(const CStateManager& mgr, const zeus::CVector3f& pos) const;
  void Render(CStateManager& mgr) override;
  void RenderReflectedPlayer(CStateManager& mgr);
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void CalculateRenderBounds() override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void ComputeFreeLook(const CFinalInput& input);
  void UpdateFreeLookState(const CFinalInput& input, float dt, CStateManager& mgr);
  void UpdateFreeLook(float dt);
  float GetMaximumPlayerPositiveVerticalVelocity(CStateManager& mgr) const;
  void ProcessInput(const CFinalInput& input, CStateManager& mgr);
  bool ShouldSampleFailsafe(CStateManager& mgr) const;
  void CalculateLeaveMorphBallDirection(const CFinalInput& input);
  void CalculatePlayerControlDirection(CStateManager& mgr);
  void CalculatePlayerMovementDirection(float dt);
  void UnFreeze(CStateManager& stateMgr);
  void Freeze(CStateManager& stateMgr, CAssetId steamTxtr, u16 sfx, CAssetId iceTxtr);
  bool GetFrozenState() const;
  void UpdateFrozenState(const CFinalInput& input, CStateManager& mgr);
  void UpdateStepCameraZBias(float dt);
  void UpdateWaterSurfaceCameraBias(CStateManager& mgr);
  void UpdateEnvironmentDamageCameraShake(float dt, CStateManager& mgr);
  void UpdatePhazonDamage(float dt, CStateManager& mgr);
  void ResetPlayerHintState();
  bool SetAreaPlayerHint(const CScriptPlayerHint& hint, CStateManager& mgr);
  void AddToPlayerHintRemoveList(TUniqueId id, CStateManager& mgr);
  void AddToPlayerHintAddList(TUniqueId id, CStateManager& mgr);
  void DeactivatePlayerHint(TUniqueId id, CStateManager& mgr);
  void UpdatePlayerHints(CStateManager& mgr);
  void UpdateBombJumpStuff();
  void UpdateTransitionFilter(float dt, CStateManager& mgr);
  void ResetControlDirectionInterpolation();
  void SetControlDirectionInterpolation(float time);
  void UpdatePlayerControlDirection(float dt, CStateManager& mgr);
  void Think(float dt, CStateManager& mgr) override;
  void PreThink(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void SetVisorSteam(float targetAlpha, float alphaInDur, float alphaOutDir, CAssetId txtr, bool affectsThermal);
  void UpdateFootstepSounds(const CFinalInput& input, CStateManager& mgr, float dt);
  u16 GetMaterialSoundUnderPlayer(const CStateManager& mgr, const u16* idList, size_t length, u16 defId) const;
  static u16 SfxIdFromMaterial(const CMaterialList& mat, const u16* idList, size_t tableLen, u16 defId);
  void UpdateCrosshairsState(const CFinalInput& input);
  void UpdateVisorTransition(float, CStateManager& mgr);
  void UpdateVisorState(const CFinalInput& input, float dt, CStateManager& mgr);
  void UpdateGunState(const CFinalInput& input, CStateManager& mgr);
  void ResetGun(CStateManager& mgr);
  void UpdateArmAndGunTransforms(float dt, CStateManager& mgr);
  void ForceGunOrientation(const zeus::CTransform&, CStateManager& mgr);
  void UpdateCameraState(CStateManager& mgr);
  void UpdateDebugCamera(CStateManager& mgr);
  void UpdateCameraTimers(float dt, const CFinalInput& input);
  void UpdateMorphBallState(float dt, const CFinalInput&, CStateManager& mgr);
  CFirstPersonCamera& GetFirstPersonCamera(CStateManager& mgr);
  void UpdateGunTransform(const zeus::CVector3f& gunPos, CStateManager& mgr);
  void UpdateAssistedAiming(const zeus::CTransform& xf, const CStateManager& mgr);
  void UpdateAimTargetPrediction(const zeus::CTransform& xf, const CStateManager& mgr);
  void ResetAimTargetPrediction(TUniqueId target);
  void DrawGun(CStateManager& mgr);
  void HolsterGun(CStateManager& mgr);
  EPlayerCameraState GetCameraState() const { return x2f4_cameraState; }
  EPlayerMorphBallState GetMorphballTransitionState() const { return x2f8_morphBallState; }
  EGunHolsterState GetGunHolsterState() const { return x498_gunHolsterState; }
  EPlayerMovementState GetPlayerMovementState() const { return x258_movementState; }
  bool IsMorphBallTransitioning() const;
  void UpdateGrappleArmTransform(const zeus::CVector3f& offset, CStateManager& mgr, float dt);
  float GetGravity() const;
  void ApplyGrappleForces(const CFinalInput& input, CStateManager& mgr, float dt);
  bool ValidateFPPosition(const zeus::CVector3f& pos, const CStateManager& mgr) const;
  void UpdateGrappleState(const CFinalInput& input, CStateManager& mgr);
  void ApplyGrappleJump(CStateManager& mgr);
  void BeginGrapple(zeus::CVector3f& vec, CStateManager& mgr);
  void BreakGrapple(EPlayerOrbitRequest req, CStateManager& mgr);
  void SetOrbitRequest(EPlayerOrbitRequest req, CStateManager& mgr);
  void SetOrbitRequestForTarget(TUniqueId id, EPlayerOrbitRequest req, CStateManager& mgr);
  bool InGrappleJumpCooldown() const;
  void PreventFallingCameraPitch();
  void OrbitCarcass(CStateManager& mgr);
  void OrbitPoint(EPlayerOrbitType type, CStateManager& mgr);
  zeus::CVector3f GetHUDOrbitTargetPosition() const;
  void SetOrbitState(EPlayerOrbitState state, CStateManager& mgr);
  void SetOrbitTargetId(TUniqueId id, CStateManager& mgr);
  void UpdateOrbitPosition(float dist, CStateManager& mgr);
  void UpdateOrbitZPosition();
  void UpdateOrbitFixedPosition();
  void SetOrbitPosition(float dist, CStateManager& mgr);
  void UpdateAimTarget(CStateManager& mgr);
  void UpdateAimTargetTimer(float dt);
  bool ValidateAimTargetId(TUniqueId uid, CStateManager& mgr);
  bool ValidateObjectForMode(TUniqueId uid, CStateManager& mgr) const;
  TUniqueId FindAimTargetId(CStateManager& mgr) const;
  TUniqueId GetAimTarget() const { return x3f4_aimTarget; }
  TUniqueId CheckEnemiesAgainstOrbitZone(const rstl::reserved_vector<TUniqueId, 1024>& list, EPlayerZoneInfo info,
                                         EPlayerZoneType zone, CStateManager& mgr) const;
  TUniqueId FindOrbitTargetId(CStateManager& mgr) const;
  void UpdateOrbitableObjects(CStateManager& mgr);
  TUniqueId FindBestOrbitableObject(const std::vector<TUniqueId>& ids, EPlayerZoneInfo info, CStateManager& mgr) const;
  void FindOrbitableObjects(const rstl::reserved_vector<TUniqueId, 1024>& nearObjects, std::vector<TUniqueId>& listOut,
                            EPlayerZoneInfo zone, EPlayerZoneType type, CStateManager& mgr, bool onScreenTest) const;
  bool WithinOrbitScreenBox(const zeus::CVector3f& screenCoords, EPlayerZoneInfo zone, EPlayerZoneType type) const;
  bool WithinOrbitScreenEllipse(const zeus::CVector3f& screenCoords, EPlayerZoneInfo zone) const;
  bool CheckOrbitDisableSourceList(CStateManager& mgr);
  bool CheckOrbitDisableSourceList() const { return !x9e4_orbitDisableList.empty(); }
  void RemoveOrbitDisableSource(TUniqueId uid);
  void AddOrbitDisableSource(CStateManager& mgr, TUniqueId addId);
  void UpdateOrbitPreventionTimer(float dt);
  void UpdateOrbitModeTimer(float dt);
  void UpdateOrbitZone(CStateManager& mgr);
  void UpdateOrbitInput(const CFinalInput& input, CStateManager& mgr);
  void ActivateOrbitSource(CStateManager& mgr);
  void UpdateOrbitSelection(const CFinalInput& input, CStateManager& mgr);
  void UpdateOrbitOrientation(CStateManager& mgr);
  void UpdateOrbitTarget(CStateManager& mgr);
  float GetOrbitMaxLockDistance(CStateManager& mgr) const;
  float GetOrbitMaxTargetDistance(CStateManager& mgr) const;
  EOrbitValidationResult ValidateOrbitTargetId(TUniqueId uid, CStateManager& mgr) const;
  EOrbitValidationResult ValidateCurrentOrbitTargetId(CStateManager& mgr);
  bool ValidateOrbitTargetIdAndPointer(TUniqueId uid, CStateManager& mgr) const;
  zeus::CVector3f GetBallPosition() const;
  zeus::CVector3f GetEyePosition() const;
  float GetEyeHeight() const;
  float GetUnbiasedEyeHeight() const;
  float GetStepUpHeight() const override;
  float GetStepDownHeight() const override;
  void Teleport(const zeus::CTransform& xf, CStateManager& mgr, bool resetBallCam);
  void BombJump(const zeus::CVector3f& pos, CStateManager& mgr);
  zeus::CTransform CreateTransformFromMovementDirection() const;
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  const CCollidableSphere* GetCollidableSphere() const;
  zeus::CTransform GetPrimitiveTransform() const override;
  void CollidedWith(TUniqueId id, const CCollisionInfoList& list, CStateManager& mgr) override;
  float GetBallMaxVelocity() const;
  float GetActualBallMaxVelocity(float dt) const;
  float GetActualFirstPersonMaxVelocity(float dt) const;
  void SetMoveState(EPlayerMovementState newState, CStateManager& mgr);
  float JumpInput(const CFinalInput& input, CStateManager& mgr);
  float TurnInput(const CFinalInput& input) const;
  float StrafeInput(const CFinalInput& input) const;
  float ForwardInput(const CFinalInput& input, float turnInput) const;
  zeus::CVector3f CalculateLeftStickEdgePosition(float strafeInput, float forwardInput) const;
  bool SidewaysDashAllowed(float strafeInput, float forwardInput, const CFinalInput& input, CStateManager& mgr) const;
  void FinishSidewaysDash();
  void ComputeDash(const CFinalInput& input, float dt, CStateManager& mgr);
  void ComputeMovement(const CFinalInput& input, CStateManager& mgr, float dt);
  float GetWeight() const override;
  zeus::CVector3f GetDampedClampedVelocityWR() const;
  const CVisorSteam& GetVisorSteam() const { return x7a0_visorSteam; }
  float GetVisorStaticAlpha() const { return x74c_visorStaticAlpha; }
  float GetGunAlpha() const { return x494_gunAlpha; }
  const CScriptWater* GetVisorRunoffEffect(const CStateManager& mgr) const;
  void SetMorphBallState(EPlayerMorphBallState state, CStateManager& mgr);
  bool CanLeaveMorphBallState(CStateManager& mgr, zeus::CVector3f& pos) const;
  void SetHudDisable(float staticTimer, float outSpeed, float inSpeed);
  void SetIntoBallReadyAnimation(CStateManager& mgr);
  void LeaveMorphBallState(CStateManager& mgr);
  bool CanEnterMorphBallState(CStateManager& mgr, float f1) const;
  void EnterMorphBallState(CStateManager& mgr);
  void ActivateMorphBallCamera(CStateManager& mgr);
  void UpdateCinematicState(CStateManager& mgr);
  void SetCameraState(EPlayerCameraState camState, CStateManager& stateMgr);
  bool IsEnergyLow(const CStateManager& mgr) const;
  EPlayerOrbitState GetOrbitState() const { return x304_orbitState; }
  EPlayerScanState GetScanningState() const { return x3a8_scanState; }
  float GetScanningTime() const { return x3ac_scanningTime; }
  TUniqueId GetOrbitTargetId() const { return x310_orbitTargetId; }
  TUniqueId GetOrbitNextTargetId() const { return x33c_orbitNextTargetId; }
  TUniqueId GetScanningObjectId() const { return x3b4_scanningObject; }
  EGrappleState GetGrappleState() const { return x3b8_grappleState; }
  bool IsNewScanScanning() const { return x9c6_30_newScanScanning; }
  float GetThreatOverride() const { return xa1c_threatOverride; }
  bool IsOverrideRadarRadius() const { return x9c6_31_overrideRadarRadius; }
  void SetIsOverrideRadarRadius(bool override) { x9c6_31_overrideRadarRadius = override; }
  float GetRadarXYRadiusOverride() const { return xa20_radarXYRadiusOverride; }
  void SetRadarXYRadiusOverride(float xyOverride) { xa20_radarXYRadiusOverride = xyOverride; }
  float GetRadarZRadiusOverride() const { return xa24_radarZRadiusOverride; }
  void SetRadarZRadiusOverride(float zOverride) { xa24_radarZRadiusOverride = zOverride; }
  bool ObjectInScanningRange(TUniqueId id, const CStateManager& mgr) const;
  float GetMorphTime() const { return x574_morphTime; }
  float GetMorphDuration() const { return x578_morphDuration; }
  float GetMorphFactor() const {
    if (0.f != x578_morphDuration)
      return zeus::clamp(0.f, x574_morphTime / x578_morphDuration, 1.f);
    return 0.f;
  }
  bool IsInFreeLook() const { return x3dc_inFreeLook; }
  bool GetFreeLookStickState() const { return x3de_lookAnalogHeld; }
  CPlayerGun* GetPlayerGun() const { return x490_gun.get(); }
  CMorphBall* GetMorphBall() const { return x768_morphball.get(); }
  CPlayerCameraBob* GetCameraBob() const { return x76c_cameraBob.get(); }
  float GetStaticTimer() const { return x740_staticTimer; }
  float GetDeathTime() const { return x9f4_deathTime; }
  const CPlayerEnergyDrain& GetEnergyDrain() const { return x274_energyDrain; }
  CPlayerEnergyDrain& GetEnergyDrain() { return x274_energyDrain; }
  EPlayerZoneInfo GetOrbitZone() const { return x330_orbitZoneMode; }
  EPlayerZoneType GetOrbitType() const { return x334_orbitType; }
  const zeus::CTransform& GetFirstPersonCameraTransform(const CStateManager& mgr) const;
  const std::vector<TUniqueId>& GetNearbyOrbitObjects() const { return x344_nearbyOrbitObjects; }
  const std::vector<TUniqueId>& GetOnScreenOrbitObjects() const { return x354_onScreenOrbitObjects; }
  const std::vector<TUniqueId>& GetOffScreenOrbitObjects() const { return x364_offScreenOrbitObjects; }
  void SetPlayerHitWallDuringMove();
  float GetTimeSinceJump() const { return x2a8_timeSinceJump; }
  void ResetTimeSinceJump() { x2a8_timeSinceJump = 1000.f; }
  ESurfaceRestraints GetCurrentSurfaceRestraint() const { return x2ac_surfaceRestraint; }
  ESurfaceRestraints GetSurfaceRestraint() const {
    return x2b0_outOfWaterTicks == 2 ? GetCurrentSurfaceRestraint() : ESurfaceRestraints::Water;
  }
  void DecrementEnvironmentDamage();
  void IncrementEnvironmentDamage();
  void ApplySubmergedPitchBend(CSfxHandle& sfx);
  void DetachActorFromPlayer();
  bool AttachActorToPlayer(TUniqueId id, bool disableGun);
  TUniqueId GetAttachedActor() const { return x26c_attachedActor; }
  float GetAttachedActorStruggle() const { return xa28_attachedActorStruggle; }
  void SetFrozenTimeoutBias(float bias) { x758_frozenTimeoutBias = bias; }
  float GetDistanceUnderWater() const { return x828_distanceUnderWater; }
  TUniqueId GetRidingPlatformId() const { return x82e_ridingPlatform; }
  const zeus::CVector3f& GetLastVelocity() const { return x794_lastVelocity; }
  const zeus::CVector3f& GetMoveDir() const { return x50c_moveDir; }
  const zeus::CVector3f& GetLeaveMorphDir() const { return x518_leaveMorphDir; }
  u32 GetBombJumpCount() const { return x9d0_bombJumpCount; }
  float GetMoveSpeed() const { return x4f8_moveSpeed; }
  EPlayerOrbitRequest GetOrbitRequest() const { return x30c_orbitRequest; }
  bool IsShowingCrosshairs() const { return x9c4_25_showCrosshairs; }
  bool IsSidewaysDashing() const { return x37c_sidewaysDashing; }
  void SetLeaveMorphBallAllowed(bool b) { x590_leaveMorphballAllowed = b; }
  const zeus::CVector3f& GetOrbitPoint() const { return x314_orbitPoint; }
  float GetAverageSpeed() const;
  bool IsInWaterMovement() const { return x9c4_31_inWaterMovement; }
  void SetNoDamageLoopSfx(bool val) { x9c7_24_noDamageLoopSfx = val; }
  void SetAccelerationChangeTimer(float time) { x2d4_accelerationChangeTimer = time; }

#ifdef PRIME2
  // Echoes
  class EchoesTweakPlayer : public DataSpec::ITweakPlayer {
  public:
    float GetDarkWorld_0x0() { return 0.0; }
    float GetDarkWorld_0x4() { return 0.0; }
    float GetDarkWorld_0x1c() { return 0.0; }
    float GetDarkWorldDarkSuitReduction() { return 0.0; }
    CDamageInfo GetDarkWorldDamageInfo() { return CDamageInfo(); }
  };

  EchoesTweakPlayer* GetTweakPlayer() const;
  void UpdateDarkAetherDamage(float dt, CStateManager& mgr);
  short Get_0x1308_Indexed(int param) {
    return field_0x130[param];
  }
#endif
};
} // namespace metaforce
