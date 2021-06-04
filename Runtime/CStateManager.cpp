#include "Runtime/CStateManager.hpp"

#include <cmath>

#include "Runtime/AutoMapper/CMapWorldInfo.hpp"
#include "Runtime/Camera/CBallCamera.hpp"
#include "Runtime/Camera/CCameraShakeData.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/CGameState.hpp"
#include "Runtime/CMemoryCardSys.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollidableSphere.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Collision/CMaterialFilter.hpp"
#include "Runtime/Collision/CollisionUtil.hpp"
#include "Runtime/CPlayerState.hpp"
#include "Runtime/CSortedLists.hpp"
#include "Runtime/CTimeProvider.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CLight.hpp"
#include "Runtime/Input/ControlMapper.hpp"
#include "Runtime/Input/CRumbleManager.hpp"
#include "Runtime/MP1/CSamusHud.hpp"
#include "Runtime/MP1/MP1.hpp"
#include "Runtime/Particle/CDecalManager.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"
#include "Runtime/Weapon/CProjectileWeapon.hpp"
#include "Runtime/Weapon/CWeapon.hpp"
#include "Runtime/Weapon/CWeaponMgr.hpp"
#include "Runtime/World/CDestroyableRock.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CProjectedShadow.hpp"
#include "Runtime/World/CScriptDebris.hpp"
#include "Runtime/World/CScriptDock.hpp"
#include "Runtime/World/CScriptDoor.hpp"
#include "Runtime/World/CScriptEffect.hpp"
#include "Runtime/World/CScriptPlatform.hpp"
#include "Runtime/World/CScriptPlayerActor.hpp"
#include "Runtime/World/CScriptRoomAcoustics.hpp"
#include "Runtime/World/CScriptSpawnPoint.hpp"
#include "Runtime/World/CScriptSpecialFunction.hpp"
#include "Runtime/World/CScriptWater.hpp"
#include "Runtime/World/CSnakeWeedSwarm.hpp"
#include "Runtime/World/CWallCrawlerSwarm.hpp"
#include "Runtime/World/CWorld.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

#include <hecl/CVarManager.hpp>
#include <zeus/CMRay.hpp>

