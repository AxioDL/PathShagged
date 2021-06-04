#pragma once

#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Runtime/CBasics.hpp"
#include "Runtime/CRandom16.hpp"
#include "Runtime/CSortedLists.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/Camera/CCameraShakeData.hpp"
#include "Runtime/GameObjectLists.hpp"
#include "Runtime/Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Runtime/Input/CFinalInput.hpp"
#include "Runtime/Input/CRumbleManager.hpp"
#include "Runtime/Weapon/CWeaponMgr.hpp"
#include "Runtime/World/CActorModelParticles.hpp"
#include "Runtime/World/CAi.hpp"
#include "Runtime/World/CEnvFxManager.hpp"
#include "Runtime/World/CFluidPlaneManager.hpp"
#include "Runtime/World/ScriptLoader.hpp"
#include "Runtime/World/ScriptObjectSupport.hpp"
#include "Runtime/World/CScriptMazeNode.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector2i.hpp>

namespace metaforce {
class CActor;
class CActorModelParticles;
class CDamageInfo;
class CEnvFxManager;
class CFluidPlaneManager;
class CLight;
class CMapWorldInfo;
class CMaterialFilter;
class CObjectList;
class CPlayer;
class CPlayerState;
class CProjectedShadow;
class CScriptMailbox;
class CRumbleManager;
class CSortedListManager;
class CTexture;
class CWorld;
class CScriptLayerManager;
class CWorldTransManager;

struct CFinalInput;

namespace MP1 {
class CMFGameLoader;
}

struct SScriptObjectStream {
  // CEntity* x0_obj;
  EScriptObjectType x0_type;
  u32 x4_position;
  u32 x8_length;
};

struct SOnScreenTex {
  CAssetId x0_id;
  zeus::CVector2i x4_origin;
  zeus::CVector2i xc_extent;
};

enum class EStateManagerTransition { InGame, MapScreen, PauseGame, LogBook, SaveGame, MessageScreen };

enum class EThermalDrawFlag { Hot, Cold, Bypass };

class CStateManager {
  friend class MP1::CMFGameLoader;

public:
  enum class EGameState { Running, SoftPaused, Paused };

private:
  s16 x0_nextFreeIndex = 0;
  std::array<u16, 1024> x8_idArr{};

  /*
  std::unique_ptr<CObjectList> x80c_allObjs;
  std::unique_ptr<CActorList> x814_actorObjs;
  std::unique_ptr<CPhysicsActorList> x81c_physActorObjs;
  std::unique_ptr<CGameCameraList> x824_cameraObjs;
  std::unique_ptr<CGameLightList> x82c_lightObjs;
  std::unique_ptr<CListeningAiList> x834_listenAiObjs;
  std::unique_ptr<CAiWaypointList> x83c_aiWaypointObjs;
  std::unique_ptr<CPlatformAndDoorList> x844_platformAndDoorObjs;
   */
  std::array<std::unique_ptr<CObjectList>, 8> x808_objLists{
      std::make_unique<CObjectList>(EGameObjectList::All),
      std::make_unique<CActorList>(),
      std::make_unique<CPhysicsActorList>(),
      std::make_unique<CGameCameraList>(),
      std::make_unique<CGameLightList>(),
      std::make_unique<CListeningAiList>(),
      std::make_unique<CAiWaypointList>(),
      std::make_unique<CPlatformAndDoorList>(),
  };

  std::unique_ptr<CPlayer> x84c_player;
  std::unique_ptr<CWorld> x850_world;

  /* Used to be a list of 32-element reserved_vectors */
  std::vector<TUniqueId> x854_objectGraveyard;