namespace metaforce {
namespace {
hecl::CVar* debugToolDrawAiPath = nullptr;
hecl::CVar* debugToolDrawLighting = nullptr;
hecl::CVar* debugToolDrawCollisionActors = nullptr;
hecl::CVar* debugToolDrawMazePath = nullptr;
hecl::CVar* debugToolDrawPlatformCollision = nullptr;
hecl::CVar* sm_logScripting = nullptr;
} // namespace
logvisor::Module LogModule("metaforce::CStateManager");
CStateManager::CStateManager(const std::weak_ptr<CScriptMailbox>& mailbox,
                             const std::weak_ptr<CMapWorldInfo>& mwInfo, const std::weak_ptr<CPlayerState>& playerState,
                             const std::weak_ptr<CWorldTransManager>& wtMgr,
                             const std::weak_ptr<CScriptLayerManager>& layerState)
: x8b8_playerState(playerState)
, x8bc_mailbox(mailbox)
, x8c0_mapWorldInfo(mwInfo)
, x8c4_worldTransManager(wtMgr)
, x8c8_worldLayerState(layerState) {
  x86c_stateManagerContainer = std::make_unique<CStateManagerContainer>();
  x870_cameraManager = &x86c_stateManagerContainer->x0_cameraManager;
  x874_sortedListManager = &x86c_stateManagerContainer->x3c0_sortedListManager;
  x878_weaponManager = &x86c_stateManagerContainer->xe3d8_weaponManager;
  x87c_fluidPlaneManager = &x86c_stateManagerContainer->xe3ec_fluidPlaneManager;
  x880_envFxManager = &x86c_stateManagerContainer->xe510_envFxManager;
  x884_actorModelParticles = &x86c_stateManagerContainer->xf168_actorModelParticles;
  x88c_rumbleManager = &x86c_stateManagerContainer->xf250_rumbleManager;

  g_Renderer->SetDrawableCallback(&CStateManager::RendererDrawCallback, this);
  x908_loaderCount = int(EScriptObjectType::ScriptObjectTypeMAX);
  x90c_loaderFuncs[size_t(EScriptObjectType::Actor)] = ScriptLoader::LoadActor;
  x90c_loaderFuncs[size_t(EScriptObjectType::Waypoint)] = ScriptLoader::LoadWaypoint;
  x90c_loaderFuncs[size_t(EScriptObjectType::Door)] = ScriptLoader::LoadDoor;
  x90c_loaderFuncs[size_t(EScriptObjectType::Trigger)] = ScriptLoader::LoadTrigger;
  x90c_loaderFuncs[size_t(EScriptObjectType::Timer)] = ScriptLoader::LoadTimer;
  x90c_loaderFuncs[size_t(EScriptObjectType::Counter)] = ScriptLoader::LoadCounter;
  x90c_loaderFuncs[size_t(EScriptObjectType::Effect)] = ScriptLoader::LoadEffect;
  x90c_loaderFuncs[size_t(EScriptObjectType::Platform)] = ScriptLoader::LoadPlatform;
  x90c_loaderFuncs[size_t(EScriptObjectType::Sound)] = ScriptLoader::LoadSound;
  x90c_loaderFuncs[size_t(EScriptObjectType::Generator)] = ScriptLoader::LoadGenerator;
  x90c_loaderFuncs[size_t(EScriptObjectType::Dock)] = ScriptLoader::LoadDock;
  x90c_loaderFuncs[size_t(EScriptObjectType::Camera)] = ScriptLoader::LoadCamera;
  x90c_loaderFuncs[size_t(EScriptObjectType::CameraWaypoint)] = ScriptLoader::LoadCameraWaypoint;
  x90c_loaderFuncs[size_t(EScriptObjectType::NewIntroBoss)] = ScriptLoader::LoadNewIntroBoss;
  x90c_loaderFuncs[size_t(EScriptObjectType::SpawnPoint)] = ScriptLoader::LoadSpawnPoint;
  x90c_loaderFuncs[size_t(EScriptObjectType::CameraHint)] = ScriptLoader::LoadCameraHint;
  x90c_loaderFuncs[size_t(EScriptObjectType::Pickup)] = ScriptLoader::LoadPickup;
  x90c_loaderFuncs[size_t(EScriptObjectType::MemoryRelay)] = ScriptLoader::LoadMemoryRelay;
  x90c_loaderFuncs[size_t(EScriptObjectType::RandomRelay)] = ScriptLoader::LoadRandomRelay;
  x90c_loaderFuncs[size_t(EScriptObjectType::Relay)] = ScriptLoader::LoadRelay;
  x90c_loaderFuncs[size_t(EScriptObjectType::Beetle)] = ScriptLoader::LoadBeetle;
  x90c_loaderFuncs[size_t(EScriptObjectType::HUDMemo)] = ScriptLoader::LoadHUDMemo;
  x90c_loaderFuncs[size_t(EScriptObjectType::CameraFilterKeyframe)] = ScriptLoader::LoadCameraFilterKeyframe;
  x90c_loaderFuncs[size_t(EScriptObjectType::CameraBlurKeyframe)] = ScriptLoader::LoadCameraBlurKeyframe;
  x90c_loaderFuncs[size_t(EScriptObjectType::DamageableTrigger)] = ScriptLoader::LoadDamageableTrigger;
  x90c_loaderFuncs[size_t(EScriptObjectType::Debris)] = ScriptLoader::LoadDebris;
  x90c_loaderFuncs[size_t(EScriptObjectType::CameraShaker)] = ScriptLoader::LoadCameraShaker;
  x90c_loaderFuncs[size_t(EScriptObjectType::ActorKeyframe)] = ScriptLoader::LoadActorKeyframe;
  x90c_loaderFuncs[size_t(EScriptObjectType::Water)] = ScriptLoader::LoadWater;
  x90c_loaderFuncs[size_t(EScriptObjectType::Warwasp)] = ScriptLoader::LoadWarWasp;
  x90c_loaderFuncs[size_t(EScriptObjectType::SpacePirate)] = ScriptLoader::LoadSpacePirate;
  x90c_loaderFuncs[size_t(EScriptObjectType::FlyingPirate)] = ScriptLoader::LoadFlyingPirate;
  x90c_loaderFuncs[size_t(EScriptObjectType::ElitePirate)] = ScriptLoader::LoadElitePirate;
  x90c_loaderFuncs[size_t(EScriptObjectType::MetroidBeta)] = ScriptLoader::LoadMetroidBeta;
  x90c_loaderFuncs[size_t(EScriptObjectType::ChozoGhost)] = ScriptLoader::LoadChozoGhost;
  x90c_loaderFuncs[size_t(EScriptObjectType::CoverPoint)] = ScriptLoader::LoadCoverPoint;
  x90c_loaderFuncs[size_t(EScriptObjectType::SpiderBallWaypoint)] = ScriptLoader::LoadSpiderBallWaypoint;
  x90c_loaderFuncs[size_t(EScriptObjectType::BloodFlower)] = ScriptLoader::LoadBloodFlower;
  x90c_loaderFuncs[size_t(EScriptObjectType::FlickerBat)] = ScriptLoader::LoadFlickerBat;
  x90c_loaderFuncs[size_t(EScriptObjectType::PathCamera)] = ScriptLoader::LoadPathCamera;
  x90c_loaderFuncs[size_t(EScriptObjectType::GrapplePoint)] = ScriptLoader::LoadGrapplePoint;
  x90c_loaderFuncs[size_t(EScriptObjectType::PuddleSpore)] = ScriptLoader::LoadPuddleSpore;
  x90c_loaderFuncs[size_t(EScriptObjectType::DebugCameraWaypoint)] = ScriptLoader::LoadDebugCameraWaypoint;
  x90c_loaderFuncs[size_t(EScriptObjectType::SpiderBallAttractionSurface)] =
      ScriptLoader::LoadSpiderBallAttractionSurface;
  x90c_loaderFuncs[size_t(EScriptObjectType::PuddleToadGamma)] = ScriptLoader::LoadPuddleToadGamma;
  x90c_loaderFuncs[size_t(EScriptObjectType::DistanceFog)] = ScriptLoader::LoadDistanceFog;
  x90c_loaderFuncs[size_t(EScriptObjectType::FireFlea)] = ScriptLoader::LoadFireFlea;
  x90c_loaderFuncs[size_t(EScriptObjectType::Metaree)] = ScriptLoader::LoadMetaree;
  x90c_loaderFuncs[size_t(EScriptObjectType::DockAreaChange)] = ScriptLoader::LoadDockAreaChange;
  x90c_loaderFuncs[size_t(EScriptObjectType::ActorRotate)] = ScriptLoader::LoadActorRotate;
  x90c_loaderFuncs[size_t(EScriptObjectType::SpecialFunction)] = ScriptLoader::LoadSpecialFunction;
  x90c_loaderFuncs[size_t(EScriptObjectType::SpankWeed)] = ScriptLoader::LoadSpankWeed;
  x90c_loaderFuncs[size_t(EScriptObjectType::Parasite)] = ScriptLoader::LoadParasite;
  x90c_loaderFuncs[size_t(EScriptObjectType::PlayerHint)] = ScriptLoader::LoadPlayerHint;
  x90c_loaderFuncs[size_t(EScriptObjectType::Ripper)] = ScriptLoader::LoadRipper;
  x90c_loaderFuncs[size_t(EScriptObjectType::PickupGenerator)] = ScriptLoader::LoadPickupGenerator;
  x90c_loaderFuncs[size_t(EScriptObjectType::AIKeyframe)] = ScriptLoader::LoadAIKeyframe;
  x90c_loaderFuncs[size_t(EScriptObjectType::PointOfInterest)] = ScriptLoader::LoadPointOfInterest;
  x90c_loaderFuncs[size_t(EScriptObjectType::Drone)] = ScriptLoader::LoadDrone;
  x90c_loaderFuncs[size_t(EScriptObjectType::Metroid)] = ScriptLoader::LoadMetroid;
  x90c_loaderFuncs[size_t(EScriptObjectType::DebrisExtended)] = ScriptLoader::LoadDebrisExtended;
  x90c_loaderFuncs[size_t(EScriptObjectType::Steam)] = ScriptLoader::LoadSteam;
  x90c_loaderFuncs[size_t(EScriptObjectType::Ripple)] = ScriptLoader::LoadRipple;
  x90c_loaderFuncs[size_t(EScriptObjectType::BallTrigger)] = ScriptLoader::LoadBallTrigger;
  x90c_loaderFuncs[size_t(EScriptObjectType::TargetingPoint)] = ScriptLoader::LoadTargetingPoint;
  x90c_loaderFuncs[size_t(EScriptObjectType::EMPulse)] = ScriptLoader::LoadEMPulse;
  x90c_loaderFuncs[size_t(EScriptObjectType::IceSheegoth)] = ScriptLoader::LoadIceSheegoth;
  x90c_loaderFuncs[size_t(EScriptObjectType::PlayerActor)] = ScriptLoader::LoadPlayerActor;
  x90c_loaderFuncs[size_t(EScriptObjectType::Flaahgra)] = ScriptLoader::LoadFlaahgra;
  x90c_loaderFuncs[size_t(EScriptObjectType::AreaAttributes)] = ScriptLoader::LoadAreaAttributes;
  x90c_loaderFuncs[size_t(EScriptObjectType::FishCloud)] = ScriptLoader::LoadFishCloud;
  x90c_loaderFuncs[size_t(EScriptObjectType::FishCloudModifier)] = ScriptLoader::LoadFishCloudModifier;
  x90c_loaderFuncs[size_t(EScriptObjectType::VisorFlare)] = ScriptLoader::LoadVisorFlare;
  x90c_loaderFuncs[size_t(EScriptObjectType::WorldTeleporter)] = ScriptLoader::LoadWorldTeleporter;
  x90c_loaderFuncs[size_t(EScriptObjectType::VisorGoo)] = ScriptLoader::LoadVisorGoo;
  x90c_loaderFuncs[size_t(EScriptObjectType::JellyZap)] = ScriptLoader::LoadJellyZap;
  x90c_loaderFuncs[size_t(EScriptObjectType::ControllerAction)] = ScriptLoader::LoadControllerAction;
  x90c_loaderFuncs[size_t(EScriptObjectType::Switch)] = ScriptLoader::LoadSwitch;
  x90c_loaderFuncs[size_t(EScriptObjectType::PlayerStateChange)] = ScriptLoader::LoadPlayerStateChange;
  x90c_loaderFuncs[size_t(EScriptObjectType::Thardus)] = ScriptLoader::LoadThardus;
  x90c_loaderFuncs[size_t(EScriptObjectType::WallCrawlerSwarm)] = ScriptLoader::LoadWallCrawlerSwarm;
  x90c_loaderFuncs[size_t(EScriptObjectType::AIJumpPoint)] = ScriptLoader::LoadAiJumpPoint;
  x90c_loaderFuncs[size_t(EScriptObjectType::FlaahgraTentacle)] = ScriptLoader::LoadFlaahgraTentacle;
  x90c_loaderFuncs[size_t(EScriptObjectType::RoomAcoustics)] = ScriptLoader::LoadRoomAcoustics;
  x90c_loaderFuncs[size_t(EScriptObjectType::ColorModulate)] = ScriptLoader::LoadColorModulate;
  x90c_loaderFuncs[size_t(EScriptObjectType::ThardusRockProjectile)] = ScriptLoader::LoadThardusRockProjectile;
  x90c_loaderFuncs[size_t(EScriptObjectType::Midi)] = ScriptLoader::LoadMidi;
  x90c_loaderFuncs[size_t(EScriptObjectType::StreamedAudio)] = ScriptLoader::LoadStreamedAudio;
  x90c_loaderFuncs[size_t(EScriptObjectType::WorldTeleporterToo)] = ScriptLoader::LoadWorldTeleporter;
  x90c_loaderFuncs[size_t(EScriptObjectType::Repulsor)] = ScriptLoader::LoadRepulsor;
  x90c_loaderFuncs[size_t(EScriptObjectType::GunTurret)] = ScriptLoader::LoadGunTurret;
  x90c_loaderFuncs[size_t(EScriptObjectType::FogVolume)] = ScriptLoader::LoadFogVolume;
  x90c_loaderFuncs[size_t(EScriptObjectType::Babygoth)] = ScriptLoader::LoadBabygoth;
  x90c_loaderFuncs[size_t(EScriptObjectType::Eyeball)] = ScriptLoader::LoadEyeball;
  x90c_loaderFuncs[size_t(EScriptObjectType::RadialDamage)] = ScriptLoader::LoadRadialDamage;
  x90c_loaderFuncs[size_t(EScriptObjectType::CameraPitchVolume)] = ScriptLoader::LoadCameraPitchVolume;
  x90c_loaderFuncs[size_t(EScriptObjectType::EnvFxDensityController)] = ScriptLoader::LoadEnvFxDensityController;
  x90c_loaderFuncs[size_t(EScriptObjectType::Magdolite)] = ScriptLoader::LoadMagdolite;
  x90c_loaderFuncs[size_t(EScriptObjectType::TeamAIMgr)] = ScriptLoader::LoadTeamAIMgr;
  x90c_loaderFuncs[size_t(EScriptObjectType::SnakeWeedSwarm)] = ScriptLoader::LoadSnakeWeedSwarm;
  x90c_loaderFuncs[size_t(EScriptObjectType::ActorContraption)] = ScriptLoader::LoadActorContraption;
  x90c_loaderFuncs[size_t(EScriptObjectType::Oculus)] = ScriptLoader::LoadOculus;
  x90c_loaderFuncs[size_t(EScriptObjectType::Geemer)] = ScriptLoader::LoadGeemer;
  x90c_loaderFuncs[size_t(EScriptObjectType::SpindleCamera)] = ScriptLoader::LoadSpindleCamera;
  x90c_loaderFuncs[size_t(EScriptObjectType::AtomicAlpha)] = ScriptLoader::LoadAtomicAlpha;
  x90c_loaderFuncs[size_t(EScriptObjectType::CameraHintTrigger)] = ScriptLoader::LoadCameraHintTrigger;
  x90c_loaderFuncs[size_t(EScriptObjectType::RumbleEffect)] = ScriptLoader::LoadRumbleEffect;
  x90c_loaderFuncs[size_t(EScriptObjectType::AmbientAI)] = ScriptLoader::LoadAmbientAI;
  x90c_loaderFuncs[size_t(EScriptObjectType::AtomicBeta)] = ScriptLoader::LoadAtomicBeta;
  x90c_loaderFuncs[size_t(EScriptObjectType::IceZoomer)] = ScriptLoader::LoadIceZoomer;
  x90c_loaderFuncs[size_t(EScriptObjectType::Puffer)] = ScriptLoader::LoadPuffer;
  x90c_loaderFuncs[size_t(EScriptObjectType::Tryclops)] = ScriptLoader::LoadTryclops;
  x90c_loaderFuncs[size_t(EScriptObjectType::Ridley)] = ScriptLoader::LoadRidley;
  x90c_loaderFuncs[size_t(EScriptObjectType::Seedling)] = ScriptLoader::LoadSeedling;
  x90c_loaderFuncs[size_t(EScriptObjectType::ThermalHeatFader)] = ScriptLoader::LoadThermalHeatFader;
  x90c_loaderFuncs[size_t(EScriptObjectType::Burrower)] = ScriptLoader::LoadBurrower;
  x90c_loaderFuncs[size_t(EScriptObjectType::ScriptBeam)] = ScriptLoader::LoadBeam;
  x90c_loaderFuncs[size_t(EScriptObjectType::WorldLightFader)] = ScriptLoader::LoadWorldLightFader;
  x90c_loaderFuncs[size_t(EScriptObjectType::MetroidPrimeStage2)] = ScriptLoader::LoadMetroidPrimeEssence;
  x90c_loaderFuncs[size_t(EScriptObjectType::MetroidPrimeStage1)] = ScriptLoader::LoadMetroidPrimeStage1;
  x90c_loaderFuncs[size_t(EScriptObjectType::MazeNode)] = ScriptLoader::LoadMazeNode;
  x90c_loaderFuncs[size_t(EScriptObjectType::OmegaPirate)] = ScriptLoader::LoadOmegaPirate;
  x90c_loaderFuncs[size_t(EScriptObjectType::PhazonPool)] = ScriptLoader::LoadPhazonPool;
  x90c_loaderFuncs[size_t(EScriptObjectType::PhazonHealingNodule)] = ScriptLoader::LoadPhazonHealingNodule;
  x90c_loaderFuncs[size_t(EScriptObjectType::NewCameraShaker)] = ScriptLoader::LoadNewCameraShaker;
  x90c_loaderFuncs[size_t(EScriptObjectType::ShadowProjector)] = ScriptLoader::LoadShadowProjector;
  x90c_loaderFuncs[size_t(EScriptObjectType::EnergyBall)] = ScriptLoader::LoadEnergyBall;

  CGameCollision::InitCollision();
  ControlMapper::ResetCommandFilters();
  x8f0_shadowTex = g_SimplePool->GetObj("DefaultShadow");
  g_StateManager = this;

  sm_logScripting = hecl::CVarManager::instance()->findOrMakeCVar(
      "stateManager.logScripting"sv, "Prints object communication to the console", false,
      hecl::CVar::EFlags::ReadOnly | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::Game);
}

CStateManager::~CStateManager() {
  x88c_rumbleManager->HardStopAll();
  x880_envFxManager->Cleanup();
  x900_activeRandom = &x8fc_random;
  ClearGraveyard();
  for (auto it = x808_objLists[0]->begin(); it != x808_objLists[0]->end();) {
    CEntity* ent = *it;
    ++it;
    if (ent == x84c_player.get()) {
      continue;
    }
    ent->AcceptScriptMsg(EScriptObjectMessage::Deleted, kInvalidUniqueId, *this);
    RemoveObject(ent->GetUniqueId());
    std::default_delete<CEntity>()(ent);
  }
  ClearGraveyard();
  x84c_player->AcceptScriptMsg(EScriptObjectMessage::Deleted, kInvalidUniqueId, *this);
  RemoveObject(x84c_player->GetUniqueId());
  x84c_player.reset();
  CCollisionPrimitive::Uninitialize();
  g_StateManager = nullptr;
}

void CStateManager::UpdateThermalVisor() {
  xf28_thermColdScale2 = 0.f;
  xf24_thermColdScale1 = 0.f;

  const auto visor = x8b8_playerState->GetActiveVisor(*this);
  if (visor != CPlayerState::EPlayerVisor::Thermal || x8cc_nextAreaId == kInvalidAreaId) {
    return;
  }

  CGameArea* area = x850_world->GetArea(x8cc_nextAreaId);
  const zeus::CTransform& playerXf = x84c_player->GetTransform();
  const zeus::CVector3f playerXYPos(playerXf.origin.x(), playerXf.origin.y(), 0.f);
  CGameArea* lastArea = nullptr;
  float closestDist = FLT_MAX;
  for (const CGameArea::Dock& dock : area->GetDocks()) {
    zeus::CVector3f dockCenter = (dock.GetPlaneVertices()[0] + dock.GetPlaneVertices()[1] + dock.GetPlaneVertices()[2] +
                                  dock.GetPlaneVertices()[3]) *
                                 0.25f;
    dockCenter.z() = 0.f;
    const float dist = (playerXYPos - dockCenter).magSquared();
    if (dist < closestDist) {
      const TAreaId connAreaId = dock.GetConnectedAreaId(0);
      if (connAreaId != kInvalidAreaId) {
        CGameArea* connArea = x850_world->GetArea(x8cc_nextAreaId);
        if (connArea->IsPostConstructed()) {
          const auto occState = connArea->GetPostConstructed()->x10dc_occlusionState;
          if (occState == CGameArea::EOcclusionState::Visible) {
            closestDist = dist;
            lastArea = connArea;
          }
        }
      }
    }
  }

  if (lastArea != nullptr) {
    if (closestDist != 0.f) {
      closestDist /= std::sqrt(closestDist);
    }
    closestDist -= 2.f;
    if (closestDist < 8.f) {
      if (closestDist > 0.f) {
        closestDist = (closestDist / 8.f) * 0.5f + 0.5f;
      } else {
        closestDist = 0.5f;
      }

      xf24_thermColdScale1 = (1.f - closestDist) * lastArea->GetPostConstructed()->x111c_thermalCurrent +
                             closestDist * area->GetPostConstructed()->x111c_thermalCurrent;
      return;
    }
  }

  xf24_thermColdScale1 = area->GetPostConstructed()->x111c_thermalCurrent;
}

void CStateManager::RendererDrawCallback(void* drawable, void* ctx, int type) {
  CStateManager& mgr = *static_cast<CStateManager*>(ctx);
  switch (type) {
  case 0: {
    CActor& actor = *static_cast<CActor*>(drawable);
    if (actor.xc8_drawnToken == mgr.x8dc_objectDrawToken) {
      break;
    }
    if (actor.xc6_nextDrawNode != kInvalidUniqueId) {
      mgr.RecursiveDrawTree(actor.xc6_nextDrawNode);
    }
    actor.Render(mgr);
    actor.xc8_drawnToken = mgr.x8dc_objectDrawToken;
    break;
  }
  case 1:
    static_cast<CSimpleShadow*>(drawable)->Render(mgr.x8f0_shadowTex);
    break;
  case 2:
    static_cast<CDecal*>(drawable)->Render();
    break;
  default:
    break;
  }
}

bool CStateManager::RenderLast(TUniqueId uid) {
  if (x86c_stateManagerContainer->xf39c_renderLast.size() == 20) {
    return false;
  }
  x86c_stateManagerContainer->xf39c_renderLast.push_back(uid);
  return true;
}

void CStateManager::AddDrawableActorPlane(CActor& actor, const zeus::CPlane& plane, const zeus::CAABox& aabb) const {
  actor.SetAddedToken(x8dc_objectDrawToken + 1);
  g_Renderer->AddPlaneObject(&actor, aabb, plane, 0);
}

void CStateManager::AddDrawableActor(CActor& actor, const zeus::CVector3f& vec, const zeus::CAABox& aabb) const {
  actor.SetAddedToken(x8dc_objectDrawToken + 1);
  g_Renderer->AddDrawable(&actor, vec, aabb, 0, IRenderer::EDrawableSorting::SortedCallback);
}

bool CStateManager::SpecialSkipCinematic() {
  if (xf38_skipCineSpecialFunc == kInvalidUniqueId) {
    return false;
  }

  auto* ent = static_cast<CScriptSpecialFunction*>(ObjectById(xf38_skipCineSpecialFunc));
  if (ent == nullptr || !ent->ShouldSkipCinematic(*this)) {
    return false;
  }

  const bool hadRandom = x900_activeRandom != nullptr;
  SetActiveRandomToDefault();
  x870_cameraManager->SkipCinematic(*this);
  ent->SkipCinematic(*this);
  x900_activeRandom = hadRandom ? &x8fc_random : nullptr;

  return true;
}

TAreaId CStateManager::GetVisAreaId() const {
  const CGameCamera* cam = static_cast<const CGameCamera*>(x870_cameraManager->GetCurrentCamera(*this));
  const CBallCamera* ballCam = x870_cameraManager->GetBallCamera();
  const TAreaId curArea = x850_world->x68_curAreaId;
  if (cam != ballCam) {
    return curArea;
  }

  const zeus::CVector3f& camTranslation = ballCam->GetTranslation();
  zeus::CAABox camAABB(camTranslation, camTranslation);
  camAABB.accumulateBounds(x84c_player->GetTranslation());
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  BuildNearList(nearList, camAABB,
                CMaterialFilter(EMaterialTypes::AIBlock, CMaterialList(), CMaterialFilter::EFilterType::Include),
                nullptr);
  for (const auto& id : nearList) {
    if (const TCastToConstPtr<CScriptDock> dock = GetObjectById(id)) {
      if (dock->GetAreaId() == curArea && dock->HasPointCrossedDock(*this, camTranslation)) {
        return dock->GetCurrentConnectedAreaId(*this);
      }
    }
  }

  return curArea;
}

s32 CStateManager::GetWeaponIdCount(TUniqueId uid, EWeaponType type) const {
  return x878_weaponManager->GetNumActive(uid, type);
}

void CStateManager::RemoveWeaponId(TUniqueId uid, EWeaponType type) { x878_weaponManager->DecrCount(uid, type); }

void CStateManager::AddWeaponId(TUniqueId uid, EWeaponType type) { x878_weaponManager->IncrCount(uid, type); }

void CStateManager::UpdateEscapeSequenceTimer(float dt) {
  if (xf0c_escapeTimer <= 0.f) {
    return;
  }

  xf0c_escapeTimer = std::max(FLT_EPSILON, xf0c_escapeTimer - dt);
  if (xf0c_escapeTimer <= FLT_EPSILON) {
    x8b8_playerState->SetPlayerAlive(false);
  }

  if (!g_EscapeShakeCountdownInit) {
    g_EscapeShakeCountdown = 0.f;
    g_EscapeShakeCountdownInit = true;
  }

  g_EscapeShakeCountdown -= dt;
  if (g_EscapeShakeCountdown >= 0.f) {
    return;
  }

  const float factor = 1.f - xf0c_escapeTimer / xf10_escapeTotalTime;
  const float factor2 = factor * factor;
  const CCameraShakeData shakeData(1.f, factor2 * 0.2f * x900_activeRandom->Range(0.5f, 1.f));
  x870_cameraManager->AddCameraShaker(shakeData, true);
  x88c_rumbleManager->Rumble(*this, ERumbleFxId::EscapeSequenceShake, 0.75f, ERumblePriority::One);
  g_EscapeShakeCountdown = -12.f * factor2 + 15.f;
}

void CStateManager::ResetEscapeSequenceTimer(float time) {
  xf0c_escapeTimer = time;
  xf10_escapeTotalTime = time;
}

void CStateManager::SetupParticleHook(const CActor& actor) const {
  x884_actorModelParticles->SetupHook(actor.GetUniqueId());
}

void CStateManager::MurderScriptInstanceNames() { xb40_uniqueInstanceNames.clear(); }

std::string CStateManager::HashInstanceName(CInputStream& in) { return in.readString(); }

void CStateManager::SetActorAreaId(CActor& actor, TAreaId aid) {
  const TAreaId actorAid = actor.GetAreaIdAlways();
  if (actorAid == aid) {
    return;
  }

  if (actorAid != kInvalidAreaId) {
    CGameArea* area = x850_world->GetArea(actorAid);
    if (area->IsPostConstructed()) {
      area->GetAreaObjects()->RemoveObject(actor.GetUniqueId());
    }
  }

  actor.x4_areaId = aid;

  if (aid == kInvalidAreaId) {
    return;
  }

  CGameArea* area = x850_world->GetArea(aid);
  if (!area->IsPostConstructed() || area->GetAreaObjects()->GetValidObjectById(actor.GetUniqueId())) {
    return;
  }

  area->GetAreaObjects()->AddObject(actor);
}

void CStateManager::TouchSky() const { x850_world->TouchSky(); }

void CStateManager::TouchPlayerActor() {
  if (xf6c_playerActorHead == kInvalidUniqueId) {
    return;
  }

  if (CEntity* ent = ObjectById(xf6c_playerActorHead)) {
    static_cast<CScriptPlayerActor*>(ent)->TouchModels(*this);
  }
}

void CStateManager::DrawSpaceWarp(const zeus::CVector3f& v, float strength) const {
  const CPlayerState::EPlayerVisor visor = x8b8_playerState->GetActiveVisor(*this);

  if (visor != CPlayerState::EPlayerVisor::Scan && visor != CPlayerState::EPlayerVisor::Combat) {
    return;
  }

  const zeus::CVector3f screenV =
      TCastToConstPtr<CGameCamera>(x870_cameraManager->GetCurrentCamera(*this))->ConvertToScreenSpace(v);
  g_Renderer->DrawSpaceWarp(screenV, strength);
}

void CStateManager::DrawReflection(const zeus::CVector3f& reflectPoint) {
  const zeus::CAABox aabb = x84c_player->GetBoundingBox();
  const zeus::CVector3f playerPos = aabb.center();
  zeus::CVector3f surfToPlayer = playerPos - reflectPoint;
  surfToPlayer.z() = 0.f;
  const zeus::CVector3f viewPos = playerPos - surfToPlayer.normalized() * 3.5f;
  const zeus::CTransform look = zeus::lookAt(viewPos, playerPos, {0.f, 0.f, -1.f});

  const zeus::CTransform backupView = CGraphics::g_ViewMatrix;
  CGraphics::SetViewPointMatrix(look);
  const CGraphics::CProjectionState backupProj = CGraphics::GetProjectionState();
  const CGameCamera* cam = x870_cameraManager->GetCurrentCamera(*this);
  g_Renderer->SetPerspective(cam->GetFov(), g_Viewport.x8_width, g_Viewport.xc_height, cam->GetNearClipDistance(),
                             cam->GetFarClipDistance());

  x84c_player->RenderReflectedPlayer(*this);

  CGraphics::SetViewPointMatrix(backupView);
  CGraphics::SetProjectionState(backupProj);
}

void CStateManager::ReflectionDrawer(void* ctx, const zeus::CVector3f& vec) {
  static_cast<CStateManager*>(ctx)->DrawReflection(vec);
}

void CStateManager::CacheReflection() { g_Renderer->CacheReflection(ReflectionDrawer, this, true); }

bool CStateManager::CanCreateProjectile(TUniqueId uid, EWeaponType type, int maxAllowed) const {
  return x878_weaponManager->GetNumActive(uid, type) < maxAllowed;
}

void CStateManager::BuildDynamicLightListForWorld() {
  if (x8b8_playerState->GetActiveVisor(*this) == CPlayerState::EPlayerVisor::Thermal) {
    x8e0_dynamicLights.clear();
    return;
  }

  if (GetLightObjectList().size() == 0) {
    return;
  }

  x8e0_dynamicLights.clear();
  for (const CEntity* ent : GetLightObjectList()) {
    const auto& light = static_cast<const CGameLight&>(*ent);
    if (light.GetActive()) {
      const CLight l = light.GetLight();
      if (l.GetIntensity() > FLT_EPSILON && l.GetRadius() > FLT_EPSILON) {
        x8e0_dynamicLights.push_back(l);
      }
    }
  }
}
void CStateManager::DrawDebugStuff() const {
  if (hecl::com_developer != nullptr && !hecl::com_developer->toBoolean()) {
    return;
  }

  // FIXME: Add proper globals for CVars
  if (debugToolDrawAiPath == nullptr || debugToolDrawCollisionActors == nullptr || debugToolDrawLighting == nullptr ||
      debugToolDrawMazePath == nullptr || debugToolDrawPlatformCollision == nullptr) {
    debugToolDrawAiPath = hecl::CVarManager::instance()->findCVar("debugTool.drawAiPath");
    debugToolDrawMazePath = hecl::CVarManager::instance()->findCVar("debugTool.drawMazePath");
    debugToolDrawCollisionActors = hecl::CVarManager::instance()->findCVar("debugTool.drawCollisionActors");
    debugToolDrawLighting = hecl::CVarManager::instance()->findCVar("debugTool.drawLighting");
    debugToolDrawPlatformCollision = hecl::CVarManager::instance()->findCVar("debugTool.drawPlatformCollision");
    return;
  }

  CGraphics::SetModelMatrix(zeus::CTransform());
  for (CEntity* ent : GetActorObjectList()) {
    if (const TCastToPtr<CPatterned> ai = ent) {
      if (CPathFindSearch* path = ai->GetSearchPath()) {
        if (debugToolDrawAiPath->toBoolean()) {
          path->DebugDraw();
        }
      }
    } else if (const TCastToPtr<CGameLight> light = ent) {
      if (debugToolDrawLighting->toBoolean()) {
        light->DebugDraw();
      }
    } else if (const TCastToPtr<CCollisionActor> colAct = ent) {
      if (colAct->GetUniqueId() == x870_cameraManager->GetBallCamera()->GetCollisionActorId()) {
        continue;
      }
      if (debugToolDrawCollisionActors->toBoolean()) {
        colAct->DebugDraw();
      }
    } else if (const TCastToPtr<CScriptPlatform> plat = ent) {
      if (debugToolDrawPlatformCollision->toBoolean() && plat->GetActive()) {
        plat->DebugDraw();
      }
    } else if (const TCastToPtr<CScriptTrigger> tr = ent) {
      tr->DebugDraw();
    }
  }

  auto* gameArea = x850_world->GetArea(x850_world->GetCurrentAreaId());
  if (gameArea != nullptr && debugToolDrawLighting->toBoolean()) {
    gameArea->DebugDraw();
  }

  if (xf70_currentMaze && debugToolDrawMazePath->toBoolean()) {
    xf70_currentMaze->DebugRender();
  }
}

void CStateManager::RenderCamerasAndAreaLights() {
  x870_cameraManager->RenderCameras(*this);
  for (const CCameraFilterPassPoly& filter : xb84_camFilterPasses) {
    filter.Draw();
  }
}

void CStateManager::DrawE3DeathEffect() {
  const CPlayer& player = *x84c_player;
  if (player.x9f4_deathTime <= 0.f) {
    return;
  }

  if (player.x2f8_morphBallState != CPlayer::EPlayerMorphBallState::Unmorphed) {
    const float blurAmt = zeus::clamp(0.f, (player.x9f4_deathTime - 1.f) / (6.f - 1.f), 1.f);
    if (blurAmt > 0.f) {
      CCameraBlurPass blur;
      blur.SetBlur(EBlurType::HiBlur, 7.f * blurAmt, 0.f);
      blur.Draw();
    }
  }

  const float whiteAmt = zeus::clamp(0.f, 1.f - player.x9f4_deathTime / (0.05f * 6.f), 1.f);
  zeus::CColor color = zeus::skWhite;
  color.a() = whiteAmt;
  m_deathWhiteout.draw(color);
}

void CStateManager::DrawAdditionalFilters() {
  if (xf0c_escapeTimer >= 1.f || xf0c_escapeTimer <= 0.f || x870_cameraManager->IsInCinematicCamera()) {
    return;
  }

  zeus::CColor color = zeus::skWhite;
  color.a() = 1.f - xf0c_escapeTimer;
  m_escapeWhiteout.draw(color);
}

zeus::CFrustum CStateManager::SetupDrawFrustum(const SViewport& vp) const {
  zeus::CFrustum ret;
  const CGameCamera* cam = x870_cameraManager->GetCurrentCamera(*this);
  const zeus::CTransform camXf = x870_cameraManager->GetCurrentCameraTransform(*this);
  const int vpWidth = static_cast<int>(xf2c_viewportScale.x() * vp.x8_width);
  const int vpHeight = static_cast<int>(xf2c_viewportScale.y() * vp.xc_height);
  const int vpLeft = static_cast<int>((vp.x8_width - vpWidth) / 2 + vp.x0_left);
  const int vpTop = static_cast<int>((vp.xc_height - vpHeight) / 2 + vp.x4_top);
  g_Renderer->SetViewport(vpLeft, vpTop, vpWidth, vpHeight);
  const float fov = std::atan(std::tan(zeus::degToRad(cam->GetFov()) * 0.5f) * xf2c_viewportScale.y()) * 2.f;
  const float width = xf2c_viewportScale.x() * vp.x8_width;
  const float height = xf2c_viewportScale.y() * vp.xc_height;
  zeus::CProjection proj;
  proj.setPersp(zeus::SProjPersp{fov, width / height, cam->GetNearClipDistance(), cam->GetFarClipDistance()});
  ret.updatePlanes(camXf, proj);
  return ret;
}

zeus::CFrustum CStateManager::SetupViewForDraw(const SViewport& vp) const {
  const CGameCamera* cam = x870_cameraManager->GetCurrentCamera(*this);
  const zeus::CTransform camXf = x870_cameraManager->GetCurrentCameraTransform(*this);
  g_Renderer->SetWorldViewpoint(camXf);
  CBooModel::SetNewPlayerPositionAndTime(x84c_player->GetTranslation());
  const int vpWidth = static_cast<int>(xf2c_viewportScale.x() * vp.x8_width);
  const int vpHeight = static_cast<int>(xf2c_viewportScale.y() * vp.xc_height);
  const int vpLeft = static_cast<int>((vp.x8_width - vpWidth) / 2 + vp.x0_left);
  const int vpTop = static_cast<int>((vp.xc_height - vpHeight) / 2 + vp.x4_top);
  g_Renderer->SetViewport(vpLeft, vpTop, vpWidth, vpHeight);
  CGraphics::SetDepthRange(DEPTH_WORLD, DEPTH_FAR);
  const float fov = std::atan(std::tan(zeus::degToRad(cam->GetFov()) * 0.5f) * xf2c_viewportScale.y()) * 2.f;
  const float width = xf2c_viewportScale.x() * vp.x8_width;
  const float height = xf2c_viewportScale.y() * vp.xc_height;
  g_Renderer->SetPerspective(zeus::radToDeg(fov), width, height, cam->GetNearClipDistance(), cam->GetFarClipDistance());
  zeus::CFrustum frustum;
  zeus::CProjection proj;
  proj.setPersp(zeus::SProjPersp{fov, width / height, cam->GetNearClipDistance(), cam->GetFarClipDistance()});
  frustum.updatePlanes(camXf, proj);
  g_Renderer->SetClippingPlanes(frustum);
  // g_Renderer->PrimColor(zeus::skWhite);
  CGraphics::SetModelMatrix(zeus::CTransform());
  x87c_fluidPlaneManager->StartFrame(false);
  g_Renderer->SetDebugOption(IRenderer::EDebugOption::One, 1);
  return frustum;
}

zeus::CFrustum CStateManager::SetupViewForCubeFaceDraw(const zeus::CVector3f& pos, int face) const {
  const zeus::CTransform mainCamXf = x870_cameraManager->GetCurrentCameraTransform(*this);
  const zeus::CTransform camXf = zeus::CTransform(mainCamXf.basis * CGraphics::skCubeBasisMats[face], pos);
  g_Renderer->SetWorldViewpoint(camXf);
  CBooModel::SetNewPlayerPositionAndTime(x84c_player->GetTranslation());
  constexpr float width = CUBEMAP_RES;
  g_Renderer->SetViewport(0, 0, width, width);
  CGraphics::SetDepthRange(DEPTH_WORLD, DEPTH_FAR);
  constexpr float fov = zeus::degToRad(90.f);
  g_Renderer->SetPerspective(zeus::radToDeg(fov), width, width, 0.2f, 750.f);
  zeus::CFrustum frustum;
  zeus::CProjection proj;
  proj.setPersp(zeus::SProjPersp{fov, 1.f, 0.2f, 750.f});
  frustum.updatePlanes(camXf, proj);
  g_Renderer->SetClippingPlanes(frustum);
  CGraphics::SetModelMatrix(zeus::CTransform());
  return frustum;
}

void CStateManager::ResetViewAfterDraw(const SViewport& backupViewport,
                                       const zeus::CTransform& backupViewMatrix) const {
  g_Renderer->SetViewport(backupViewport.x0_left, backupViewport.x4_top, backupViewport.x8_width,
                          backupViewport.xc_height);
  const CGameCamera* cam = x870_cameraManager->GetCurrentCamera(*this);

  zeus::CFrustum frustum;
  frustum.updatePlanes(backupViewMatrix, zeus::SProjPersp(zeus::degToRad(cam->GetFov()), g_Viewport.aspect,
                                                          cam->GetNearClipDistance(), cam->GetFarClipDistance()));
  g_Renderer->SetClippingPlanes(frustum);

  g_Renderer->SetPerspective(cam->GetFov(), g_Viewport.x8_width, g_Viewport.xc_height, cam->GetNearClipDistance(),
                             cam->GetFarClipDistance());
}

void CStateManager::DrawWorld() {
  SCOPED_GRAPHICS_DEBUG_GROUP("CStateManager::DrawWorld", zeus::skBlue);
  const CTimeProvider timeProvider(xf14_curTimeMod900);
  const SViewport backupViewport = g_Viewport;

  /* Area camera is in (not necessarily player) */
  const TAreaId visAreaId = GetVisAreaId();

  x850_world->TouchSky();

  DrawWorldCubeFaces();

  const zeus::CFrustum frustum = SetupViewForDraw(g_Viewport);
  const zeus::CTransform backupViewMatrix = CGraphics::g_ViewMatrix;

  int areaCount = 0;
  std::array<const CGameArea*, 10> areaArr;
  for (const CGameArea& area : *x850_world) {
    if (areaCount == 10) {
      break;
    }
    CGameArea::EOcclusionState occState = CGameArea::EOcclusionState::Occluded;
    if (area.IsPostConstructed()) {
      occState = area.GetOcclusionState();
    }
    if (occState == CGameArea::EOcclusionState::Visible) {
      areaArr[areaCount++] = &area;
    }
  }

  std::sort(areaArr.begin(), areaArr.begin() + areaCount, [visAreaId](const CGameArea* a, const CGameArea* b) {
    if (a->x4_selfIdx == b->x4_selfIdx) {
      return false;
    }
    if (visAreaId == a->x4_selfIdx) {
      return false;
    }
    if (visAreaId == b->x4_selfIdx) {
      return true;
    }
    return CGraphics::g_ViewPoint.dot(a->GetAABB().center()) > CGraphics::g_ViewPoint.dot(b->GetAABB().center());
  });

  int pvsCount = 0;
  std::array<CPVSVisSet, 10> pvsArr;
  for (auto area = areaArr.cbegin(); area != areaArr.cbegin() + areaCount; ++area) {
    const CGameArea* areaPtr = *area;
    CPVSVisSet& pvsSet = pvsArr[pvsCount++];
    pvsSet.Reset(EPVSVisSetState::OutOfBounds);
    GetVisSetForArea(areaPtr->x4_selfIdx, visAreaId, pvsSet);
  }

  int mask;
  int targetMask;
  const auto visor = x8b8_playerState->GetActiveVisor(*this);
  const bool thermal = visor == CPlayerState::EPlayerVisor::Thermal;
  if (thermal) {
    xf34_thermalFlag = EThermalDrawFlag::Cold;
    mask = 0x34;
    targetMask = 0;
  } else {
    xf34_thermalFlag = EThermalDrawFlag::Bypass;
    mask = 1 << (visor == CPlayerState::EPlayerVisor::XRay ? 3 : 1);
    targetMask = 0;
  }

  g_Renderer->SetThermal(thermal, g_tweakGui->GetThermalVisorLevel(), g_tweakGui->GetThermalVisorColor());
  g_Renderer->SetThermalColdScale(xf28_thermColdScale2 + xf24_thermColdScale1);

  for (int i = areaCount - 1; i >= 0; --i) {
    OPTICK_EVENT("CStateManager::DrawWorld DrawArea");
    const CGameArea& area = *areaArr[i];
    SetupFogForArea(area);
    g_Renderer->EnablePVS(pvsArr[i], area.x4_selfIdx);
    g_Renderer->SetWorldLightFadeLevel(area.GetPostConstructed()->x1128_worldLightingLevel);
    g_Renderer->UpdateAreaUniforms(area.x4_selfIdx);
    g_Renderer->DrawUnsortedGeometry(area.x4_selfIdx, mask, targetMask);
  }

  if (!SetupFogForDraw()) {
    g_Renderer->SetWorldFog(ERglFogMode::None, 0.f, 1.f, zeus::skBlack);
  }

  x850_world->DrawSky(zeus::CTransform::Translate(CGraphics::g_ViewPoint));

  if (areaCount != 0) {
    SetupFogForArea(*areaArr[areaCount - 1]);
  }

  for (const auto& id : x86c_stateManagerContainer->xf370_) {
    if (auto* ent = static_cast<CActor*>(ObjectById(id))) {
      if (!thermal || (ent->xe6_27_thermalVisorFlags & 1) != 0) {
        ent->Render(*this);
      }
    }
  }

  bool morphingPlayerVisible = false;
  int thermalActorCount = 0;
  std::array<CActor*, 1024> thermalActorArr;
  for (int i = 0; i < areaCount; ++i) {
    const CGameArea& area = *areaArr[i];
    CPVSVisSet& pvs = pvsArr[i];
    const bool isVisArea = area.x4_selfIdx == visAreaId;
    SetupFogForArea(area);
    g_Renderer->SetWorldLightFadeLevel(area.GetPostConstructed()->x1128_worldLightingLevel);
    for (CEntity* ent : *area.GetAreaObjects()) {
      if (const TCastToPtr<CActor> actor = ent) {
        if (!actor->IsDrawEnabled()) {
          continue;
        }
        const TUniqueId actorId = actor->GetUniqueId();
        if (!thermal && area.LookupPVSUniqueID(actorId) == actorId) {
          if (pvs.GetVisible(area.LookupPVSID(actorId)) == EPVSVisSetState::EndOfTree) {
            continue;
          }
        }
        if (x84c_player.get() == actor.GetPtr()) {
          if (thermal) {
            continue;
          }
          switch (x84c_player->GetMorphballTransitionState()) {
          case CPlayer::EPlayerMorphBallState::Unmorphed:
          case CPlayer::EPlayerMorphBallState::Morphed:
            x84c_player->AddToRenderer(frustum, *this);
            continue;
          default:
            morphingPlayerVisible = true;
            continue;
          }
        }
        if (!thermal || (actor->xe6_27_thermalVisorFlags & 1) != 0) {
          actor->AddToRenderer(frustum, *this);
        }
        if (thermal && (actor->xe6_27_thermalVisorFlags & 2) != 0) {
          thermalActorArr[thermalActorCount++] = actor.GetPtr();
        }
      }
    }

    if (isVisArea && !thermal) {
      CDecalManager::AddToRenderer(frustum, *this);
      x884_actorModelParticles->AddStragglersToRenderer(*this);
    }

    ++x8dc_objectDrawToken;

    x84c_player->GetMorphBall()->DrawBallShadow(*this);

    if (xf7c_projectedShadow != nullptr) {
      xf7c_projectedShadow->Render(*this);
    }

    g_Renderer->EnablePVS(pvs, area.x4_selfIdx);
    g_Renderer->DrawSortedGeometry(area.x4_selfIdx, mask, targetMask);
  }

  x880_envFxManager->Render(*this);

  if (morphingPlayerVisible) {
    x84c_player->Render(*this);
  }

  g_Renderer->PostRenderFogs();

  if (thermal) {
    if (x86c_stateManagerContainer->xf39c_renderLast.size()) {
      CGraphics::SetDepthRange(DEPTH_SCREEN_ACTORS, DEPTH_GUN);
      for (const auto& id : x86c_stateManagerContainer->xf39c_renderLast) {
        if (auto* actor = static_cast<CActor*>(ObjectById(id))) {
          if ((actor->xe6_27_thermalVisorFlags & 1) != 0) {
            actor->Render(*this);
          }
        }
      }
      CGraphics::SetDepthRange(DEPTH_WORLD, DEPTH_FAR);
    }
    g_Renderer->DoThermalBlendCold();
    xf34_thermalFlag = EThermalDrawFlag::Hot;

    for (const auto& id : x86c_stateManagerContainer->xf370_) {
      if (auto* actor = static_cast<CActor*>(ObjectById(id))) {
        if ((actor->xe6_27_thermalVisorFlags & 2) != 0) {
          actor->Render(*this);
        }
      }
    }

    for (int i = areaCount - 1; i >= 0; --i) {
      const CGameArea& area = *areaArr[i];
      CPVSVisSet& pvs = pvsArr[i];

      g_Renderer->EnablePVS(pvs, area.x4_selfIdx);
      g_Renderer->DrawUnsortedGeometry(area.x4_selfIdx, mask, 0x20);
      g_Renderer->DrawAreaGeometry(area.x4_selfIdx, mask, 0x10);
    }

    ++x8dc_objectDrawToken;

    for (int i = 0; i < areaCount; ++i) {
      const CGameArea& area = *areaArr[i];
      CPVSVisSet& pvs = pvsArr[i];

      for (int j = 0; j < thermalActorCount; ++j) {
        CActor* actor = thermalActorArr[j];
        if (actor->GetAreaIdAlways() != area.x4_selfIdx) {
          if (actor->GetAreaIdAlways() != kInvalidAreaId || area.x4_selfIdx != visAreaId) {
            continue;
          }
        }
        actor->AddToRenderer(frustum, *this);
      }

      if (areaCount - 1 == i) {
        x884_actorModelParticles->AddStragglersToRenderer(*this);
        CDecalManager::AddToRenderer(frustum, *this);
        if (x84c_player) {
          x84c_player->AddToRenderer(frustum, *this);
        }
      }

      ++x8dc_objectDrawToken;

      g_Renderer->EnablePVS(pvs, area.x4_selfIdx);
      g_Renderer->DrawSortedGeometry(area.x4_selfIdx, mask, 0x10);
    }

    g_Renderer->PostRenderFogs();
  }

  x87c_fluidPlaneManager->EndFrame();

  g_Renderer->SetWorldFog(ERglFogMode::None, 0.f, 1.f, zeus::skBlack);

#if 0
    if (false) {
        CacheReflection();
    }
#endif

  if (x84c_player) {
    x84c_player->RenderGun(*this, x870_cameraManager->GetGlobalCameraTranslation(*this));
  }

  if (x86c_stateManagerContainer->xf39c_renderLast.size()) {
    CGraphics::SetDepthRange(DEPTH_SCREEN_ACTORS, DEPTH_GUN);
    for (const auto& id : x86c_stateManagerContainer->xf39c_renderLast) {
      if (auto* actor = static_cast<CActor*>(ObjectById(id))) {
        if (!thermal || actor->xe6_27_thermalVisorFlags & 0x2) {
          actor->Render(*this);
        }
      }
    }
    CGraphics::SetDepthRange(DEPTH_WORLD, DEPTH_FAR);
  }

  if (thermal) {
    g_Renderer->DoThermalBlendHot();
    g_Renderer->SetThermal(false, 0.f, zeus::skBlack);
    xf34_thermalFlag = EThermalDrawFlag::Bypass;
  }

  DrawDebugStuff();
  RenderCamerasAndAreaLights();
  ResetViewAfterDraw(backupViewport, backupViewMatrix);
  DrawE3DeathEffect();
  DrawAdditionalFilters();
}

void CStateManager::DrawActorCubeFaces(CActor& actor, int& cubeInst) const {
  if (!actor.m_reflectionCube ||
      (!TCastToPtr<CPlayer>(actor) && (!actor.GetActive() || !actor.IsDrawEnabled() || actor.xe4_30_outOfFrustum)))
    return;

  const TAreaId visAreaId = actor.GetAreaIdAlways();
  const SViewport backupVp = g_Viewport;

  int areaCount = 0;
  std::array<const CGameArea*, 10> areaArr;
  for (const CGameArea& area : *x850_world) {
    if (areaCount == 10) {
      break;
    }
    auto occState = CGameArea::EOcclusionState::Occluded;
    if (area.IsPostConstructed()) {
      occState = area.GetOcclusionState();
    }
    if (occState == CGameArea::EOcclusionState::Visible) {
      areaArr[areaCount++] = &area;
    }
  }

  for (int f = 0; f < 6; ++f) {
    SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(FMT_STRING("CStateManager::DrawActorCubeFaces [{}] {} {} {}"), f,
                                            actor.GetUniqueId(), actor.GetEditorId(), actor.GetName())
                                    .c_str(),
                                zeus::skOrange);
    CGraphics::g_BooMainCommandQueue->setRenderTarget(actor.m_reflectionCube, f);
    SetupViewForCubeFaceDraw(actor.GetRenderBounds().center(), f);
    CGraphics::g_BooMainCommandQueue->clearTarget();

    std::sort(areaArr.begin(), areaArr.begin() + areaCount, [visAreaId](const CGameArea* a, const CGameArea* b) {
      if (a->x4_selfIdx == b->x4_selfIdx) {
        return false;
      }
      if (visAreaId == a->x4_selfIdx) {
        return false;
      }
      if (visAreaId == b->x4_selfIdx) {
        return true;
      }
      return CGraphics::g_ViewPoint.dot(a->GetAABB().center()) > CGraphics::g_ViewPoint.dot(b->GetAABB().center());
    });

    int pvsCount = 0;
    std::array<CPVSVisSet, 10> pvsArr;
    for (auto area = areaArr.cbegin(); area != areaArr.cbegin() + areaCount; ++area) {
      const CGameArea* areaPtr = *area;
      CPVSVisSet& pvsSet = pvsArr[pvsCount++];
      pvsSet.Reset(EPVSVisSetState::OutOfBounds);
      GetVisSetForArea(areaPtr->x4_selfIdx, visAreaId, pvsSet);
    }

    for (int i = areaCount - 1; i >= 0; --i) {
      const CGameArea& area = *areaArr[i];
      SetupFogForArea(area);
      g_Renderer->EnablePVS(pvsArr[i], area.x4_selfIdx);
      g_Renderer->SetWorldLightFadeLevel(area.GetPostConstructed()->x1128_worldLightingLevel);
      g_Renderer->UpdateAreaUniforms(area.x4_selfIdx, EWorldShadowMode::None, true, cubeInst * 6 + f);
      g_Renderer->DrawUnsortedGeometry(area.x4_selfIdx, 0x2, 0x0);
    }

    if (!SetupFogForDraw()) {
      g_Renderer->SetWorldFog(ERglFogMode::None, 0.f, 1.f, zeus::skBlack);
    }

    x850_world->DrawSky(zeus::CTransform::Translate(CGraphics::g_ViewPoint));

    for (int i = 0; i < areaCount; ++i) {
      const CGameArea& area = *areaArr[i];
      CPVSVisSet& pvs = pvsArr[i];
      SetupFogForArea(area);
      g_Renderer->SetWorldLightFadeLevel(area.GetPostConstructed()->x1128_worldLightingLevel);
      g_Renderer->EnablePVS(pvs, area.x4_selfIdx);
      g_Renderer->DrawSortedGeometry(area.x4_selfIdx, 0x2, 0x0);
    }
  }

  CGraphics::g_BooMainCommandQueue->generateMipmaps(actor.m_reflectionCube);

  CBooRenderer::BindMainDrawTarget();
  g_Renderer->SetViewport(backupVp.x0_left, backupVp.x4_top, backupVp.x8_width, backupVp.xc_height);

  ++cubeInst;
}

void CStateManager::DrawWorldCubeFaces() const {
  size_t areaCount = 0;
  std::array<const CGameArea*, 10> areaArr;
  for (const CGameArea& area : *x850_world) {
    if (areaCount == areaArr.size()) {
      break;
    }
    auto occState = CGameArea::EOcclusionState::Occluded;
    if (area.IsPostConstructed()) {
      occState = area.GetOcclusionState();
    }
    if (occState == CGameArea::EOcclusionState::Visible) {
      areaArr[areaCount++] = &area;
    }
  }

  for (size_t ai = 0; ai < areaCount; ++ai) {
    const CGameArea& area = *areaArr[ai];
    int cubeInst = 0;
    for (CEntity* ent : *area.GetAreaObjects()) {
      if (const TCastToPtr<CActor> actor = ent) {
        DrawActorCubeFaces(*actor, cubeInst);
      }
    }
  }
}

void CStateManager::SetupFogForArea3XRange(TAreaId area) const {
  if (area == kInvalidAreaId) {
    area = x8cc_nextAreaId;
  }

  const CGameArea* areaObj = x850_world->GetAreaAlways(area);
  if (areaObj->IsPostConstructed()) {
    SetupFogForArea3XRange(*areaObj);
  }
}