  struct CStateManagerContainer {
    CCameraManager x0_cameraManager;
    CSortedListManager x3c0_sortedListManager;
    CWeaponMgr xe3d8_weaponManager;
    CFluidPlaneManager xe3ec_fluidPlaneManager;
    CEnvFxManager xe510_envFxManager;
    CActorModelParticles xf168_actorModelParticles;
    CRumbleManager xf250_rumbleManager;
    u32 xf344_ = 0;
    rstl::reserved_vector<TUniqueId, 20> xf370_;
    rstl::reserved_vector<TUniqueId, 20> xf39c_renderLast;
  };
  std::unique_ptr<CStateManagerContainer> x86c_stateManagerContainer;
  CCameraManager* x870_cameraManager = nullptr;
  CSortedListManager* x874_sortedListManager = nullptr;
  CWeaponMgr* x878_weaponManager = nullptr;
  CFluidPlaneManager* x87c_fluidPlaneManager = nullptr;
  CEnvFxManager* x880_envFxManager = nullptr;
  CActorModelParticles* x884_actorModelParticles = nullptr;
  CRumbleManager* x88c_rumbleManager = nullptr;

  std::multimap<TEditorId, TUniqueId> x890_scriptIdMap;
  std::map<TEditorId, SScriptObjectStream> x8a4_loadedScriptObjects;

  std::shared_ptr<CPlayerState> x8b8_playerState;
  std::shared_ptr<CScriptMailbox> x8bc_mailbox;
  std::shared_ptr<CMapWorldInfo> x8c0_mapWorldInfo;
  std::shared_ptr<CWorldTransManager> x8c4_worldTransManager;
  std::shared_ptr<CScriptLayerManager> x8c8_worldLayerState;

  TAreaId x8cc_nextAreaId = 0;
  TAreaId x8d0_prevAreaId = kInvalidAreaId;
  // u32 x8d0_extFrameIdx = 0;
  u32 x8d4_inputFrameIdx = 0;
  u32 x8d8_updateFrameIdx = 0;
  u32 x8dc_objectDrawToken = 0;

  std::vector<CLight> x8e0_dynamicLights;

  TLockedToken<CTexture> x8f0_shadowTex; /* DefaultShadow in MiscData */
  CRandom16 x8fc_random;
  CRandom16* x900_activeRandom = nullptr;
  EGameState x904_gameState = EGameState::Running;
  u32 x908_loaderCount = 0;
  std::array<FScriptLoader, size_t(EScriptObjectType::ScriptObjectTypeMAX)> x90c_loaderFuncs{};

  bool xab0_worldLoaded = false;

  enum class EInitPhase { LoadWorld, LoadFirstArea, Done } xb3c_initPhase = EInitPhase::LoadWorld;

  std::set<std::string> xb40_uniqueInstanceNames;

  CFinalInput xb54_finalInput;

  static constexpr size_t numCameraPasses = 9;
  std::array<CCameraFilterPassPoly, numCameraPasses> xb84_camFilterPasses; // size: 0x2c
  std::array<CCameraBlurPass, numCameraPasses> xd14_camBlurPasses;         // size: 0x34

  s32 xeec_hintIdx = -1;
  u32 xef0_hintPeriods = 0;
  SOnScreenTex xef4_pendingScreenTex;
  CAssetId xf08_pauseHudMessage;
  float xf0c_escapeTimer = 0.f;
  float xf10_escapeTotalTime = 0.f;
  float xf14_curTimeMod900 = 0.f;
  TUniqueId xf18_bossId = kInvalidUniqueId;
  float xf1c_totalBossEnergy = 0.f;
  u32 xf20_bossStringIdx = 0;
  float xf24_thermColdScale1 = 0.f;
  float xf28_thermColdScale2 = 0.f;
  zeus::CVector2f xf2c_viewportScale = {1.f, 1.f};
  EThermalDrawFlag xf34_thermalFlag = EThermalDrawFlag::Bypass;
  TUniqueId xf38_skipCineSpecialFunc = kInvalidUniqueId;
  std::list<TUniqueId> xf3c_activeFlickerBats;
  std::list<TUniqueId> xf54_activeParasites;
  TUniqueId xf6c_playerActorHead = kInvalidUniqueId;
  std::unique_ptr<CMazeState> xf70_currentMaze;

  TUniqueId xf74_lastTrigger = kInvalidUniqueId;
  TUniqueId xf76_lastRelay = kInvalidUniqueId;

  float xf78_hudMessageTime = 0.f;
  CProjectedShadow* xf7c_projectedShadow = nullptr;
  u32 xf80_hudMessageFrameCount = 0;
  s32 xf84_ = -1;
  CAssetId xf88_;
  float xf8c_ = 0.f;
  EStateManagerTransition xf90_deferredTransition = EStateManagerTransition::InGame;
  bool xf94_24_readyToRender : 1 = false;
  bool xf94_25_quitGame : 1 = false;
  bool xf94_26_generatingObject : 1 = false;
  bool xf94_27_inMapScreen : 1 = false;
  bool xf94_28_inSaveUI : 1 = false;
  bool xf94_29_cinematicPause : 1 = false;
  bool xf94_30_fullThreat : 1 = false;

  CColoredQuadFilter m_deathWhiteout{EFilterType::Add};
  CColoredQuadFilter m_escapeWhiteout{EFilterType::Add};
  bool m_warping = false;
  std::map<TEditorId, std::set<SConnection>> m_incomingConnections;

  void UpdateThermalVisor();
  static void RendererDrawCallback(void*, void*, int);

public:
  CStateManager(const std::weak_ptr<CScriptMailbox>&, const std::weak_ptr<CMapWorldInfo>&,
                const std::weak_ptr<CPlayerState>&, const std::weak_ptr<CWorldTransManager>&,
                const std::weak_ptr<CScriptLayerManager>&);
  ~CStateManager();