void CStateManager::SetupFogForArea(TAreaId area) const {
  if (area == kInvalidAreaId) {
    area = x8cc_nextAreaId;
  }

  const CGameArea* areaObj = x850_world->GetAreaAlways(area);
  if (areaObj->IsPostConstructed()) {
    SetupFogForArea(*areaObj);
  }
}

void CStateManager::SetupFogForAreaNonCurrent(TAreaId area) const {
  if (area == kInvalidAreaId) {
    area = x8cc_nextAreaId;
  }

  const CGameArea* areaObj = x850_world->GetAreaAlways(area);
  if (areaObj->IsPostConstructed()) {
    SetupFogForAreaNonCurrent(*areaObj);
  }
}

void CStateManager::SetupFogForArea3XRange(const CGameArea& area) const {
  if (x8b8_playerState->GetActiveVisor(*this) != CPlayerState::EPlayerVisor::XRay) {
    return;
  }

  const float fogDist = area.GetXRayFogDistance();
  const float farz = (g_tweakGui->GetXRayFogNearZ() * (1.f - fogDist) + g_tweakGui->GetXRayFogFarZ() * fogDist) * 3.f;
  g_Renderer->SetWorldFog(ERglFogMode(g_tweakGui->GetXRayFogMode()), g_tweakGui->GetXRayFogNearZ(), farz,
                          g_tweakGui->GetXRayFogColor());
}

void CStateManager::SetupFogForArea(const CGameArea& area) const {
  if (SetupFogForDraw()) {
    return;
  }

  if (x8b8_playerState->GetActiveVisor(*this) == CPlayerState::EPlayerVisor::XRay) {
    const float fogDist = area.GetXRayFogDistance();
    const float farz = g_tweakGui->GetXRayFogNearZ() * (1.f - fogDist) + g_tweakGui->GetXRayFogFarZ() * fogDist;
    g_Renderer->SetWorldFog(ERglFogMode(g_tweakGui->GetXRayFogMode()), g_tweakGui->GetXRayFogNearZ(), farz,
                            g_tweakGui->GetXRayFogColor());
  } else {
    area.GetAreaFog()->SetCurrent();
  }
}

void CStateManager::SetupFogForAreaNonCurrent(const CGameArea& area) const {
  if (SetupFogForDraw()) {
    return;
  }

  if (x8b8_playerState->GetActiveVisor(*this) != CPlayerState::EPlayerVisor::XRay) {
    return;
  }

  const float fogDist = area.GetXRayFogDistance();
  const float farz = g_tweakGui->GetXRayFogNearZ() * (1.f - fogDist) + g_tweakGui->GetXRayFogFarZ() * fogDist;
  g_Renderer->SetWorldFog(ERglFogMode(g_tweakGui->GetXRayFogMode()), g_tweakGui->GetXRayFogNearZ(), farz,
                          g_tweakGui->GetXRayFogColor());
}

bool CStateManager::SetupFogForDraw() const {
  switch (x8b8_playerState->GetActiveVisor(*this)) {
  case CPlayerState::EPlayerVisor::Thermal:
    g_Renderer->SetWorldFog(ERglFogMode::None, 0.f, 1.f, zeus::skBlack);
    return true;
  case CPlayerState::EPlayerVisor::XRay:
  default:
    return false;
  case CPlayerState::EPlayerVisor::Combat:
  case CPlayerState::EPlayerVisor::Scan:
    auto& fog = x870_cameraManager->Fog();
    if (fog.IsFogDisabled()) {
      return false;
    }
    fog.SetCurrent();
    return true;
  }
}

void CStateManager::PreRender() {
  if (!xf94_24_readyToRender) {
    return;
  }

  SCOPED_GRAPHICS_DEBUG_GROUP("CStateManager::PreRender", zeus::skBlue);
  const zeus::CFrustum frustum = SetupDrawFrustum(g_Viewport);
  x86c_stateManagerContainer->xf370_.clear();
  x86c_stateManagerContainer->xf39c_renderLast.clear();
  xf7c_projectedShadow = nullptr;
  x850_world->PreRender();
  BuildDynamicLightListForWorld();
  for (const CGameArea& area : *x850_world) {
    auto occState = CGameArea::EOcclusionState::Occluded;
    if (area.IsPostConstructed()) {
      occState = area.GetOcclusionState();
    }
    if (occState == CGameArea::EOcclusionState::Visible) {
      for (CEntity* ent : *area.GetPostConstructed()->x10c0_areaObjs) {
        if (const TCastToPtr<CActor> act = ent) {
          if (act->IsDrawEnabled()) {
            act->CalculateRenderBounds();
            act->PreRender(*this, frustum);
          }
        }
      }
    }
  }

  CacheReflection();
  g_Renderer->PrepareDynamicLights(x8e0_dynamicLights);
}