  u32 GetInputFrameIdx() const { return x8d4_inputFrameIdx; }
  bool RenderLast(TUniqueId);
  void AddDrawableActorPlane(CActor& actor, const zeus::CPlane&, const zeus::CAABox& aabb) const;
  void AddDrawableActor(CActor& actor, const zeus::CVector3f& vec, const zeus::CAABox& aabb) const;
  bool SpecialSkipCinematic();
  TAreaId GetVisAreaId() const;
  s32 GetWeaponIdCount(TUniqueId, EWeaponType) const;
  void RemoveWeaponId(TUniqueId, EWeaponType);
  void AddWeaponId(TUniqueId, EWeaponType);
  void UpdateEscapeSequenceTimer(float);
  float GetEscapeSequenceTimer() const { return xf0c_escapeTimer; }
  void ResetEscapeSequenceTimer(float);
  void SetupParticleHook(const CActor& actor) const;
  void MurderScriptInstanceNames();
  std::string HashInstanceName(CInputStream& in);
  void SetActorAreaId(CActor& actor, TAreaId);
  void TouchSky() const;
  void TouchPlayerActor();
  void DrawSpaceWarp(const zeus::CVector3f&, float) const;
  void DrawReflection(const zeus::CVector3f&);
  static void ReflectionDrawer(void*, const zeus::CVector3f&);
  void CacheReflection();
  bool CanCreateProjectile(TUniqueId, EWeaponType, int) const;
  const std::vector<CLight>& GetDynamicLightList() const { return x8e0_dynamicLights; }
  void BuildDynamicLightListForWorld();
  void DrawDebugStuff() const;
  void RenderCamerasAndAreaLights();
  void DrawE3DeathEffect();
  void DrawAdditionalFilters();
  zeus::CFrustum SetupDrawFrustum(const SViewport& vp) const;
  zeus::CFrustum SetupViewForDraw(const SViewport& vp) const;
  zeus::CFrustum SetupViewForCubeFaceDraw(const zeus::CVector3f& pos, int face) const;
  void ResetViewAfterDraw(const SViewport& backupViewport, const zeus::CTransform& backupViewMatrix) const;
  void DrawWorld();
  void DrawActorCubeFaces(CActor& actor, int& cubeInst) const;
  void DrawWorldCubeFaces() const;
  void SetupFogForArea3XRange(TAreaId area) const;
  void SetupFogForArea(TAreaId area) const;
  void SetupFogForAreaNonCurrent(TAreaId area) const;
  void SetupFogForArea3XRange(const CGameArea& area) const;
  void SetupFogForArea(const CGameArea& area) const;
  void SetupFogForAreaNonCurrent(const CGameArea& area) const;
  bool SetupFogForDraw() const;
  void PreRender();
  void GetCharacterRenderMaskAndTarget(bool thawed, int& mask, int& target) const;
  bool GetVisSetForArea(TAreaId, TAreaId, CPVSVisSet& setOut) const;
  void RecursiveDrawTree(TUniqueId);
  void SendScriptMsg(CEntity* dest, TUniqueId src, EScriptObjectMessage msg);
  void SendScriptMsg(TUniqueId dest, TUniqueId src, EScriptObjectMessage msg);
  void SendScriptMsg(TUniqueId src, TEditorId dest, EScriptObjectMessage msg, EScriptObjectState state);
  void SendScriptMsgAlways(TUniqueId dest, TUniqueId src, EScriptObjectMessage);
  void FreeScriptObjects(TAreaId);
  void FreeScriptObject(TUniqueId);
  std::pair<const SScriptObjectStream*, TEditorId> GetBuildForScript(TEditorId) const;
  TEditorId GetEditorIdForUniqueId(TUniqueId) const;
  TUniqueId GetIdForScript(TEditorId) const;
  std::pair<std::multimap<TEditorId, TUniqueId>::const_iterator, std::multimap<TEditorId, TUniqueId>::const_iterator>
      GetIdListForScript(TEditorId) const;
  std::multimap<TEditorId, TUniqueId>::const_iterator GetIdListEnd() const { return x890_scriptIdMap.cend(); }
  void LoadScriptObjects(TAreaId, CInputStream& in, std::vector<TEditorId>& idsOut);
  void InitializeScriptObjects(const std::vector<TEditorId>& objIds);
  std::pair<TEditorId, TUniqueId> LoadScriptObject(TAreaId, EScriptObjectType, u32, CInputStream& in);
  std::pair<TEditorId, TUniqueId> GenerateObject(TEditorId);
  void InitScriptObjects(const std::vector<TEditorId>& ids);
  void InformListeners(const zeus::CVector3f&, EListenNoiseType);
  void ApplyKnockBack(CActor& actor, const CDamageInfo& info, const CDamageVulnerability&, const zeus::CVector3f&,
                      float);
  void KnockBackPlayer(CPlayer& player, const zeus::CVector3f& pos, float power, float resistance);
  void ApplyDamageToWorld(TUniqueId, const CActor&, const zeus::CVector3f&, const CDamageInfo& info,
                          const CMaterialFilter&);
  void ProcessRadiusDamage(const CActor&, CActor&, TUniqueId senderId, const CDamageInfo& info, const CMaterialFilter&);
  void ApplyRadiusDamage(const CActor&, const zeus::CVector3f&, CActor&, const CDamageInfo& info);
  bool TestRayDamage(const zeus::CVector3f& pos, const CActor& damagee,
                     const rstl::reserved_vector<TUniqueId, 1024>& nearList) const;
  bool RayCollideWorld(const zeus::CVector3f& start, const zeus::CVector3f& end, const CMaterialFilter& filter,
                       const CActor* damagee) const;
  bool RayCollideWorld(const zeus::CVector3f& start, const zeus::CVector3f& end,
                       const rstl::reserved_vector<TUniqueId, 1024>& nearList, const CMaterialFilter& filter,
                       const CActor* damagee) const;
  bool RayCollideWorldInternal(const zeus::CVector3f& start, const zeus::CVector3f& end, const CMaterialFilter& filter,
                               const rstl::reserved_vector<TUniqueId, 1024>& nearList, const CActor* damagee) const;
  bool MultiRayCollideWorld(const zeus::CMRay& ray, const CMaterialFilter& filter) const;
  void TestBombHittingWater(const CActor& damager, const zeus::CVector3f& pos, CActor& damagee);
  bool ApplyLocalDamage(const zeus::CVector3f&, const zeus::CVector3f&, CActor&, float, const CWeaponMode&);
  bool ApplyDamage(TUniqueId damagerId, TUniqueId damageeId, TUniqueId radiusSender, const CDamageInfo& info,
                   const CMaterialFilter& filter, const zeus::CVector3f& knockbackVec);
  void UpdateAreaSounds();
  void FrameEnd();
  void ProcessPlayerInput();
  void SetGameState(EGameState state);
  EGameState GetGameState() const { return x904_gameState; }
  void ProcessInput(const CFinalInput& input);
  void UpdateGraphicsTiming(float dt);
  void Update(float dt);
  void UpdateGameState();
  void UpdateHintState(float dt);
  void PreThinkObjects(float dt);
  void MovePlatforms(float dt);
  void MoveActors(float dt);
  void CrossTouchActors();
  void Think(float dt);
  void PostUpdatePlayer(float dt);
  void ShowPausedHUDMemo(CAssetId strg, float time);
  void ClearGraveyard();
  void FrameBegin(s32 frameCount);
  void InitializeState(CAssetId mlvlId, TAreaId aid, CAssetId mreaId);
  void CreateStandardGameObjects();
  const std::unique_ptr<CObjectList>& GetObjectList() const { return x808_objLists[0]; }
  CObjectList* ObjectListById(EGameObjectList type);
  const CObjectList* GetObjectListById(EGameObjectList type) const;
  void RemoveObject(TUniqueId);
  void UpdateRoomAcoustics(TAreaId);
  TAreaId GetNextAreaId() const { return x8cc_nextAreaId; }
  void SetCurrentAreaId(TAreaId);
  CEntity* ObjectById(TUniqueId uid) const { return GetAllObjectList().GetObjectById(uid); }
  const CEntity* GetObjectById(TUniqueId uid) const { return GetAllObjectList().GetObjectById(uid); }
  void AreaUnloaded(TAreaId);
  void PrepareAreaUnload(TAreaId);
  void AreaLoaded(TAreaId);
  void BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& listOut, const zeus::CVector3f&, const zeus::CVector3f&,
                     float, const CMaterialFilter&, const CActor*) const;
  void BuildColliderList(rstl::reserved_vector<TUniqueId, 1024>& listOut, const CActor&, const zeus::CAABox&) const;
  void BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& listOut, const zeus::CAABox&, const CMaterialFilter&,
                     const CActor*) const;
  void UpdateActorInSortedLists(CActor&);
  void UpdateSortedLists();
  std::optional<zeus::CAABox> CalculateObjectBounds(const CActor&);
  void AddObject(CEntity&);
  void AddObject(CEntity*);
  CRayCastResult RayStaticIntersection(const zeus::CVector3f& pos, const zeus::CVector3f& dir, float length,
                                       const CMaterialFilter& filter) const;
  CRayCastResult RayWorldIntersection(TUniqueId& idOut, const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                      float length, const CMaterialFilter& filter,
                                      const rstl::reserved_vector<TUniqueId, 1024>& list) const;
  void UpdateObjectInLists(CEntity&);
  TUniqueId AllocateUniqueId();
  void DeferStateTransition(EStateManagerTransition t);
  EStateManagerTransition GetDeferredStateTransition() const { return xf90_deferredTransition; }
  bool CanShowMapScreen() const;
  TUniqueId GetSkipCinematicSpecialFunction() const { return xf38_skipCineSpecialFunc; }
  void SetSkipCinematicSpecialFunction(TUniqueId id) { xf38_skipCineSpecialFunc = id; }
  float GetHUDMessageTime() const { return xf78_hudMessageTime; }
  u32 GetHUDMessageFrameCount() const { return xf80_hudMessageFrameCount; }
  CAssetId GetPauseHUDMessage() const { return xf08_pauseHudMessage; }
  void IncrementHUDMessageFrameCounter() { ++xf80_hudMessageFrameCount; }
  bool ShouldQuitGame() const { return xf94_25_quitGame; }
  void SetShouldQuitGame(bool should) { xf94_25_quitGame = should; }
  void SetInSaveUI(bool b) { xf94_28_inSaveUI = b; }
  bool GetInSaveUI() const { return xf94_28_inSaveUI; }
  void SetInMapScreen(bool b) { xf94_27_inMapScreen = b; }
  bool GetInMapScreen() const { return xf94_27_inMapScreen; }
  bool IsFullThreat() const { return xf94_30_fullThreat; }
  void SetIsFullThreat(bool v) { xf94_30_fullThreat = v; }

  const std::shared_ptr<CPlayerState>& GetPlayerState() const { return x8b8_playerState; }
  CRandom16* GetActiveRandom() { return x900_activeRandom; }
  const CRandom16* GetActiveRandom() const { return x900_activeRandom; }
  zeus::CVector3f Random2f(float scaleMin, float scaleMax);
  void SetActiveRandomToDefault() { x900_activeRandom = &x8fc_random; }
  void ClearActiveRandom() { x900_activeRandom = nullptr; }
  CRumbleManager& GetRumbleManager() { return *x88c_rumbleManager; }
  const CRumbleManager& GetRumbleManager() const { return *x88c_rumbleManager; }
  CCameraFilterPassPoly& GetCameraFilterPass(int idx) { return xb84_camFilterPasses[idx]; }
  const CCameraFilterPassPoly& GetCameraFilterPass(int idx) const { return xb84_camFilterPasses[idx]; }
  CCameraBlurPass& GetCameraBlurPass(int idx) { return xd14_camBlurPasses[idx]; }
  const CCameraBlurPass& GetCameraBlurPass(int idx) const { return xd14_camBlurPasses[idx]; }

  CEnvFxManager* GetEnvFxManager() { return x880_envFxManager; }
  const CEnvFxManager* GetEnvFxManager() const { return x880_envFxManager; }
  CWorld* GetWorld() { return x850_world.get(); }
  const CWorld* GetWorld() const { return x850_world.get(); }
  CScriptMailbox* GetMailbox() { return x8bc_mailbox.get(); }
  const CScriptMailbox* GetRelayTracker() const { return x8bc_mailbox.get(); }
  CCameraManager* GetCameraManager() const { return x870_cameraManager; }
  CFluidPlaneManager* GetFluidPlaneManager() const { return x87c_fluidPlaneManager; }
  CActorModelParticles* GetActorModelParticles() const { return x884_actorModelParticles; }

  const std::shared_ptr<CMapWorldInfo>& MapWorldInfo() const { return x8c0_mapWorldInfo; }
  const std::shared_ptr<CWorldTransManager>& WorldTransManager() const { return x8c4_worldTransManager; }
  const std::shared_ptr<CScriptLayerManager>& WorldLayerState() const { return x8c8_worldLayerState; }
  std::shared_ptr<CScriptLayerManager>& WorldLayerState() { return x8c8_worldLayerState; }

  CPlayer& GetPlayer() const { return *x84c_player; }
  CPlayer* Player() const { return x84c_player.get(); }

  CObjectList& GetAllObjectList() const { return *x808_objLists[0]; }
  CActorList& GetActorObjectList() const { return static_cast<CActorList&>(*x808_objLists[1]); }
  CPhysicsActorList& GetPhysicsActorObjectList() const { return static_cast<CPhysicsActorList&>(*x808_objLists[2]); }
  CGameCameraList& GetCameraObjectList() const { return static_cast<CGameCameraList&>(*x808_objLists[3]); }
  CGameLightList& GetLightObjectList() const { return static_cast<CGameLightList&>(*x808_objLists[4]); }
  CListeningAiList& GetListeningAiObjectList() const { return static_cast<CListeningAiList&>(*x808_objLists[5]); }
  CAiWaypointList& GetAiWaypointObjectList() const { return static_cast<CAiWaypointList&>(*x808_objLists[6]); }
  CPlatformAndDoorList& GetPlatformAndDoorObjectList() const {
    return static_cast<CPlatformAndDoorList&>(*x808_objLists[7]);
  }
  std::pair<u32, u32> CalculateScanCompletionRate() const;
  void SetCurrentMaze(std::unique_ptr<CMazeState> maze) { xf70_currentMaze = std::move(maze); }
  void ClearCurrentMaze() { xf70_currentMaze.reset(); }
  CMazeState* GetCurrentMaze() { return xf70_currentMaze.get(); }
  void SetLastTriggerId(TUniqueId uid) { xf74_lastTrigger = uid; }
  TUniqueId GetLastTriggerId() const { return xf74_lastTrigger; }
  void SetLastRelayId(TUniqueId uid) { xf76_lastRelay = uid; }
  TUniqueId* GetLastRelayIdPtr() { return &xf76_lastRelay; }
  TUniqueId GetLastRelayId() const { return xf76_lastRelay; }
  bool GetIsGeneratingObject() const { return xf94_26_generatingObject; }
  void SetIsGeneratingObject(bool gen) { xf94_26_generatingObject = gen; }
  EThermalDrawFlag GetThermalDrawFlag() const { return xf34_thermalFlag; }
  const CFinalInput& GetFinalInput() const { return xb54_finalInput; }
  void SetBossParams(TUniqueId bossId, float maxEnergy, u32 stringIdx);
  TUniqueId GetBossId() const { return xf18_bossId; }
  float GetTotalBossEnergy() const { return xf1c_totalBossEnergy; }
  u32 GetBossStringIdx() const { return xf20_bossStringIdx; }
  void SetPendingOnScreenTex(CAssetId texId, const zeus::CVector2i& origin, const zeus::CVector2i& extent) {
    xef4_pendingScreenTex.x0_id = texId;
    xef4_pendingScreenTex.x4_origin = origin;
    xef4_pendingScreenTex.xc_extent = extent;
  }
  const SOnScreenTex& GetPendingScreenTex() const { return xef4_pendingScreenTex; }
  void SetViewportScale(const zeus::CVector2f& scale) { xf2c_viewportScale = scale; }
  float GetThermalColdScale1() const { return xf24_thermColdScale1; }
  float GetThermalColdScale2() const { return xf28_thermColdScale2; }
  void SetThermalColdScale2(float s) { xf28_thermColdScale2 = s; }
  float IntegrateVisorFog(float f) const;
  u32 GetUpdateFrameIndex() const { return x8d8_updateFrameIdx; }
  void SetCinematicPause(bool p) { xf94_29_cinematicPause = p; }
  void QueueMessage(u32 frameCount, CAssetId msg, float f1) {
    xf84_ = frameCount;
    xf88_ = msg;
    xf8c_ = f1;
  }
  TUniqueId GetPlayerActorHead() const { return xf6c_playerActorHead; }
  void SetPlayerActorHead(TUniqueId id) { xf6c_playerActorHead = id; }
  std::list<TUniqueId>& GetActiveFlickerBats() { return xf3c_activeFlickerBats; }
  std::list<TUniqueId>& GetActiveParasites() { return xf54_activeParasites; }
  static float g_EscapeShakeCountdown;
  static bool g_EscapeShakeCountdownInit;

  void sub_80044098(const CCollisionResponseData& colRespData, const CRayCastResult& rayCast, TUniqueId uid,
                    const CWeaponMode& weaponMode, u32 w1, u8 thermalFlags);

  const CGameArea* GetCurrentArea() const;
  void SetWarping(bool warp) { m_warping = warp; }

  #ifdef PRIME2
  struct astruct_6 {
    bool FUN_801eaf90(CPlayer* player, CStateManager& mgr) { /* NYI */ return false; }
  };

  int MaskUIdNumPlayers(TUniqueId uid) const;

  // Echoes: x294c
  bool echoesFlags_31_inDarkWorld;

  // Echoes: x14f8
  int numPlayers;

  astruct_6* field_0x1638;
  #endif
};
} // namespace metaforce