void CStateManager::GetCharacterRenderMaskAndTarget(bool thawed, int& mask, int& target) const {
  switch (x8b8_playerState->GetActiveVisor(*this)) {
  case CPlayerState::EPlayerVisor::Combat:
  case CPlayerState::EPlayerVisor::Scan:
    mask = 0x1000;
    target = 0x0;
    break;
  case CPlayerState::EPlayerVisor::XRay:
    mask = 0x800;
    target = 0x0;
    break;
  case CPlayerState::EPlayerVisor::Thermal:
    if (thawed) {
      if (xf34_thermalFlag == EThermalDrawFlag::Hot) {
        mask = 0x600;
        target = 0x0;
      } else {
        mask = 0x600;
        target = 0x200;
      }
    } else {
      if (xf34_thermalFlag == EThermalDrawFlag::Cold) {
        mask = 0x500;
        target = 0x0;
      } else {
        mask = 0x500;
        target = 0x100;
      }
    }
    break;
  default:
    mask = 0x0;
    target = 0x0;
    break;
  }
}

bool CStateManager::GetVisSetForArea(TAreaId a, TAreaId b, CPVSVisSet& setOut) const {
  if (b == kInvalidAreaId) {
    return false;
  }

  const zeus::CVector3f viewPoint = CGraphics::g_ViewMatrix.origin;
  zeus::CVector3f closestDockPoint = viewPoint;
  bool hasClosestDock = false;
  if (a != b) {
    const CGameArea& area = *x850_world->GetGameAreas()[b];
    if (area.IsPostConstructed()) {
      for (const CGameArea::Dock& dock : area.GetDocks()) {
        for (int i = 0; i < dock.GetDockRefs().size(); ++i) {
          const TAreaId connArea = dock.GetConnectedAreaId(i);
          if (connArea == a) {
            const auto& verts = dock.GetPlaneVertices();
            const zeus::CVector3f dockCenter = (verts[0] + verts[1] + verts[2] + verts[3]) * 0.25f;
            if (hasClosestDock) {
              if ((dockCenter - viewPoint).magSquared() >= (closestDockPoint - viewPoint).magSquared()) {
                continue;
              }
            }
            closestDockPoint = dockCenter;
            hasClosestDock = true;
          }
        }
      }
    }
  } else {
    hasClosestDock = true;
  }

  if (hasClosestDock) {
    if (CPVSAreaSet* pvs = x850_world->GetGameAreas()[a]->GetPostConstructed()->xa0_pvs.get()) {
      const CPVSVisOctree& octree = pvs->GetVisOctree();
      const zeus::CVector3f closestDockLocal = x850_world->GetGameAreas()[a]->GetInverseTransform() * closestDockPoint;
      CPVSVisSet set;
      set.SetTestPoint(octree, closestDockLocal);

      if (set.GetState() == EPVSVisSetState::NodeFound) {
        setOut = set;
        return true;
      }
    }
  }

  return false;
}

void CStateManager::RecursiveDrawTree(TUniqueId node) {
  if (const TCastToPtr<CActor> actor = ObjectById(node)) {
    if (x8dc_objectDrawToken != actor->xc8_drawnToken) {
      if (actor->xc6_nextDrawNode != kInvalidUniqueId) {
        RecursiveDrawTree(actor->xc6_nextDrawNode);
      }
      if (x8dc_objectDrawToken == actor->xcc_addedToken) {
        actor->Render(*this);
      }
      actor->xc8_drawnToken = x8dc_objectDrawToken;
    }
  }
}

void CStateManager::SendScriptMsg(CEntity* dest, TUniqueId src, EScriptObjectMessage msg) {
  if (dest == nullptr || dest->x30_26_scriptingBlocked) {
    return;
  }

  if (sm_logScripting != nullptr && sm_logScripting->toBoolean()) {
    LogModule.report(logvisor::Info, FMT_STRING("Sending '{}' to '{}' id= {}"), ScriptObjectMessageToStr(msg),
                     dest->GetName(), dest->GetUniqueId());
  }

  dest->AcceptScriptMsg(msg, src, *this);
}

void CStateManager::SendScriptMsg(TUniqueId dest, TUniqueId src, EScriptObjectMessage msg) {
  CEntity* ent = ObjectById(dest);
  SendScriptMsg(ent, src, msg);
}

void CStateManager::SendScriptMsgAlways(TUniqueId dest, TUniqueId src, EScriptObjectMessage msg) {
  CEntity* dst = ObjectById(dest);
  if (dst == nullptr) {
    return;
  }

  if (sm_logScripting != nullptr && sm_logScripting->toBoolean()) {
    LogModule.report(logvisor::Info, FMT_STRING("Sending '{}' to '{}' id= {}"), ScriptObjectMessageToStr(msg),
                     dst->GetName(), dst->GetUniqueId());
  }

  dst->AcceptScriptMsg(msg, src, *this);
}

void CStateManager::SendScriptMsg(TUniqueId src, TEditorId dest, EScriptObjectMessage msg, EScriptObjectState state) {
  // CEntity* ent = GetObjectById(src);
  const auto search = GetIdListForScript(dest);
  if (search.first == x890_scriptIdMap.cend()) {
    return;
  }

  for (auto it = search.first; it != search.second; ++it) {
    const TUniqueId id = it->second;
    CEntity* dobj = GetAllObjectList().GetObjectById(id);
    SendScriptMsg(dobj, src, msg);
  }
}

void CStateManager::FreeScriptObjects(TAreaId aid) {
  for (const auto& p : x890_scriptIdMap) {
    if (p.first.AreaNum() == aid) {
      FreeScriptObject(p.second);
    }
  }

  std::set<TEditorId> freedObjects;
  for (auto it = x8a4_loadedScriptObjects.begin(); it != x8a4_loadedScriptObjects.end();) {
    if (it->first.AreaNum() == aid) {
      freedObjects.emplace(it->first);
      it = x8a4_loadedScriptObjects.erase(it);
      continue;
    }
    ++it;
  }

  const CGameArea* area = x850_world->GetGameAreas()[aid].get();
  if (area->IsPostConstructed()) {
    const CGameArea::CPostConstructed* pc = area->GetPostConstructed();
    for (CEntity* ent : *pc->x10c0_areaObjs) {
      if (ent != nullptr && !ent->IsInUse()) {
        FreeScriptObject(ent->GetUniqueId());
      }
    }
  }

  for (const auto& id : freedObjects) {
    m_incomingConnections.erase(id);
  }
}

void CStateManager::FreeScriptObject(TUniqueId id) {
  CEntity* ent = ObjectById(id);
  if (ent == nullptr || ent->IsInGraveyard()) {
    return;
  }

  ent->SetIsInGraveyard(true);
  x854_objectGraveyard.push_back(id);
  ent->AcceptScriptMsg(EScriptObjectMessage::Deleted, kInvalidUniqueId, *this);
  ent->SetIsScriptingBlocked(true);

  if (const TCastToPtr<CActor> act = ent) {
    x874_sortedListManager->Remove(act.GetPtr());
    act->SetUseInSortedLists(false);
  }

  if (sm_logScripting && sm_logScripting->toBoolean()) {
    LogModule.report(logvisor::Info, FMT_STRING("Removed '{}'"), ent->GetName());
  }
}

std::pair<const SScriptObjectStream*, TEditorId> CStateManager::GetBuildForScript(TEditorId id) const {
  const auto search = x8a4_loadedScriptObjects.find(id);
  if (search == x8a4_loadedScriptObjects.cend()) {
    return {nullptr, kInvalidEditorId};
  }
  return {&search->second, search->first};
}

TEditorId CStateManager::GetEditorIdForUniqueId(TUniqueId id) const {
  const CEntity* ent = GetObjectById(id);
  if (ent != nullptr) {
    return ent->GetEditorId();
  }
  return kInvalidEditorId;
}

TUniqueId CStateManager::GetIdForScript(TEditorId id) const {
  const auto search = x890_scriptIdMap.find(id);
  if (search == x890_scriptIdMap.cend()) {
    return kInvalidUniqueId;
  }
  return search->second;
}

std::pair<std::multimap<TEditorId, TUniqueId>::const_iterator, std::multimap<TEditorId, TUniqueId>::const_iterator>
CStateManager::GetIdListForScript(TEditorId id) const {
  auto ret = x890_scriptIdMap.equal_range(id);
  if (ret.first != x890_scriptIdMap.cend() && ret.first->first != id) {
    ret.first = x890_scriptIdMap.cend();
    ret.second = x890_scriptIdMap.cend();
  }
  return ret;
}

void CStateManager::LoadScriptObjects(TAreaId aid, CInputStream& in, std::vector<TEditorId>& idsOut) {
  in.readUByte();

  const u32 objCount = in.readUint32Big();
  idsOut.reserve(idsOut.size() + objCount);
  for (u32 i = 0; i < objCount; ++i) {
    const auto objType = static_cast<EScriptObjectType>(in.readUByte());
    const u32 objSize = in.readUint32Big();
    const u32 pos = static_cast<u32>(in.position());
    const auto id = LoadScriptObject(aid, objType, objSize, in);
    if (id.first == kInvalidEditorId) {
      continue;
    }

    const auto build = GetBuildForScript(id.first);
    if (build.first) {
      continue;
    }

    x8a4_loadedScriptObjects[id.first] = SScriptObjectStream{objType, pos, objSize};
    idsOut.push_back(id.first);
  }

  for (const auto& pair : m_incomingConnections) {
    if (auto* ent = ObjectById(GetIdForScript(pair.first))) {
      ent->SetIncomingConnectionList(&pair.second);
    }
  }
}

std::pair<TEditorId, TUniqueId> CStateManager::LoadScriptObject(TAreaId aid, EScriptObjectType type, u32 length,
                                                                CInputStream& in) {
  OPTICK_EVENT();
  const TEditorId id = in.readUint32Big();
  const u32 connCount = in.readUint32Big();
  length -= 8;
  std::vector<SConnection> conns;
  conns.reserve(connCount);
  for (u32 i = 0; i < connCount; ++i) {
    const auto state = EScriptObjectState(in.readUint32Big());
    const auto msg = EScriptObjectMessage(in.readUint32Big());
    const TEditorId target = in.readUint32Big();
    // Metaforce Addition
    if (m_incomingConnections.find(target) == m_incomingConnections.cend()) {
      m_incomingConnections.emplace(target, std::set<SConnection>());
    }
    SConnection inConn{state, msg, id};
    m_incomingConnections[target].emplace(inConn);
    // End Metaforce Addition
    length -= 12;
    conns.push_back(SConnection{state, msg, target});
  }
  const u32 propCount = in.readUint32Big();
  length -= 4;
  const auto startPos = in.position();

  bool error = false;
  FScriptLoader loader = {};
  if (type < EScriptObjectType::ScriptObjectTypeMAX && type >= EScriptObjectType::Actor) {
    loader = x90c_loaderFuncs[size_t(type)];
  }

  CEntity* ent = nullptr;
  if (loader != nullptr) {
    const CEntityInfo info(aid, std::move(conns), id);
    ent = loader(*this, in, propCount, info);
  } else {
    error = true;
  }

  if (ent != nullptr) {
    AddObject(ent);
  } else {
    error = true;
  }

  const u32 readAmt = in.position() - startPos;
  if (readAmt > length) {
    LogModule.report(logvisor::Fatal, FMT_STRING("Script object overread while reading {}"),
                     ScriptObjectTypeToStr(type));
  }

  const u32 leftover = length - readAmt;
  for (u32 i = 0; i < leftover; ++i) {
    in.readByte();
  }

  if (error || ent == nullptr) {
    in.seek(startPos, athena::SeekOrigin::Begin);
    const std::string name = HashInstanceName(in);
    in.seek(startPos + length, athena::SeekOrigin::Begin);
    LogModule.report(logvisor::Error, FMT_STRING("Script load error while loading {}, name: {}"),
                     ScriptObjectTypeToStr(type), name);
    return {kInvalidEditorId, kInvalidUniqueId};
  } else {
#ifndef NDEBUG
    LogModule.report(logvisor::Info, FMT_STRING("Loaded {} in area {}"), ent->GetName(), ent->GetAreaIdAlways());
#endif
    return {id, ent->GetUniqueId()};
  }
}

std::pair<TEditorId, TUniqueId> CStateManager::GenerateObject(TEditorId eid) {
  const std::pair<const SScriptObjectStream*, TEditorId> build = GetBuildForScript(eid);

  if (build.first) {
    const CGameArea* area = x850_world->GetArea(build.second.AreaNum());
    if (area->IsPostConstructed()) {
      const std::pair<const u8*, u32> buf = area->GetLayerScriptBuffer(build.second.LayerNum());
      CMemoryInStream stream(buf.first + build.first->x4_position, build.first->x8_length);
      auto ret = LoadScriptObject(build.second.AreaNum(), build.first->x0_type, build.first->x8_length, stream);
      // Metaforce Addition
      if (m_incomingConnections.find(eid) != m_incomingConnections.end()) {
        if (auto ent = ObjectById(ret.second)) {
          ent->SetIncomingConnectionList(&m_incomingConnections[eid]);
        }
      }
      // End Metaforce Addition
      return ret;
    }
  }

  return {kInvalidEditorId, kInvalidUniqueId};
}

void CStateManager::InitScriptObjects(const std::vector<TEditorId>& ids) {
  for (const auto& id : ids) {
    if (id == kInvalidEditorId) {
      continue;
    }

    const TUniqueId uid = GetIdForScript(id);
    SendScriptMsg(uid, kInvalidUniqueId, EScriptObjectMessage::InitializedInArea);
  }

  MurderScriptInstanceNames();
}

void CStateManager::InformListeners(const zeus::CVector3f& pos, EListenNoiseType type) {
  for (CEntity* ent : GetListeningAiObjectList()) {
    if (const TCastToPtr<CAi> ai = ent) {
      if (!ai->GetActive()) {
        continue;
      }

      CGameArea* area = x850_world->GetArea(ai->GetAreaIdAlways());
      CGameArea::EOcclusionState occState = CGameArea::EOcclusionState::Occluded;

      if (area->IsPostConstructed()) {
        occState = area->GetPostConstructed()->x10dc_occlusionState;
      }

      if (occState != CGameArea::EOcclusionState::Occluded) {
        ai->Listen(pos, type);
      }
    }
  }
}

void CStateManager::ApplyKnockBack(CActor& actor, const CDamageInfo& info, const CDamageVulnerability& vuln,
                                   const zeus::CVector3f& pos, float dampen) {
  if (vuln.GetVulnerability(info.GetWeaponMode(), false) == EVulnerability::Deflect) {
    return;
  }

  const CHealthInfo* hInfo = actor.HealthInfo(*this);
  if (hInfo == nullptr) {
    return;
  }

  const float dampedPower = (1.f - dampen) * info.GetKnockBackPower();
  if (const TCastToPtr<CPlayer> player = actor) {
    KnockBackPlayer(*player, pos, dampedPower, hInfo->GetKnockbackResistance());
    return;
  }

  const TCastToPtr<CAi> ai = actor;
  if (!ai && hInfo->GetHP() <= 0.f) {
    if (dampedPower > hInfo->GetKnockbackResistance()) {
      if (const TCastToPtr<CPhysicsActor> physActor = actor) {
        const zeus::CVector3f kbVec =
            pos * (dampedPower - hInfo->GetKnockbackResistance()) * physActor->GetMass() * 1.5f;
        if (physActor->GetMaterialList().HasMaterial(EMaterialTypes::Immovable) ||
            !physActor->GetMaterialList().HasMaterial(EMaterialTypes::Solid)) {
          return;
        }
        physActor->ApplyImpulseWR(kbVec, zeus::CAxisAngle());
        return;
      }
    }
  }

  if (ai) {
    ai->KnockBack(pos, *this, info, dampen == 0.f ? EKnockBackType::Direct : EKnockBackType::Radius, false,
                  dampedPower);
  }
}

void CStateManager::KnockBackPlayer(CPlayer& player, const zeus::CVector3f& pos, float power, float resistance) {
  if (player.GetMaterialList().HasMaterial(EMaterialTypes::Immovable)) {
    return;
  }

  float usePower;
  if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
    usePower = power * 1000.f;
    const auto surface =
        player.x2b0_outOfWaterTicks == 2 ? player.x2ac_surfaceRestraint : CPlayer::ESurfaceRestraints::Water;
    if (surface != CPlayer::ESurfaceRestraints::Normal &&
        player.GetOrbitState() == CPlayer::EPlayerOrbitState::NoOrbit) {
      usePower /= 7.f;
    }
  } else {
    usePower = power * 500.f;
  }

  const float minVel = player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed ? 35.f : 70.f;
  const float playerVel = player.x138_velocity.magnitude();
  const float maxVel = std::max(playerVel, minVel);
  const zeus::CVector3f negVel = -player.x138_velocity;
  usePower *= (1.f - 0.5f * zeus::CVector3f::getAngleDiff(pos, negVel) / M_PIF);
  player.ApplyImpulseWR(pos * usePower, zeus::CAxisAngle());
  player.UseCollisionImpulses();
  player.x2d4_accelerationChangeTimer = 0.25f;

  const float newVel = player.x138_velocity.magnitude();
  if (newVel > maxVel) {
    const zeus::CVector3f vel = (1.f / newVel) * player.x138_velocity * maxVel;
    player.SetVelocityWR(vel);
  }
}

void CStateManager::ApplyDamageToWorld(TUniqueId damager, const CActor& actor, const zeus::CVector3f& pos,
                                       const CDamageInfo& info, const CMaterialFilter& filter) {
  const zeus::CAABox aabb(pos - info.GetRadius(), pos + info.GetRadius());

  bool bomb = false;
  const TCastToConstPtr<CWeapon> weapon = actor;
  if (weapon) {
    bomb = True(weapon->GetAttribField() & (EProjectileAttrib::Bombs | EProjectileAttrib::PowerBombs));
  }

  rstl::reserved_vector<TUniqueId, 1024> nearList;
  BuildNearList(nearList, aabb, filter, &actor);
  for (const auto& id : nearList) {
    CEntity* ent = ObjectById(id);
    if (ent == nullptr) {
      continue;
    }

    const TCastToPtr<CPlayer> player = ent;
    if (bomb && player) {
      if (player->GetFrozenState()) {
        g_GameState->SystemOptions().IncrementFrozenBallCount();
        MP1::CSamusHud::DisplayHudMemo(u"", CHUDMemoParms{0.f, true, true, true});
        player->UnFreeze(*this);
      } else {
        if ((weapon->GetAttribField() & EProjectileAttrib::Bombs) == EProjectileAttrib::Bombs) {
          player->BombJump(pos, *this);
        }
      }
    } else if (ent->GetUniqueId() != damager) {
      TestBombHittingWater(actor, pos, static_cast<CActor&>(*ent));
      if (TestRayDamage(pos, static_cast<CActor&>(*ent), nearList)) {
        ApplyRadiusDamage(actor, pos, static_cast<CActor&>(*ent), info);
      }
    }

    if (const TCastToPtr<CWallCrawlerSwarm> swarm = ent) {
      swarm->ApplyRadiusDamage(pos, info, *this);
    }

    if (const TCastToPtr<CSnakeWeedSwarm> swarm = ent) {
      swarm->ApplyRadiusDamage(pos, info, *this);
    }
  }
}

void CStateManager::ProcessRadiusDamage(const CActor& damager, CActor& damagee, TUniqueId senderId,
                                        const CDamageInfo& info, const CMaterialFilter& filter) {
  const zeus::CAABox aabb(damager.GetTranslation() - info.GetRadius(), damager.GetTranslation() + info.GetRadius());
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  BuildNearList(nearList, aabb, filter, nullptr);
  for (const auto& id : nearList) {
    CEntity* ent = ObjectById(id);
    if (ent == nullptr || ent->GetUniqueId() == damager.GetUniqueId() || ent->GetUniqueId() == senderId ||
        ent->GetUniqueId() == damagee.GetUniqueId()) {
      continue;
    }

    TestBombHittingWater(damager, damager.GetTranslation(), static_cast<CActor&>(*ent));
    if (TestRayDamage(damager.GetTranslation(), static_cast<CActor&>(*ent), nearList)) {
      ApplyRadiusDamage(damager, damager.GetTranslation(), static_cast<CActor&>(*ent), info);
    }
  }
}

void CStateManager::ApplyRadiusDamage(const CActor& a1, const zeus::CVector3f& pos, CActor& a2,
                                      const CDamageInfo& info) {
  zeus::CVector3f delta = a2.GetTranslation() - pos;
  std::optional<zeus::CAABox> bounds;
  if (delta.magSquared() < info.GetRadius() * info.GetRadius() ||
      ((bounds = a2.GetTouchBounds()) &&
       CCollidableSphere::Sphere_AABox_Bool(zeus::CSphere{pos, info.GetRadius()}, *bounds))) {
    float rad = info.GetRadius();
    if (rad > FLT_EPSILON) {
      rad = delta.magnitude() / rad;
    } else {
      rad = 0.f;
    }
    if (rad > 0.f) {
      delta.normalize();
    }

    bool alive = false;
    if (const CHealthInfo* hInfo = a2.HealthInfo(*this)) {
      if (hInfo->GetHP() > 0.f) {
        alive = true;
      }
    }

    const CDamageVulnerability* vuln =
        rad > 0.f ? a2.GetDamageVulnerability(pos, delta, info) : a2.GetDamageVulnerability();

    if (vuln->WeaponHurts(info.GetWeaponMode(), true)) {
      const float dam = info.GetRadiusDamage(*vuln);
      if (dam > 0.f) {
        ApplyLocalDamage(pos, delta, a2, dam, info.GetWeaponMode());
      }
      a2.SendScriptMsgs(EScriptObjectState::Damage, *this, EScriptObjectMessage::None);
      SendScriptMsg(&a2, a1.GetUniqueId(), EScriptObjectMessage::Damage);
    } else {
      a2.SendScriptMsgs(EScriptObjectState::InvulnDamage, *this, EScriptObjectMessage::None);
      SendScriptMsg(&a2, a1.GetUniqueId(), EScriptObjectMessage::InvulnDamage);
    }

    if (alive && info.GetKnockBackPower() > 0.f) {
      ApplyKnockBack(a2, info, *vuln, (a2.GetTranslation() - a1.GetTranslation()).normalized(), rad);
    }
  }
}

bool CStateManager::TestRayDamage(const zeus::CVector3f& pos, const CActor& damagee,
                                  const rstl::reserved_vector<TUniqueId, 1024>& nearList) const {
  const CHealthInfo* hInfo = const_cast<CActor&>(damagee).HealthInfo(const_cast<CStateManager&>(*this));
  if (hInfo == nullptr) {
    return false;
  }

  static constexpr CMaterialList incList(EMaterialTypes::Solid);
  static constexpr CMaterialList exList(EMaterialTypes::ProjectilePassthrough, EMaterialTypes::Player,
                                        EMaterialTypes::Occluder, EMaterialTypes::Character);
  static constexpr CMaterialFilter filter(incList, exList, CMaterialFilter::EFilterType::IncludeExclude);

  const std::optional<zeus::CAABox> bounds = damagee.GetTouchBounds();
  if (!bounds) {
    return false;
  }

  const zeus::CVector3f center = bounds->center();
  zeus::CVector3f dir = center - pos;

  if (!dir.canBeNormalized()) {
    return true;
  }
  const float origMag = dir.magnitude();
  dir = dir * (1.f / origMag);

  if (RayCollideWorld(pos, center, nearList, filter, &damagee)) {
    return true;
  }

  const zeus::CMRay ray(pos, dir, origMag);
  if (!MultiRayCollideWorld(ray, filter)) {
    return false;
  }

  float depth;
  zeus::CVector3f norm;
  const u32 count = CollisionUtil::RayAABoxIntersection(ray, *bounds, norm, depth);
  if (count == 0 || count == 1) {
    return true;
  }

  return CGameCollision::RayDynamicIntersectionBool(*this, pos, dir, filter, nearList, &damagee, depth * origMag);
}

bool CStateManager::RayCollideWorld(const zeus::CVector3f& start, const zeus::CVector3f& end,
                                    const CMaterialFilter& filter, const CActor* damagee) const {
  zeus::CVector3f delta = end - start;
  const float mag = delta.magnitude();
  delta = delta / mag;
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  BuildNearList(nearList, start, delta, mag, filter, damagee);
  return RayCollideWorldInternal(start, end, filter, nearList, damagee);
}

bool CStateManager::RayCollideWorld(const zeus::CVector3f& start, const zeus::CVector3f& end,
                                    const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                    const CMaterialFilter& filter, const CActor* damagee) const {
  return RayCollideWorldInternal(start, end, filter, nearList, damagee);
}

bool CStateManager::RayCollideWorldInternal(const zeus::CVector3f& start, const zeus::CVector3f& end,
                                            const CMaterialFilter& filter,
                                            const rstl::reserved_vector<TUniqueId, 1024>& nearList,
                                            const CActor* damagee) const {
  const zeus::CVector3f delta = end - start;
  if (!delta.canBeNormalized()) {
    return true;
  }

  const float mag = delta.magnitude();
  const zeus::CVector3f dir = delta * (1.f / mag);
  if (!CGameCollision::RayStaticIntersectionBool(*this, start, dir, mag, filter)) {
    return false;
  }
  return CGameCollision::RayDynamicIntersectionBool(*this, start, dir, filter, nearList, damagee, mag);
}

bool CStateManager::MultiRayCollideWorld(const zeus::CMRay& ray, const CMaterialFilter& filter) const {
  zeus::CVector3f crossed = {-ray.dir.z() * ray.dir.z() - ray.dir.y() * ray.dir.x(),
                             ray.dir.x() * ray.dir.x() - ray.dir.z() * ray.dir.y(),
                             ray.dir.y() * ray.dir.y() - ray.dir.x() * -ray.dir.z()};

  crossed.normalize();
  const zeus::CVector3f crossed2 = ray.dir.cross(crossed) * 0.35355338f;
  const zeus::CVector3f negCrossed2 = -crossed2;
  const zeus::CVector3f rms = crossed * 0.35355338f;
  const zeus::CVector3f negRms = -rms;

  for (int i = 0; i < 4; ++i) {
    const zeus::CVector3f& useCrossed = (i & 2) != 0 ? negCrossed2 : crossed2;
    const zeus::CVector3f& useRms = (i & 1) != 0 ? rms : negRms;
    if (CGameCollision::RayStaticIntersectionBool(*this, ray.start + useCrossed + useRms, ray.dir, ray.length,
                                                  filter)) {
      return true;
    }
  }

  return false;
}

void CStateManager::TestBombHittingWater(const CActor& damager, const zeus::CVector3f& pos, CActor& damagee) {
  const TCastToConstPtr<CWeapon> wpn = damager;
  if (!wpn) {
    return;
  }

  if (False(wpn->GetAttribField() & (EProjectileAttrib::Bombs | EProjectileAttrib::PowerBombs))) {
    return;
  }

  const bool powerBomb = (wpn->GetAttribField() & EProjectileAttrib::PowerBombs) == EProjectileAttrib::PowerBombs;
  const TCastToPtr<CScriptWater> water = damagee;
  if (!water) {
    return;
  }

  const zeus::CAABox bounds = water->GetTriggerBoundsWR();
  const zeus::CVector3f hitPos(pos.x(), pos.y(), bounds.max.z());
  const float bombRad = powerBomb ? 4.f : 2.f;
  const float delta = bounds.max.z() - pos.dot(zeus::skUp);
  if (delta <= bombRad && delta > 0.f) {
    // Below surface
    const float rippleFactor = 1.f - delta / bombRad;
    if (x87c_fluidPlaneManager->GetLastRippleDeltaTime(damager.GetUniqueId()) >= 0.15f) {
      const float bombMag = powerBomb ? 1.f : 0.75f;
      const float mag = 0.6f * bombMag + 0.4f * bombMag * std::sin(2.f * M_PIF * rippleFactor * 0.25f);
      water->GetFluidPlane().AddRipple(mag, damager.GetUniqueId(), hitPos, *water, *this);
    }
    if (!powerBomb) {
      x87c_fluidPlaneManager->CreateSplash(damager.GetUniqueId(), *this, *water, hitPos, rippleFactor, true);
    }
  } else {
    // Above surface
    const float bombMag = powerBomb ? 2.f : 1.f;
    if (delta <= -bombMag || delta >= 0.f) {
      return;
    }
    const CRayCastResult res = RayStaticIntersection(pos, zeus::skDown, -delta, CMaterialFilter::skPassEverything);
    if (res.IsInvalid() && x87c_fluidPlaneManager->GetLastRippleDeltaTime(damager.GetUniqueId()) >= 0.15f) {
      // Not blocked by static geometry
      const float mag = 0.6f * bombMag + 0.4f * bombMag * std::sin(2.f * M_PIF * -delta / bombMag * 0.25f);
      water->GetFluidPlane().AddRipple(mag, damager.GetUniqueId(), hitPos, *water, *this);
    }
  }
}

bool CStateManager::ApplyLocalDamage(const zeus::CVector3f& pos, const zeus::CVector3f& dir, CActor& damagee, float dam,
                                     const CWeaponMode& weapMode) {
  CHealthInfo* hInfo = damagee.HealthInfo(*this);
  if (hInfo == nullptr || dam < 0.f) {
    return false;
  }

  if (hInfo->GetHP() <= 0.f) {
    return true;
  }

  float mulDam = dam;

  TCastToPtr<CPlayer> player = damagee;
  CAi* ai = TCastToPtr<CPatterned>(damagee).GetPtr();
  if (ai == nullptr) {
    ai = TCastToPtr<CDestroyableRock>(damagee).GetPtr();
  }

  if (player) {
    if (GetPlayerState()->CanTakeDamage()) {
      if (x870_cameraManager->IsInCinematicCamera() ||
          (weapMode.GetType() == EWeaponType::Phazon &&
           x8b8_playerState->HasPowerUp(CPlayerState::EItemType::PhazonSuit))) {
        return false;
      }

      if (g_GameState->GetHardMode()) {
        mulDam *= g_GameState->GetHardModeDamageMultiplier();
      }

      float damReduction = 0.f;
      if (x8b8_playerState->HasPowerUp(CPlayerState::EItemType::VariaSuit)) {
        damReduction = g_tweakPlayer->GetVariaDamageReduction();
      }
      if (x8b8_playerState->HasPowerUp(CPlayerState::EItemType::GravitySuit)) {
        damReduction = std::max(g_tweakPlayer->GetGravityDamageReduction(), damReduction);
      }
      if (x8b8_playerState->HasPowerUp(CPlayerState::EItemType::PhazonSuit)) {
        damReduction = std::max(g_tweakPlayer->GetPhazonDamageReduction(), damReduction);
      }

      mulDam = -(damReduction * mulDam - mulDam);
    } else {
      mulDam = 0.f;
    }
  }

  const float newHp = hInfo->GetHP() - mulDam;
  const bool significant = std::fabs(newHp - hInfo->GetHP()) >= 0.00001;
  hInfo->SetHP(newHp);

  if (player && GetPlayerState()->CanTakeDamage()) {
    player->TakeDamage(significant, pos, mulDam, weapMode.GetType(), *this);
    if (newHp <= 0.f) {
      x8b8_playerState->SetPlayerAlive(false);
    }
  }

  if (ai != nullptr) {
    if (significant) {
      ai->TakeDamage(dir, mulDam);
    }
    if (newHp <= 0.f) {
      ai->Death(*this, dir, EScriptObjectState::DeathRattle);
    }
  }

  return significant;
}

bool CStateManager::ApplyDamage(TUniqueId damagerId, TUniqueId damageeId, TUniqueId radiusSender,
                                const CDamageInfo& info, const CMaterialFilter& filter,
                                const zeus::CVector3f& knockbackVec) {
  CEntity* ent0 = ObjectById(damagerId);
  CEntity* ent1 = ObjectById(damageeId);
  const TCastToPtr<CActor> damager = ent0;
  const TCastToPtr<CActor> damagee = ent1;
  const bool isPlayer = TCastToPtr<CPlayer>(ent1);

  if (damagee) {
    if (CHealthInfo* hInfo = damagee->HealthInfo(*this)) {
      zeus::CVector3f position;
      zeus::CVector3f direction = zeus::skRight;
      const bool alive = hInfo->GetHP() > 0.f;
      if (damager) {
        position = damager->GetTranslation();
        direction = damager->GetTransform().basis[1];
      }

      const CDamageVulnerability* dVuln;
      if (damager || isPlayer) {
        dVuln = damagee->GetDamageVulnerability(position, direction, info);
      } else {
        dVuln = damagee->GetDamageVulnerability();
      }

      if (info.GetWeaponMode().GetType() == EWeaponType::None || dVuln->WeaponHurts(info.GetWeaponMode(), false)) {
        if (info.GetDamage() > 0.f) {
          ApplyLocalDamage(position, direction, *damagee, info.GetDamage(), info.GetWeaponMode());
        }
        damagee->SendScriptMsgs(EScriptObjectState::Damage, *this, EScriptObjectMessage::None);
        SendScriptMsg(damagee.GetPtr(), damagerId, EScriptObjectMessage::Damage);
      } else {
        damagee->SendScriptMsgs(EScriptObjectState::InvulnDamage, *this, EScriptObjectMessage::None);
        SendScriptMsg(damagee.GetPtr(), damagerId, EScriptObjectMessage::InvulnDamage);
      }

      if (alive && damager && info.GetKnockBackPower() > 0.f) {
        zeus::CVector3f delta =
            knockbackVec.isZero() ? (damagee->GetTranslation() - damager->GetTranslation()) : knockbackVec;
        delta.z() = FLT_EPSILON;
        ApplyKnockBack(*damagee, info, *dVuln, delta.normalized(), 0.f);
      }
    }

    if (damager && info.GetRadius() > 0.f) {
      ProcessRadiusDamage(*damager, *damagee, radiusSender, info, filter);
    }

    if (const TCastToPtr<CWallCrawlerSwarm> swarm = ent1) {
      if (damager) {
        swarm->ApplyRadiusDamage(damager->GetTranslation(), info, *this);
      }
    }
  }

  return false;
}

void CStateManager::UpdateAreaSounds() {
  rstl::reserved_vector<TAreaId, 10> areas;
  for (CGameArea& area : *x850_world) {
    auto occState = CGameArea::EOcclusionState::Occluded;
    if (area.IsPostConstructed()) {
      occState = area.GetOcclusionState();
    }
    if (occState == CGameArea::EOcclusionState::Visible) {
      areas.push_back(area.GetAreaId());
    }
  }
  CSfxManager::SetActiveAreas(areas);
}

void CStateManager::FrameEnd() { g_SimplePool->Flush(); }

void CStateManager::ProcessPlayerInput() {
  if (x84c_player) {
    x84c_player->ProcessInput(xb54_finalInput, *this);
  }
}

void CStateManager::SetGameState(EGameState state) {
  if (x904_gameState == state) {
    return;
  }

  if (x904_gameState == EGameState::SoftPaused) {
    x850_world->SetLoadPauseState(false);
  }

  switch (state) {
  case EGameState::Running:
    if (x88c_rumbleManager->IsDisabled()) {
      x88c_rumbleManager->SetDisabled(false);
    }
    break;
  case EGameState::SoftPaused:
    if (!x88c_rumbleManager->IsDisabled()) {
      x88c_rumbleManager->SetDisabled(true);
    }
    x850_world->SetLoadPauseState(true);
    break;
  default:
    break;
  }

  x904_gameState = state;
}

static const CFinalInput s_DisabledFinalInput = {};

void CStateManager::ProcessInput(const CFinalInput& input) {
  if (input.ControllerIdx() == 0) {
    const CGameCamera* cam = x870_cameraManager->GetCurrentCamera(*this);
    bool disableInput = cam->x170_25_disablesInput;
    if (x84c_player->x9c6_29_disableInput) {
      disableInput = true;
    }
    if (disableInput) {
      xb54_finalInput = s_DisabledFinalInput;
      xb54_finalInput.x0_dt = input.DeltaTime();
    } else {
      xb54_finalInput = input;
    }
  }
  x870_cameraManager->ProcessInput(input, *this);
}

void CStateManager::UpdateGraphicsTiming(float dt) {
  xf14_curTimeMod900 += dt;
  if (xf14_curTimeMod900 > 900.f) {
    xf14_curTimeMod900 -= 900.f;
  }
}

void CStateManager::Update(float dt) {
  MP1::CMain::UpdateDiscordPresence(GetWorld()->IGetStringTableAssetId());

  CElementGen::SetGlobalSeed(x8d8_updateFrameIdx);
  CParticleElectric::SetGlobalSeed(x8d8_updateFrameIdx);
  CDecal::SetGlobalSeed(x8d8_updateFrameIdx);
  CProjectileWeapon::SetGlobalSeed(x8d8_updateFrameIdx);

  xf08_pauseHudMessage = {};

  CScriptEffect::ResetParticleCounts();
  UpdateThermalVisor();
  UpdateGameState();

  const bool dying = x84c_player->x9f4_deathTime > 0.f;

  if (x904_gameState == EGameState::Running) {
    if (!TCastToPtr<CCinematicCamera>(x870_cameraManager->GetCurrentCamera(*this))) {
      g_GameState->SetTotalPlayTime(g_GameState->GetTotalPlayTime() + dt);
      UpdateHintState(dt);
    }

    for (size_t i = 0; i < numCameraPasses; ++i) {
      xb84_camFilterPasses[i].Update(dt);
      xd14_camBlurPasses[i].Update(dt);
    }
  }

  if (x904_gameState != EGameState::Paused) {
    PreThinkObjects(dt);
    x87c_fluidPlaneManager->Update(dt);
  }

  if (x904_gameState == EGameState::Running) {
    if (!dying) {
      CDecalManager::Update(dt, *this);
    }
    UpdateSortedLists();
    if (!dying) {
      MovePlatforms(dt);
      MoveActors(dt);
    }
    ProcessPlayerInput();
    if (x904_gameState != EGameState::SoftPaused) {
      CGameCollision::Move(*this, *x84c_player, dt, nullptr);
    }
    UpdateSortedLists();
    if (!dying) {
      CrossTouchActors();
    }
  } else {
    ProcessPlayerInput();
  }

  if (!dying && x904_gameState == EGameState::Running) {
    x884_actorModelParticles->Update(dt, *this);
  }

  if (x904_gameState == EGameState::Running || x904_gameState == EGameState::SoftPaused) {
    Think(dt);
  }

  if (x904_gameState != EGameState::SoftPaused) {
    x870_cameraManager->Update(dt, *this);
  }

  while (xf76_lastRelay != kInvalidUniqueId) {
    if (CEntity* ent = ObjectById(xf76_lastRelay)) {
      ent->Think(dt, *this);
    } else {
      xf76_lastRelay = kInvalidUniqueId;
      break;
    }
  }

  if (x904_gameState != EGameState::Paused) {
    PostUpdatePlayer(dt);
  }

  if (xf84_ == xf80_hudMessageFrameCount) {
    ShowPausedHUDMemo(xf88_, xf8c_);
    --xf84_;
    xf88_.Reset();
  }

  if (!dying && x904_gameState == EGameState::Running && !x870_cameraManager->IsInCinematicCamera()) {
    UpdateEscapeSequenceTimer(dt);
  }

  x850_world->Update(dt);
  x88c_rumbleManager->Update(dt);

  if (!dying) {
    x880_envFxManager->Update(dt, *this);
  }

  UpdateAreaSounds();

  xf94_24_readyToRender = true;

  if (xf94_27_inMapScreen) {
    if (const CHintOptions::SHintState* hint = g_GameState->HintOptions().GetCurrentDisplayedHint()) {
      if (hint->CanContinue()) {
        g_GameState->HintOptions().DismissDisplayedHint();
      }
    }
    xf94_27_inMapScreen = false;
  }

  if (!m_warping) {
    g_GameState->CurrentWorldState().SetAreaId(x8cc_nextAreaId);
    x850_world->TravelToArea(x8cc_nextAreaId, *this, false);
  }

  ClearGraveyard();
  ++x8d8_updateFrameIdx;
}

void CStateManager::UpdateGameState() {
  // Intentionally empty
}

void CStateManager::UpdateHintState(float dt) {
  CHintOptions& ho = g_GameState->HintOptions();
  ho.Update(dt, *this);
  u32 nextHintIdx = -1;
  u32 pageIdx = -1;
  if (const CHintOptions::SHintState* state = ho.GetCurrentDisplayedHint()) {
    const CGameHintInfo::CGameHint& next = g_MemoryCardSys->GetHints()[ho.GetNextHintIdx()];
    for (const CGameHintInfo::SHintLocation& loc : next.GetLocations()) {
      const auto& mwInfo = g_GameState->StateForWorld(loc.x0_mlvlId).MapWorldInfo();
      mwInfo->SetIsMapped(loc.x8_areaId, true);
    }
    if (state->x4_time < next.GetTextTime()) {
      nextHintIdx = ho.GetNextHintIdx();
      pageIdx = state->x4_time / 3.f;
    }
  }

  if (xeec_hintIdx != nextHintIdx || xef0_hintPeriods != pageIdx) {
    if (nextHintIdx == -1) {
      CHUDMemoParms memoInfo = {0.f, true, true, true};
      MP1::CSamusHud::DisplayHudMemo(u"", memoInfo);
    } else {
      const CGameHintInfo::CGameHint& data = g_MemoryCardSys->GetHints()[nextHintIdx];
      CHUDMemoParms memoInfo = {0.f, true, false, true};
      MP1::CSamusHud::DeferHintMemo(data.GetStringID(), pageIdx, memoInfo);
    }
    xeec_hintIdx = nextHintIdx;
    xef0_hintPeriods = pageIdx;
  }
}

void CStateManager::PreThinkObjects(float dt) {
  if (x84c_player->x9f4_deathTime > 0.f) {
    x84c_player->DoPreThink(dt, *this);
  } else if (x904_gameState == EGameState::SoftPaused) {
    for (CEntity* ent : GetAllObjectList()) {
      if (const TCastToPtr<CScriptEffect> effect = ent) {
        effect->PreThink(dt, *this);
      }
    }
  } else {
    for (CEntity* ent : GetAllObjectList()) {
      if (ent != nullptr && !GetCameraObjectList().GetObjectById(ent->GetUniqueId())) {
        ent->PreThink(dt, *this);
      }
    }
  }
}

void CStateManager::MovePlatforms(float dt) {
  for (CEntity* ent : GetPlatformAndDoorObjectList()) {
    if (ent == nullptr || !GetPlatformAndDoorObjectList().IsPlatform(*ent)) {
      continue;
    }

    auto& plat = static_cast<CScriptPlatform&>(*ent);
    if (!plat.GetActive() || plat.GetMass() == 0.f) {
      continue;
    }

    CGameCollision::Move(*this, plat, dt, nullptr);
  }
}

void CStateManager::MoveActors(float dt) {
  for (CEntity* ent : GetPhysicsActorObjectList()) {
    if (ent == nullptr || !ent->GetActive()) {
      continue;
    }

    auto& physActor = static_cast<CPhysicsActor&>(*ent);
    if (physActor.GetMass() == 0.f) {
      continue;
    }

    if (const TCastToPtr<CAi> ai = physActor) {
      bool doThink = !xf94_29_cinematicPause;
      if (doThink && ai->GetAreaIdAlways() != kInvalidAreaId) {
        const CGameArea* area = x850_world->GetAreaAlways(ai->GetAreaIdAlways());
        float occTime = 0.0f;
        if (area->IsPostConstructed()) {
          occTime = area->GetPostConstructed()->x10e4_occludedTime;
        }
        if (occTime > 5.f) {
          doThink = false;
        }
      }
      if (!doThink) {
        SendScriptMsgAlways(ai->GetUniqueId(), kInvalidUniqueId, EScriptObjectMessage::SuspendedMove);
        continue;
      }
    }

    if (x84c_player.get() != ent) {
      if (!GetPlatformAndDoorObjectList().IsPlatform(*ent)) {
        CGameCollision::Move(*this, physActor, dt, nullptr);
      }
    }
  }
}

void CStateManager::CrossTouchActors() {
  std::array<bool, 1024> visits{};
  rstl::reserved_vector<TUniqueId, 1024> nearList;

  for (CEntity* ent : GetActorObjectList()) {
    if (ent == nullptr) {
      continue;
    }

    auto& actor = static_cast<CActor&>(*ent);
    if (!actor.GetActive() || !actor.GetCallTouch()) {
      continue;
    }

    const std::optional<zeus::CAABox> touchAABB = actor.GetTouchBounds();
    if (!touchAABB) {
      continue;
    }

    CMaterialFilter filter = CMaterialFilter::skPassEverything;
    if (actor.GetMaterialList().HasMaterial(EMaterialTypes::Trigger)) {
      filter = CMaterialFilter::MakeExclude(EMaterialTypes::Trigger);
    }

    nearList.clear();
    BuildNearList(nearList, *touchAABB, filter, &actor);

    for (const auto& id : nearList) {
      auto* ent2 = static_cast<CActor*>(ObjectById(id));
      if (!ent2) {
        continue;
      }

      const std::optional<zeus::CAABox> touchAABB2 = ent2->GetTouchBounds();
      if (!ent2->GetActive() || !touchAABB2) {
        continue;
      }

      if (visits[ent2->GetUniqueId().Value()]) {
        continue;
      }

      if (touchAABB->intersects(*touchAABB2)) {
        actor.Touch(*ent2, *this);
        ent2->Touch(actor, *this);
      }

      visits[actor.GetUniqueId().Value()] = true;
    }
  }
}

void CStateManager::Think(float dt) {
  if (x84c_player->x9f4_deathTime > 0.f) {
    x84c_player->DoThink(dt, *this);
    return;
  }

  if (x904_gameState == EGameState::SoftPaused) {
    for (CEntity* ent : GetAllObjectList()) {
      if (const TCastToPtr<CScriptEffect> effect = ent) {
        effect->Think(dt, *this);
      }
    }
  } else {
    for (CEntity* ent : GetAllObjectList()) {
      if (const TCastToPtr<CAi> ai = ent) {
        bool doThink = !xf94_29_cinematicPause;
        if (doThink && ai->GetAreaIdAlways() != kInvalidAreaId) {
          const CGameArea* area = x850_world->GetAreaAlways(ai->GetAreaIdAlways());
          float occTime = 0.0f;
          if (area->IsPostConstructed()) {
            occTime = area->GetPostConstructed()->x10e4_occludedTime;
          }
          if (occTime > 5.f) {
            doThink = false;
          }
        }
        if (!doThink) {
          continue;
        }
      }
      if (!GetCameraObjectList().GetObjectById(ent->GetUniqueId())) {
        ent->Think(dt, *this);
      }
    }
  }
}

void CStateManager::PostUpdatePlayer(float dt) { x84c_player->PostUpdate(dt, *this); }

void CStateManager::ShowPausedHUDMemo(CAssetId strg, float time) {
  xf78_hudMessageTime = time;
  xf08_pauseHudMessage = strg;
  DeferStateTransition(EStateManagerTransition::MessageScreen);
}

void CStateManager::ClearGraveyard() {
  for (const auto& id : x854_objectGraveyard) {
    CEntity* ent = GetAllObjectList().GetValidObjectById(id);
    RemoveObject(id);
    std::default_delete<CEntity>()(ent);
  }
  x854_objectGraveyard.clear();
}

void CStateManager::FrameBegin(s32 frameCount) { x8d4_inputFrameIdx = frameCount; }

void CStateManager::InitializeState(CAssetId mlvlId, TAreaId aid, CAssetId mreaId) {
  const bool hadRandom = x900_activeRandom != nullptr;
  SetActiveRandomToDefault();

  if (xb3c_initPhase == EInitPhase::LoadWorld) {
    CreateStandardGameObjects();
    x850_world = std::make_unique<CWorld>(*g_SimplePool, *g_ResFactory, mlvlId);
    xb3c_initPhase = EInitPhase::LoadFirstArea;
  }

  if (xb3c_initPhase == EInitPhase::LoadFirstArea) {
    if (!x8f0_shadowTex.IsLoaded()) {
      return;
    }
    x8f0_shadowTex.GetObj();

    if (!x850_world->CheckWorldComplete(this, aid, mreaId)) {
      return;
    }
    x8cc_nextAreaId = x850_world->x68_curAreaId;
    CGameArea* area = x850_world->x18_areas[x8cc_nextAreaId].get();
    if (x850_world->ScheduleAreaToLoad(area, *this)) {
      area->StartStreamIn(*this);
      return;
    }
    xb3c_initPhase = EInitPhase::Done;
  }

  SetCurrentAreaId(x8cc_nextAreaId);
  g_GameState->CurrentWorldState().SetAreaId(x8cc_nextAreaId);
  x850_world->TravelToArea(x8cc_nextAreaId, *this, true);
  UpdateRoomAcoustics(x8cc_nextAreaId);

  for (CEntity* ent : GetAllObjectList()) {
    SendScriptMsg(ent, kInvalidUniqueId, EScriptObjectMessage::WorldInitialized);
  }

  for (CEntity* ent : GetAllObjectList()) {
    CScriptSpawnPoint* sp = TCastToPtr<CScriptSpawnPoint>(ent);
    if (sp != nullptr && sp->x30_24_active && sp->FirstSpawn()) {
      const zeus::CTransform& xf = sp->GetTransform();
      const zeus::CVector3f lookVec = xf.frontVector();
      if (lookVec.canBeNormalized()) {
        const auto lookXf = zeus::lookAt(xf.origin, xf.origin + lookVec);
        x84c_player->Teleport(lookXf, *this, true);
      }

      if (!g_GameState->x228_25_initPowerupsAtFirstSpawn) {
        break;
      }

      g_GameState->x228_25_initPowerupsAtFirstSpawn = false;
      for (int i = 0; i < int(CPlayerState::EItemType::Max); ++i) {
        const auto iType = CPlayerState::EItemType(i);

        u32 spawnPu = sp->GetPowerup(iType);
        u32 statePu = x8b8_playerState->GetItemAmount(iType);
        if (statePu < spawnPu) {
          x8b8_playerState->AddPowerUp(iType, spawnPu - statePu);
        }

        spawnPu = sp->GetPowerup(iType);
        statePu = x8b8_playerState->GetItemAmount(iType);
        if (statePu < spawnPu) {
          x8b8_playerState->IncrPickup(iType, spawnPu - statePu);
        }
      }
    }
  }

  x84c_player->AsyncLoadSuit(*this);
  x870_cameraManager->ResetCameras(*this);

  if (!hadRandom) {
    ClearActiveRandom();
  } else {
    SetActiveRandomToDefault();
  }

  x880_envFxManager->AsyncLoadResources(*this);
}

void CStateManager::CreateStandardGameObjects() {
  const float height = g_tweakPlayer->GetPlayerHeight();
  const float xyHe = g_tweakPlayer->GetPlayerXYHalfExtent();
  const float stepUp = g_tweakPlayer->GetStepUpHeight();
  const float stepDown = g_tweakPlayer->GetStepDownHeight();
  const float ballRadius = g_tweakPlayer->GetPlayerBallHalfExtent();
  const zeus::CAABox pBounds = {{-xyHe, -xyHe, 0.f}, {xyHe, xyHe, height}};
  const auto q = zeus::CQuaternion::fromAxisAngle(zeus::CVector3f{0.f, 0.f, 1.f}, zeus::degToRad(129.6f));

  x84c_player = std::make_unique<CPlayer>(
      AllocateUniqueId(), zeus::CTransform(q), pBounds, g_tweakPlayerRes->xc4_ballTransitionsANCS,
      zeus::CVector3f{1.65f, 1.65f, 1.65f}, 200.f, stepUp, stepDown, ballRadius,
      CMaterialList(EMaterialTypes::Player, EMaterialTypes::Solid, EMaterialTypes::GroundCollider));
  AddObject(*x84c_player);
  x870_cameraManager->CreateStandardCameras(*this);
}

CObjectList* CStateManager::ObjectListById(EGameObjectList type) {
  if (type == EGameObjectList::Invalid) {
    return nullptr;
  }
  return x808_objLists[int(type)].get();
}

const CObjectList* CStateManager::GetObjectListById(EGameObjectList type) const {
  if (type == EGameObjectList::Invalid) {
    return nullptr;
  }
  return x808_objLists[int(type)].get();
}

void CStateManager::RemoveObject(TUniqueId uid) {
  if (CEntity* ent = GetAllObjectList().GetValidObjectById(uid)) {
    if (ent->GetEditorId() != kInvalidEditorId) {
      const auto search = x890_scriptIdMap.equal_range(ent->GetEditorId());
      for (auto it = search.first; it != search.second;) {
        if (it->second == uid) {
          it = x890_scriptIdMap.erase(it);
          continue;
        }
        ++it;
      }
    }
    if (ent->GetAreaIdAlways() != kInvalidAreaId) {
      CGameArea* area = x850_world->GetArea(ent->GetAreaIdAlways());
      if (area->IsPostConstructed()) {
        area->GetAreaObjects()->RemoveObject(uid);
      }
    }
    if (const TCastToPtr<CActor> act = ent) {
      x874_sortedListManager->Remove(act.GetPtr());
    }
  }
  for (auto& list : x808_objLists) {
    list->RemoveObject(uid);
  }
}

void CStateManager::UpdateRoomAcoustics(TAreaId aid) {
  u32 updateCount = 0;
  std::array<CScriptRoomAcoustics*, 10> updates;
  for (CEntity* ent : GetAllObjectList()) {
    if (const TCastToPtr<CScriptRoomAcoustics> acoustics = ent) {
      if (acoustics->GetAreaIdAlways() != aid || !acoustics->GetActive()) {
        continue;
      }
      updates[updateCount++] = acoustics.GetPtr();
    }
    if (updateCount >= updates.size()) {
      break;
    }
  }

  if (updateCount == 0) {
    CScriptRoomAcoustics::DisableAuxCallbacks();
    return;
  }

  const auto idx = int(updateCount * x900_activeRandom->Float() * 0.99f);
  updates[idx]->EnableAuxCallbacks();
}

void CStateManager::SetCurrentAreaId(TAreaId aid) {
  if (aid != x8cc_nextAreaId) {
    x8d0_prevAreaId = x8cc_nextAreaId;
    UpdateRoomAcoustics(aid);
    x8cc_nextAreaId = aid;
  }

  if (aid == kInvalidAreaId) {
    return;
  }
  if (x8c0_mapWorldInfo->IsAreaVisited(aid)) {
    return;
  }
  x8c0_mapWorldInfo->SetAreaVisited(aid, true);
  x850_world->IGetMapWorld()->RecalculateWorldSphere(*x8c0_mapWorldInfo, *x850_world);
}

void CStateManager::AreaUnloaded(TAreaId) {
  // Intentionally empty
}

void CStateManager::PrepareAreaUnload(TAreaId aid) {
  for (CEntity* ent : GetAllObjectList()) {
    if (const TCastToPtr<CScriptDoor> door = ent) {
      if (door->IsConnectedToArea(*this, aid)) {
        door->ForceClosed(*this);
      }
    }
  }
  FreeScriptObjects(aid);
}

void CStateManager::AreaLoaded(TAreaId aid) {
  x8bc_mailbox->SendMsgs(aid, *this);
  x880_envFxManager->AreaLoaded();
}

void CStateManager::BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& listOut, const zeus::CVector3f& v1,
                                  const zeus::CVector3f& v2, float f1, const CMaterialFilter& filter,
                                  const CActor* actor) const {
  x874_sortedListManager->BuildNearList(listOut, v1, v2, f1, filter, actor);
}

void CStateManager::BuildColliderList(rstl::reserved_vector<TUniqueId, 1024>& listOut, const CActor& actor,
                                      const zeus::CAABox& aabb) const {
  x874_sortedListManager->BuildNearList(listOut, actor, aabb);
}

void CStateManager::BuildNearList(rstl::reserved_vector<TUniqueId, 1024>& listOut, const zeus::CAABox& aabb,
                                  const CMaterialFilter& filter, const CActor* actor) const {
  x874_sortedListManager->BuildNearList(listOut, aabb, filter, actor);
}

void CStateManager::UpdateActorInSortedLists(CActor& act) {
  if (!act.GetUseInSortedLists() || !act.xe4_27_notInSortedLists) {
    return;
  }

  const std::optional<zeus::CAABox> aabb = CalculateObjectBounds(act);
  const bool actorInLists = x874_sortedListManager->ActorInLists(&act);
  if (actorInLists || aabb) {
    act.xe4_27_notInSortedLists = false;
    if (actorInLists) {
      if (!act.GetActive() || !aabb) {
        x874_sortedListManager->Remove(&act);
      } else {
        x874_sortedListManager->Move(&act, *aabb);
      }
    } else if (act.GetActive() && aabb) {
      x874_sortedListManager->Insert(&act, *aabb);
    }
  }
}

void CStateManager::UpdateSortedLists() {
  if (!x850_world) {
    return;
  }

  for (CEntity* actor : GetActorObjectList()) {
    UpdateActorInSortedLists(static_cast<CActor&>(*actor));
  }
}

std::optional<zeus::CAABox> CStateManager::CalculateObjectBounds(const CActor& actor) {
  std::optional<zeus::CAABox> bounds = actor.GetTouchBounds();
  if (bounds) {
    zeus::CAABox aabb;
    aabb.accumulateBounds(bounds->min);
    aabb.accumulateBounds(bounds->max);
    if (TCastToConstPtr<CPhysicsActor> physAct = actor) {
      const zeus::CAABox physAabb = physAct->GetBoundingBox();
      aabb.accumulateBounds(physAabb.min);
      aabb.accumulateBounds(physAabb.max);
    }
    return aabb;
  } else {
    if (const TCastToConstPtr<CPhysicsActor> physAct = actor) {
      return physAct->GetBoundingBox();
    }
  }
  return std::nullopt;
}

void CStateManager::AddObject(CEntity& ent) {
  if (ent.GetEditorId() != kInvalidEditorId) {
    x890_scriptIdMap.insert(std::make_pair(ent.GetEditorId(), ent.GetUniqueId()));
  }
  for (auto& list : x808_objLists) {
    list->AddObject(ent);
  }

  if (ent.GetAreaIdAlways() == kInvalidAreaId && x84c_player && ent.GetUniqueId() != x84c_player->GetUniqueId()) {
    ent.x4_areaId = x84c_player->GetAreaIdAlways();
  }
  if (ent.GetAreaIdAlways() != kInvalidAreaId) {
    CGameArea* area = x850_world->GetArea(ent.GetAreaIdAlways());
    if (area->IsPostConstructed()) {
      area->GetAreaObjects()->AddObject(ent);
    }
  }

  if (const TCastToPtr<CActor> act = ent) {
    UpdateActorInSortedLists(*act.GetPtr());
  }

  ent.AcceptScriptMsg(EScriptObjectMessage::Registered, kInvalidUniqueId, *this);

  if (ent.GetAreaIdAlways() != kInvalidAreaId && x850_world) {
    CGameArea* area = x850_world->GetArea(ent.GetAreaIdAlways());
    if (area->IsValidated()) {
      SendScriptMsg(&ent, kInvalidUniqueId, EScriptObjectMessage::InitializedInArea);
    }
  }

  if (sm_logScripting != nullptr && sm_logScripting->toBoolean()) {
    LogModule.report(logvisor::Info, FMT_STRING("Added '{}'"), ent.GetName());
  }
}

void CStateManager::AddObject(CEntity* ent) {
  if (!ent) {
    return;
  }

  AddObject(*ent);
}

CRayCastResult CStateManager::RayStaticIntersection(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                                    float length, const CMaterialFilter& filter) const {
  return CGameCollision::RayStaticIntersection(*this, pos, dir, length, filter);
}

CRayCastResult CStateManager::RayWorldIntersection(TUniqueId& idOut, const zeus::CVector3f& pos,
                                                   const zeus::CVector3f& dir, float length,
                                                   const CMaterialFilter& filter,
                                                   const rstl::reserved_vector<TUniqueId, 1024>& list) const {
  return CGameCollision::RayWorldIntersection(*this, idOut, pos, dir, length, filter, list);
}

zeus::CVector3f CStateManager::Random2f(float scaleMin, float scaleMax) {
  zeus::CVector3f ret(x900_activeRandom->Float() - 0.5f, x900_activeRandom->Float() - 0.5f, 0.f);
  if (std::fabs(ret.x()) < 0.001f) {
    ret.x() = 0.001f;
  }
  ret.normalize();
  return ret * ((scaleMax - scaleMin) * x900_activeRandom->Float() + scaleMin);
}

void CStateManager::UpdateObjectInLists(CEntity& ent) {
  for (auto& list : x808_objLists) {
    if (list->GetValidObjectById(ent.GetUniqueId())) {
      if (!list->IsQualified(ent)) {
        list->RemoveObject(ent.GetUniqueId());
      }
    }
    if (!list->GetValidObjectById(ent.GetUniqueId())) {
      list->AddObject(ent);
    }
  }
}

TUniqueId CStateManager::AllocateUniqueId() {
  const s16 lastIndex = x0_nextFreeIndex;
  s16 ourIndex;
  do {
    ourIndex = x0_nextFreeIndex;
    x0_nextFreeIndex = (ourIndex + 1) & 0x3ff;
    if (x0_nextFreeIndex == lastIndex) {
      LogModule.report(logvisor::Fatal, FMT_STRING("Object list full!"));
    }
  } while (GetAllObjectList().GetObjectByIndex(ourIndex) != nullptr);

  x8_idArr[ourIndex] = (x8_idArr[ourIndex] + 1) & 0x3f;
  if (TUniqueId(ourIndex, x8_idArr[ourIndex]) == kInvalidUniqueId) {
    x8_idArr[ourIndex] = 0;
  }

  return TUniqueId(ourIndex, x8_idArr[ourIndex]);
}

void CStateManager::DeferStateTransition(EStateManagerTransition t) {
  if (t == EStateManagerTransition::InGame) {
    if (xf90_deferredTransition != EStateManagerTransition::InGame) {
      x850_world->SetLoadPauseState(false);
      xf90_deferredTransition = EStateManagerTransition::InGame;
    }
  } else {
    if (xf90_deferredTransition == EStateManagerTransition::InGame) {
      x850_world->SetLoadPauseState(true);
      xf90_deferredTransition = t;
    }
  }
}

bool CStateManager::CanShowMapScreen() const {
  const CHintOptions::SHintState* curDispHint = g_GameState->HintOptions().GetCurrentDisplayedHint();
  return curDispHint == nullptr || curDispHint->CanContinue();
}

std::pair<u32, u32> CStateManager::CalculateScanCompletionRate() const {
  u32 num = 0;
  u32 denom = 0;
  int idx = 0;
  for (const std::pair<CAssetId, float>& scan : x8b8_playerState->GetScanTimes()) {
    const auto category = g_MemoryCardSys->GetScanStates()[idx++].second;
    if (category != CSaveWorld::EScanCategory::None && category != CSaveWorld::EScanCategory::Research) {
      ++denom;
      if (scan.second == 1.f) {
        ++num;
      }
    }
  }
  return {num, denom};
}

void CStateManager::SetBossParams(TUniqueId bossId, float maxEnergy, u32 stringIdx) {
  xf18_bossId = bossId;
  xf1c_totalBossEnergy = maxEnergy;
  xf20_bossStringIdx = stringIdx - (g_Main->IsUSA() && !g_Main->IsTrilogy() ? 0 : 6);
}

float CStateManager::IntegrateVisorFog(float f) const {
  if (x8b8_playerState->GetActiveVisor(*this) == CPlayerState::EPlayerVisor::Scan) {
    return (1.f - x8b8_playerState->GetVisorTransitionFactor()) * f;
  }
  return f;
}

float CStateManager::g_EscapeShakeCountdown;
bool CStateManager::g_EscapeShakeCountdownInit = false;

void CStateManager::sub_80044098(const CCollisionResponseData& colRespData, const CRayCastResult& rayCast,
                                 TUniqueId uid, const CWeaponMode& weaponMode, u32 w1, u8 thermalFlags) {
  // TODO implement
}

const CGameArea* CStateManager::GetCurrentArea() const {
  if (x850_world != nullptr && x850_world->GetCurrentAreaId() != kInvalidAreaId) {
    return x850_world->GetAreaAlways(x850_world->GetCurrentAreaId());
  }
  return nullptr;
};

#ifdef PRIME2

int CStateManager::MaskUIdNumPlayers(TUniqueId uid) const {
  // Give a TUniqueId of a CPlayer object, returns the player index
  u16 uVar1 = uid.Value();
  if (uVar1 < numPlayers) {
    return uVar1;
  } else {
    return 0;
  }
}

#endif

} // namespace metaforce
