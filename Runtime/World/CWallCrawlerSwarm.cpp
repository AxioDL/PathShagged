#include "Runtime/World/CWallCrawlerSwarm.hpp"

#include <array>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/Character/CSteeringBehaviors.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Collision/CMaterialList.hpp"
#include "Runtime/Collision/CMetroidAreaCollider.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"
#include "Runtime/Graphics/CVertexMorphEffect.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CMarkerGrid.hpp"
#include "Runtime/World/CPhysicsActor.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptDoor.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

static CMaterialList MakeMaterialList() {
  return CMaterialList(EMaterialTypes::Scannable, EMaterialTypes::Trigger, EMaterialTypes::NonSolidDamageable,
                       EMaterialTypes::RadarObject);
}

CWallCrawlerSwarm::CWallCrawlerSwarm(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
                                     const zeus::CVector3f& boundingBoxExtent, const zeus::CTransform& xf,
                                     EFlavor flavor, const CAnimRes& animRes, s32 launchAnim, s32 attractAnim,
                                     CAssetId part1, CAssetId part2, CAssetId part3, CAssetId part4,
                                     const CDamageInfo& crabDamage, const CDamageInfo& scarabExplodeDamage,
                                     float crabDamageCooldown, float boidRadius, float touchRadius,
                                     float playerTouchRadius, u32 numBoids, u32 maxCreatedBoids,
                                     float animPlaybackSpeed, float separationRadius, float cohesionMagnitude,
                                     float alignmentWeight, float separationMagnitude, float moveToWaypointWeight,
                                     float attractionMagnitude, float attractionRadius, float boidGenRate,
                                     u32 maxLaunches, float scarabBoxMargin, float scarabScatterXYVelocity,
                                     float scarabTimeToExplode, const CHealthInfo& hInfo,
                                     const CDamageVulnerability& dVuln, s32 launchSfx,
                                     s32 scatterSfx, const CActorParameters& aParams)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), MakeMaterialList(), aParams, kInvalidUniqueId)
, x118_boundingBoxExtent(boundingBoxExtent)
, x13c_separationRadius(separationRadius)
, x140_cohesionMagnitude(cohesionMagnitude)
, x144_alignmentWeight(alignmentWeight)
, x148_separationMagnitude(separationMagnitude)
, x14c_moveToWaypointWeight(moveToWaypointWeight)
, x150_attractionMagnitude(attractionMagnitude)
, x154_attractionRadius(attractionRadius)
, x158_scarabScatterXYVelocity(scarabScatterXYVelocity)
, x15c_scarabTimeToExplode(scarabTimeToExplode)
, x160_animPlaybackSpeed(animPlaybackSpeed)
, x364_boidGenRate(boidGenRate)
, x370_crabDamageCooldown(crabDamageCooldown)
, x374_boidRadius(boidRadius)
, x378_touchRadius(touchRadius)
, x37c_scarabBoxMargin(scarabBoxMargin)
, x380_playerTouchRadius(playerTouchRadius)
, x384_crabDamage(crabDamage)
, x3a0_scarabExplodeDamage(scarabExplodeDamage)
, x3bc_hInfo(hInfo)
, x3c4_dVuln(dVuln)
, x548_numBoids(numBoids)
, x54c_maxCreatedBoids(maxCreatedBoids)
, x554_maxLaunches(maxLaunches)
, x558_flavor(flavor) {
  x168_partitionedBoidLists.resize(125);
  x55c_launchSfx = CSfxManager::TranslateSFXID(launchSfx != -1 ? u16(launchSfx) : u16(0xffff));
  x55e_scatterSfx = CSfxManager::TranslateSFXID(scatterSfx != -1 ? u16(scatterSfx) : u16(0xffff));
  CAnimRes attractAnimRes(animRes);
  attractAnimRes.SetCanLoop(true);
  attractAnimRes.SetDefaultAnim(attractAnim != -1 ? attractAnim : 0);
  CAnimRes launchAnimRes(animRes);
  launchAnimRes.SetCanLoop(true);
  launchAnimRes.SetDefaultAnim(launchAnim != -1 ? launchAnim : 0);
  x4b0_modelDatas.emplace_back(std::make_unique<CModelData>(animRes));
  x4b0_modelDatas.emplace_back(std::make_unique<CModelData>(animRes));
  x4b0_modelDatas.emplace_back(std::make_unique<CModelData>(animRes));
  x4b0_modelDatas.emplace_back(std::make_unique<CModelData>(animRes));
  x4b0_modelDatas.emplace_back(std::make_unique<CModelData>(attractAnimRes));
  x4b0_modelDatas.emplace_back(std::make_unique<CModelData>(attractAnimRes));
  x4b0_modelDatas.emplace_back(std::make_unique<CModelData>(attractAnimRes));
  x4b0_modelDatas.emplace_back(std::make_unique<CModelData>(attractAnimRes));
  x4b0_modelDatas.emplace_back(std::make_unique<CModelData>(launchAnimRes));
  x4b0_modelDatas.emplace_back(std::make_unique<CModelData>(animRes));
  if (aParams.GetXRayAssets().first.IsValid()) {
    for (size_t i = 0; i < 9; ++i) {
      x4b0_modelDatas[i]->SetXRayModel(aParams.GetXRayAssets());
    }
    x560_26_modelAssetDirty = true;
  }
  if (aParams.GetThermalAssets().first.IsValid()) {
    for (size_t i = 0; i < 9; ++i) {
      x4b0_modelDatas[i]->SetInfraModel(aParams.GetThermalAssets());
    }
    x560_26_modelAssetDirty = true;
  }
  if (part1.IsValid()) {
    x4f0_particleDescs.push_back(g_SimplePool->GetObj({FOURCC('PART'), part1}));
  }
  if (part2.IsValid()) {
    x4f0_particleDescs.push_back(g_SimplePool->GetObj({FOURCC('PART'), part2}));
  }
  if (part3.IsValid()) {
    x4f0_particleDescs.push_back(g_SimplePool->GetObj({FOURCC('PART'), part3}));
  }
  if (part4.IsValid()) {
    x4f0_particleDescs.push_back(g_SimplePool->GetObj({FOURCC('PART'), part4}));
  }
  for (const auto& t : x4f0_particleDescs) {
    x524_particleGens.emplace_back(new CElementGen(t));
    x524_particleGens.back()->SetParticleEmission(false);
  }
}

void CWallCrawlerSwarm::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CWallCrawlerSwarm::AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which) {
  //x430_.clear();
  for (size_t i = 0; i < 9; ++i) {
    //x430_.push_back(x4b0_[i]->PickAnimatedModel(which).Clone());
    x4b0_modelDatas[i]->EnableLooping(true);
    x4b0_modelDatas[i]->AdvanceAnimation(x4b0_modelDatas[i]->GetAnimationData()->GetAnimTimeRemaining("Whole Body"sv) *
                                             (float(i) * 0.0625f),
                                         mgr, x4_areaId, true);
  }
  //x430_.push_back(x4b0_.back()->PickAnimatedModel(which).Clone());
  x4dc_whichModel = which;
}

void CWallCrawlerSwarm::AddDoorRepulsors(CStateManager& mgr) {
  size_t doorCount = 0;
  for (const CEntity* ent : mgr.GetPhysicsActorObjectList()) {
    if (const TCastToConstPtr<CScriptDoor> door = ent) {
      if (door->GetAreaIdAlways() == x4_areaId) {
        ++doorCount;
      }
    }
  }

  x4e0_doorRepulsors.reserve(doorCount);

  for (const CEntity* ent : mgr.GetPhysicsActorObjectList()) {
    if (const TCastToConstPtr<CScriptDoor> door = ent) {
      if (door->GetAreaIdAlways() == x4_areaId) {
        if (const auto tb = door->GetTouchBounds()) {
          x4e0_doorRepulsors.emplace_back(tb->center(), (tb->min - tb->max).magnitude() * 0.75f);
        }
      }
    }
  }
}

void CWallCrawlerSwarm::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, sender, mgr);
  switch (msg) {
  case EScriptObjectMessage::Registered:
    x108_boids.reserve(size_t(x548_numBoids));
    for (int i = 0; i < x548_numBoids; ++i) {
      x108_boids.emplace_back(zeus::CTransform(), i);
    }
    AllocateSkinnedModels(mgr, CModelData::EWhichModel::Normal);
    AddDoorRepulsors(mgr);
    CreateShadow(false);
    break;
  default:
    break;
  }
}

void CWallCrawlerSwarm::UpdateParticles(float dt) {
  for (auto& p : x524_particleGens) {
    p->Update(dt);
  }
}

int CWallCrawlerSwarm::SelectLockOnIdx(CStateManager& mgr) const {
  const zeus::CTransform fpCamXf = mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform();
  if (x42c_lockOnIdx != -1) {
    const CBoid& b = x108_boids[x42c_lockOnIdx];
    if (b.GetActive()) {
      zeus::CVector3f dir = b.GetTranslation() - fpCamXf.origin;
      const float mag = dir.magnitude();
      dir = dir / mag;
      if (fpCamXf.basis[1].dot(dir) > 0.92388f) {
        if (mgr.RayStaticIntersection(fpCamXf.origin, dir, mag, CMaterialFilter::MakeInclude(EMaterialTypes::Solid))
                .IsInvalid()) {
          return x42c_lockOnIdx;
        }
      }
    }
    return -1;
  }

  int ret = -1;
  const float omtd = mgr.GetPlayer().GetOrbitMaxTargetDistance(mgr);
  const float omtdSq = omtd * omtd;
  float maxDot = 0.5f;
  for (size_t i = 0; i < x108_boids.size(); ++i) {
    const CBoid& b = x108_boids[i];
    if (b.GetActive()) {
      zeus::CVector3f delta = b.GetTranslation() - fpCamXf.origin;
      if (delta.magSquared() > omtdSq) {
        continue;
      }
      if (delta.canBeNormalized()) {
        const float thisDot = fpCamXf.basis[1].dot(delta.normalized());
        if (thisDot > maxDot) {
          ret = static_cast<int>(i);
          maxDot = thisDot;
        }
      }
    }
  }
  return ret;
}

zeus::CAABox CWallCrawlerSwarm::GetBoundingBox() const {
  const zeus::CVector3f he = x118_boundingBoxExtent * 0.75f;
  return zeus::CAABox(-he, he).getTransformedAABox(x34_transform);
}

TUniqueId CWallCrawlerSwarm::GetWaypointForState(EScriptObjectState state, CStateManager& mgr) const {
  for (const auto& c : GetConnectionList()) {
    if (c.x0_state == state && c.x4_msg == EScriptObjectMessage::Follow) {
      return mgr.GetIdForScript(c.x8_objId);
    }
  }
  return kInvalidUniqueId;
}

bool CWallCrawlerSwarm::PointOnSurface(const CCollisionSurface& surf, const zeus::CVector3f& pos,
                                       const zeus::CPlane& plane) const {
  const zeus::CVector3f projPt = ProjectPointToPlane(pos, surf.GetVert(0), plane.normal());
  for (int i = 0; i < 3; ++i) {
    if (plane.normal().dot((projPt - surf.GetVert(i)).cross(surf.GetVert((i + 2) % 3) - surf.GetVert(i))) < 0.f) {
      return false;
    }
  }
  return true;
}

bool CWallCrawlerSwarm::FindBestSurface(const CAreaCollisionCache& ccache, const zeus::CVector3f& pos, float radius,
                                        CCollisionSurface& out) const {
  bool ret = false;
  const float radiusSq = radius * radius;
  zeus::CSphere sphere(pos, radius);
  for (const auto& c : ccache) {
    for (const auto& n : c) {
      if (CCollidableSphere::Sphere_AABox_Bool(sphere, n.GetBoundingBox())) {
        const auto triList = n.GetTriangleArray();
        for (int i = 0; i < triList.GetSize(); ++i) {
          CCollisionSurface surf = n.GetOwner().GetMasterListTriangle(triList.GetAt(i));
          const zeus::CPlane plane = surf.GetPlane();
          const float distSq = std::fabs(plane.pointToPlaneDist(pos));
          if (distSq < radiusSq && PointOnSurface(surf, pos, plane)) {
            float dist = 0.f;
            if (distSq != 0.f) {
              dist = std::sqrt(distSq);
            }
            sphere.radius = dist;
            out = surf;
            ret = true;
          }
        }
      }
    }
  }
  return ret;
}

CCollisionSurface CWallCrawlerSwarm::FindBestCollisionInBox(CStateManager& mgr, const zeus::CVector3f& wpPos) const {
  CCollisionSurface ret(zeus::skRight, zeus::skForward, zeus::skUp, 0xffffffff);
  const zeus::CVector3f aabbExtents = GetBoundingBox().extents();
  float f25 = 0.1f;
  while (f25 < 1.f) {
    const zeus::CVector3f scaledExtents = aabbExtents * f25;
    CAreaCollisionCache ccache(zeus::CAABox(wpPos - scaledExtents, wpPos + scaledExtents));
    CGameCollision::BuildAreaCollisionCache(mgr, ccache);
    if (FindBestSurface(ccache, wpPos, 2.f * scaledExtents.magnitude(), ret)) {
      return ret;
    }
    f25 += 0.1f;
  }
  return ret;
}

static zeus::CTransform LookAt(const zeus::CUnitVector3f& a, const zeus::CUnitVector3f& b, const zeus::CRelAngle& ang) {
  const float dot = a.dot(b);
  if (zeus::close_enough(dot, 1.f)) {
    return zeus::CTransform();
  }
  if (dot > -0.99981f) {
    return zeus::CQuaternion::clampedRotateTo(a, b, ang).toTransform();
  }
  if (a != zeus::skRight && b != zeus::skRight) {
    return zeus::CQuaternion::fromAxisAngle(a.cross(zeus::skRight), ang).toTransform();
  }
  return zeus::CQuaternion::fromAxisAngle(a.cross(zeus::skUp), ang).toTransform();
}

void CWallCrawlerSwarm::CreateBoid(CStateManager& mgr, int idx) {
  //zeus::CAABox aabb = GetBoundingBox();
  const TUniqueId wpId = GetWaypointForState(EScriptObjectState::Patrol, mgr);
  if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(wpId)) {
    const CCollisionSurface surf = FindBestCollisionInBox(mgr, wp->GetTranslation());
    x108_boids[idx].Transform() =
        zeus::CTransform::Translate(ProjectPointToPlane(wp->GetTranslation(), surf.GetVert(0), surf.GetNormal()) +
                                    surf.GetNormal() * x374_boidRadius);
    if (zeus::close_enough(zeus::skUp.dot(surf.GetNormal()), -1.f)) {
      x108_boids[idx].Transform().setRotation(
          zeus::CTransform(zeus::skRight, zeus::skBack, zeus::skDown, zeus::skZero3f));
    } else {
      x108_boids[idx].Transform().setRotation(LookAt(zeus::skUp, surf.GetNormal(), M_PIF));
    }
    x108_boids[idx].x80_24_active = true;
    x108_boids[idx].x30_velocity = zeus::skZero3f;
    x108_boids[idx].x3c_targetWaypoint = wpId;
    x108_boids[idx].x7c_framesNotOnSurface = 0;
    x108_boids[idx].x48_timeToDie = 0.f;
    x108_boids[idx].x80_27_scarabExplodeTimerEnabled = false;
    x108_boids[idx].x78_health = x3bc_hInfo.GetHP();
  }
}

void CWallCrawlerSwarm::ExplodeBoid(CBoid& boid, CStateManager& mgr) {
  KillBoid(boid, mgr, 0.f, 1.f);
  mgr.ApplyDamageToWorld(GetUniqueId(), *this, boid.GetTranslation(), x3a0_scarabExplodeDamage,
                         CMaterialFilter::MakeInclude({EMaterialTypes::Player}));
}

void CWallCrawlerSwarm::SetExplodeTimers(const zeus::CVector3f& pos, float radius, float minTime, float maxTime) {
  const float radiusSq = radius * radius;
  const float range = maxTime - minTime;

  for (auto& b : x108_boids) {
    if (b.GetActive() && b.x48_timeToDie <= 0.f) {
      const float dist = (b.GetTranslation() - pos).magSquared();
      if (dist < radiusSq) {
        const float fac = dist / radiusSq * range + minTime;
        if (b.x4c_timeToExplode > fac || b.x4c_timeToExplode == 0.f) {
          b.x4c_timeToExplode = fac;
        }
      }
    }
  }
}

CWallCrawlerSwarm::CBoid* CWallCrawlerSwarm::GetListAt(const zeus::CVector3f& pos) {
  const zeus::CAABox aabb = GetBoundingBox();
  const zeus::CVector3f ints = (pos - aabb.min) / ((aabb.max - aabb.min) / 5.f);
  const int idx = int(ints.x()) + int(ints.y()) * 5 + int(ints.z()) * 25;
  if (idx < 0 || idx >= 125) {
    return x360_outlierBoidList;
  }
  return x168_partitionedBoidLists[idx];
}

void CWallCrawlerSwarm::BuildBoidNearList(const CBoid& boid, float radius,
                                          rstl::reserved_vector<CBoid*, 50>& nearList) {
  CBoid* b = GetListAt(boid.GetTranslation());
  while (b && nearList.size() < 50) {
    const float distSq = (b->GetTranslation() - boid.GetTranslation()).magSquared();
    if (distSq != 0.f && distSq < radius) {
      nearList.push_back(b);
    }
    b = b->x44_next;
  }
}

void CWallCrawlerSwarm::ApplySeparation(const CBoid& boid, const rstl::reserved_vector<CBoid*, 50>& nearList,
                                        zeus::CVector3f& aheadVec) const {
  if (nearList.empty()) {
    return;
  }

  float minDist = FLT_MAX;
  zeus::CVector3f pos;
  for (const CBoid* b : nearList) {
    const float dist = (boid.GetTranslation() - b->GetTranslation()).magSquared();
    if (dist != 0.f && dist < minDist) {
      minDist = dist;
      pos = b->GetTranslation();
    }
  }

  ApplySeparation(boid, pos, x13c_separationRadius, x148_separationMagnitude, aheadVec);
}

void CWallCrawlerSwarm::ApplySeparation(const CBoid& boid, const zeus::CVector3f& separateFrom, float separationRadius,
                                        float separationMagnitude, zeus::CVector3f& aheadVec) const {
  const zeus::CVector3f delta = boid.GetTranslation() - separateFrom;
  if (!delta.canBeNormalized()) {
    return;
  }

  const float deltaDistSq = delta.magSquared();
  const float capDeltaDistSq = separationRadius * separationRadius;
  if (deltaDistSq < capDeltaDistSq) {
    aheadVec += (1.f - deltaDistSq / capDeltaDistSq) * delta.normalized() * separationMagnitude;
  }
}

void CWallCrawlerSwarm::ScatterScarabBoid(CBoid& boid, CStateManager& mgr) const {
  const zeus::CVector3f oldDir = boid.GetTransform().basis[1];
  boid.Transform().setRotation(zeus::CTransform());
  boid.Transform() = LookAt(boid.GetTransform().basis[1], oldDir, M_PIF).multiplyIgnoreTranslation(boid.GetTransform());
  boid.x30_velocity = zeus::skZero3f;
  const float angle = mgr.GetActiveRandom()->Float() * (2.f * M_PIF);
  const float mag = mgr.GetActiveRandom()->Float() * x158_scarabScatterXYVelocity;
  boid.x30_velocity.x() = mag * std::cos(angle);
  boid.x30_velocity.y() = mag * std::sin(angle);
  boid.x80_26_launched = true;
  boid.x7c_remainingLaunchNotOnSurfaceFrames = 5;
  CSfxManager::AddEmitter(x55c_launchSfx, boid.GetTranslation(), zeus::skZero3f, true, false, 0x7f, x4_areaId);
}

void CWallCrawlerSwarm::MoveToWayPoint(CBoid& boid, CStateManager& mgr, zeus::CVector3f& aheadVec) const {
  if (const TCastToConstPtr<CScriptWaypoint> wp = mgr.ObjectById(boid.x3c_targetWaypoint)) {
    const CScriptWaypoint* useWp = wp.GetPtr();
    if ((useWp->GetTranslation() - boid.GetTranslation()).magSquared() <
        x164_waypointGoalRadius * x164_waypointGoalRadius) {
      boid.x3c_targetWaypoint = useWp->NextWaypoint(mgr);
      if (boid.x3c_targetWaypoint == kInvalidUniqueId) {
        if (x558_flavor == EFlavor::Scarab) {
          ScatterScarabBoid(boid, mgr);
        } else {
          boid.x80_24_active = false;
          return;
        }
      } else {
        useWp = TCastToConstPtr<CScriptWaypoint>(mgr.ObjectById(boid.x3c_targetWaypoint)).GetPtr();
      }
    }
    aheadVec += (useWp->GetTranslation() - boid.GetTranslation()).normalized() * x14c_moveToWaypointWeight;
  }
}

void CWallCrawlerSwarm::ApplyCohesion(const CBoid& boid, const rstl::reserved_vector<CBoid*, 50>& nearList,
                                      zeus::CVector3f& aheadVec) const {
  if (nearList.empty()) {
    return;
  }

  zeus::CVector3f avg;
  for (const CBoid* b : nearList) {
    avg += b->GetTranslation();
  }

  avg = avg / float(nearList.size());
  ApplyCohesion(boid, avg, x13c_separationRadius, x140_cohesionMagnitude, aheadVec);
}

void CWallCrawlerSwarm::ApplyCohesion(const CBoid& boid, const zeus::CVector3f& cohesionFrom, float cohesionRadius,
                                      float cohesionMagnitude, zeus::CVector3f& aheadVec) const {
  const zeus::CVector3f delta = cohesionFrom - boid.GetTranslation();
  if (!delta.canBeNormalized()) {
    return;
  }

  const float distSq = delta.magSquared();
  const float capDistSq = cohesionRadius * cohesionRadius;
  aheadVec += ((distSq > capDistSq) ? 1.f : distSq / capDistSq) * delta.normalized() * cohesionMagnitude;
}

void CWallCrawlerSwarm::ApplyAlignment(const CBoid& boid, const rstl::reserved_vector<CBoid*, 50>& nearList,
                                       zeus::CVector3f& aheadVec) const {
  if (nearList.empty()) {
    return;
  }

  zeus::CVector3f avg;
  for (const CBoid* b : nearList) {
    avg += b->GetTransform().basis[1];
  }

  avg = avg / float(nearList.size());
  aheadVec += zeus::CVector3f::getAngleDiff(boid.GetTransform().basis[1], avg) / M_PIF * (avg * x144_alignmentWeight);
}

void CWallCrawlerSwarm::ApplyAttraction(const CBoid& boid, const zeus::CVector3f& attractTo, float attractionRadius,
                                        float attractionMagnitude, zeus::CVector3f& aheadVec) const {
  const zeus::CVector3f delta = attractTo - boid.GetTranslation();
  if (!delta.canBeNormalized()) {
    return;
  }

  const float distSq = delta.magSquared();
  const float capDistSq = attractionRadius * attractionRadius;
  aheadVec += ((distSq > capDistSq) ? 0.f : (1.f - distSq / capDistSq)) * delta.normalized() * attractionMagnitude;
}

void CWallCrawlerSwarm::UpdateBoid(const CAreaCollisionCache& ccache, CStateManager& mgr, float dt, CBoid& boid) {
  if (boid.x80_27_scarabExplodeTimerEnabled) {
    if (x558_flavor == EFlavor::Scarab && boid.x4c_timeToExplode > 0.f) {
      boid.x4c_timeToExplode -= 2.f * dt;
      if (boid.x4c_timeToExplode <= 0.f) {
        ExplodeBoid(boid, mgr);
      }
    }
  } else if (boid.x80_26_launched) {
    const float radius = 2.f * x374_boidRadius;
    const float boidMag = boid.x30_velocity.magnitude();
    float f20 = boidMag * dt;
    const zeus::CVector3f f25 = (-boid.x30_velocity / boidMag) * x374_boidRadius;
    zeus::CVector3f f28 = boid.GetTranslation();
    bool found = false;
    while (f20 >= 0.f && !found) {
      CCollisionSurface surf(zeus::skRight, zeus::skForward, zeus::skUp, 0xffffffff);
      if (FindBestSurface(ccache, boid.x30_velocity * dt * 1.5f + f28, radius, surf) &&
          boid.x7c_remainingLaunchNotOnSurfaceFrames == 0) {
        if (x558_flavor != EFlavor::Scarab) {
          boid.Transform() = LookAt(boid.GetTransform().basis[2], surf.GetNormal(), M_PIF)
                                 .multiplyIgnoreTranslation(boid.GetTransform());
        }
        const auto plane = surf.GetPlane();
        boid.Translation() +=
            -(plane.pointToPlaneDist(boid.GetTranslation()) - x374_boidRadius - 0.01f) * plane.normal();
        boid.x7c_framesNotOnSurface = 0;
        boid.x80_26_launched = false;
        if (x558_flavor == EFlavor::Scarab) {
          boid.x80_27_scarabExplodeTimerEnabled = true;
          boid.x4c_timeToExplode = x15c_scarabTimeToExplode;
          CSfxManager::AddEmitter(x55e_scatterSfx, boid.GetTranslation(), zeus::skZero3f, true, false, 0x7f, x4_areaId);
        }
        found = true;
      }
      f20 -= x374_boidRadius;
      f28 += f25;
    }
    if (!found) {
      boid.x30_velocity += zeus::CVector3f(0.f, 0.f,
                                           -(x558_flavor == EFlavor::Scarab ? 3.f * CPhysicsActor::GravityConstant()
                                                                            : CPhysicsActor::GravityConstant())) *
                           dt;
      if (boid.x7c_remainingLaunchNotOnSurfaceFrames) {
        boid.x7c_remainingLaunchNotOnSurfaceFrames -= 1;
      }
    }
  } else if (boid.x7c_framesNotOnSurface >= 30) {
    boid.x80_24_active = false;
  } else {
    const float radius = 2.f * x374_boidRadius;
    bool found = false;
    CCollisionSurface surf(zeus::skRight, zeus::skForward, zeus::skUp, 0xffffffff);
    if (FindBestSurface(ccache, boid.GetTranslation() + boid.x30_velocity * dt * 1.5f, radius, surf)) {
      boid.x50_surface = surf;
      boid.Transform() = LookAt(boid.GetTransform().basis[2], surf.GetNormal(), zeus::degToRad(180.f * dt))
                             .multiplyIgnoreTranslation(boid.GetTransform());
      const auto plane = surf.GetPlane();
      const float dist = plane.pointToPlaneDist(boid.GetTranslation());
      if (dist <= 1.5f * x374_boidRadius) {
        boid.Translation() += -(dist - x374_boidRadius - 0.01f) * plane.normal();
        boid.x7c_framesNotOnSurface = 0;
        found = true;
      }
    }
    if (!found) {
      boid.Transform() = LookAt(boid.GetTransform().basis[2], boid.GetTransform().basis[1],
                                boid.x30_velocity.magnitude() / x374_boidRadius * dt)
                             .multiplyIgnoreTranslation(boid.GetTransform());
      boid.x7c_framesNotOnSurface += 1;
    }
    rstl::reserved_vector<CBoid*, 50> nearList;
    BuildBoidNearList(boid, x13c_separationRadius, nearList);
    zeus::CVector3f aheadVec = boid.GetTransform().basis[1] * 0.3f;
    for (int r26 = 0; r26 < 8; ++r26) {
      switch (r26) {
      case 0:
        for (const auto& rep : x4e0_doorRepulsors) {
          if ((rep.x0_center - boid.GetTranslation()).magSquared() < rep.xc_mag * rep.xc_mag) {
            ApplySeparation(boid, rep.x0_center, rep.xc_mag, 4.5f, aheadVec);
          }
        }
        break;
      case 4:
        ApplySeparation(boid, nearList, aheadVec);
        break;
      case 5:
        MoveToWayPoint(boid, mgr, aheadVec);
        break;
      case 6:
        ApplyCohesion(boid, nearList, aheadVec);
        break;
      case 7:
        ApplyAlignment(boid, nearList, aheadVec);
        break;
      case 3:
        ApplyAttraction(boid, mgr.GetPlayer().GetTranslation(), x154_attractionRadius,
                        x150_attractionMagnitude, aheadVec);
        break;
      default:
        break;
      }
      if (aheadVec.magSquared() >= 9.f) {
        break;
      }
    }
    boid.Transform() = LookAt(boid.GetTransform().basis[1],
                              ProjectVectorToPlane(aheadVec, boid.GetTransform().basis[2]).normalized(), M_PIF * dt)
                           .multiplyIgnoreTranslation(boid.GetTransform());
  }
}

void CWallCrawlerSwarm::LaunchBoid(CBoid& boid, const zeus::CVector3f& dir) {
  const zeus::CVector3f pos = boid.GetTranslation();
  static float skAttackTime = std::sqrt(2.5f / CPhysicsActor::GravityConstant()) * 2.f;
  static float skAttackVelocity = 15.f / skAttackTime;
  zeus::CVector3f deltaFlat = dir - pos;
  const float deltaZ = deltaFlat.z();
  deltaFlat.z() = 0.f;
  const float deltaMag = deltaFlat.magnitude();
  boid.Transform().setRotation(zeus::CTransform());
  boid.Transform() = LookAt(boid.GetTransform().basis[1], deltaFlat.normalized(), M_PIF)
                         .multiplyIgnoreTranslation(boid.GetTransform());
  zeus::CVector3f vec(skAttackVelocity * boid.GetTransform().basis[1].toVec2f(), 0.5f * skAttackVelocity);
  if (deltaMag > FLT_EPSILON) {
    deltaFlat = deltaFlat / deltaMag;
    const float dot = deltaFlat.dot(vec);
    if (dot > FLT_EPSILON) {
      const bool r29 = deltaZ < 0.f;
      float _12c, _130;
      float f25 = 0.f;
      if (CSteeringBehaviors::SolveQuadratic(-CPhysicsActor::GravityConstant(), vec.z(), -deltaZ, _12c, _130)) {
        f25 = r29 ? _130 : _12c;
      }
      if (!r29) {
        f25 += deltaMag / dot;
      }
      if (f25 < 10.f) {
        vec.x() = deltaMag / f25 * deltaFlat.x() * 0.6f;
        vec.y() = deltaMag / f25 * deltaFlat.y() * 0.6f;
        vec.z() = deltaZ / f25 - 0.5f * -CPhysicsActor::GravityConstant() * f25;
      }
    }
  }
  boid.x30_velocity = vec;
  boid.x80_26_launched = true;
  boid.x7c_remainingLaunchNotOnSurfaceFrames = 1;
  CSfxManager::AddEmitter(x55c_launchSfx, pos, zeus::skZero3f, true, false, 0x7f, x4_areaId);
}

void CWallCrawlerSwarm::AddParticle(const zeus::CTransform& xf) {
  static constexpr std::array particleCounts{8, 2, 0, 0};

  size_t i = 0;
  for (auto& p : x524_particleGens) {
    p->SetParticleEmission(true);
    p->SetTranslation(xf.origin);
    p->ForceParticleCreation(particleCounts[i]);
    p->SetParticleEmission(false);
    ++i;
  }
}

void CWallCrawlerSwarm::KillBoid(CBoid& boid, CStateManager& mgr, float deathRattleChance, float deadChance) {
  x130_lastKilledOffset = boid.GetTranslation();
  AddParticle(boid.GetTransform());
  boid.x80_24_active = false;

  const float sendDeadRoll = mgr.GetActiveRandom()->Float();
  const float sendDeathRattleRoll = mgr.GetActiveRandom()->Float();
  if (sendDeathRattleRoll < deathRattleChance) {
    SendScriptMsgs(EScriptObjectState::DeathRattle, mgr, EScriptObjectMessage::None);
  }
  if (sendDeadRoll < deadChance) {
    SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
  }
}

void CWallCrawlerSwarm::UpdatePartition() {
  x168_partitionedBoidLists.clear();
  x168_partitionedBoidLists.resize(125);
  x360_outlierBoidList = nullptr;

  const zeus::CAABox aabb = GetBoundingBox();
  const zeus::CVector3f vec = (aabb.max - aabb.min) / 5.f;
  for (auto& b : x108_boids) {
    if (b.GetActive()) {
      const zeus::CVector3f divVec = (b.GetTranslation() - aabb.min) / vec;
      const int xIdx = int(divVec.x());
      const int yIdx = int(divVec.y());
      const int zIdx = int(divVec.z());
      const int idx = xIdx + yIdx * 5 + zIdx * 25;
      if (idx < 0 || idx >= 125 || xIdx < 0 || xIdx >= 5 || yIdx < 0 || yIdx >= 5 || zIdx < 0 || zIdx >= 5) {
        b.x44_next = x360_outlierBoidList;
        x360_outlierBoidList = &b;
      } else {
        b.x44_next = x168_partitionedBoidLists[idx];
        x168_partitionedBoidLists[idx] = &b;
      }
    }
  }
}

zeus::CVector3f CWallCrawlerSwarm::FindClosestCell(const zeus::CVector3f& pos) const {
  float minDist = FLT_MAX;
  zeus::CVector3f ret;
  for (int r28 = 0; r28 < 5; ++r28) {
    for (int r29 = 0; r29 < 5; ++r29) {
      for (int r25 = 0; r25 < 5; ++r25) {
        const zeus::CAABox aabb = BoxForPosition(r28, r29, r25, 0.1f);
        const float dist = (aabb.center() - pos).magSquared();
        if (dist < minDist) {
          minDist = dist;
          ret = aabb.center();
        }
      }
    }
  }
  return ret;
}

void CWallCrawlerSwarm::UpdateEffects(CStateManager& mgr, CAnimData& aData, int vol) {
  if (aData.GetPassedSoundPOICount() == 0 || CAnimData::g_SoundPOINodes.empty()) {
    return;
  }

  for (size_t i = 0; i < aData.GetPassedSoundPOICount(); ++i) {
    const CSoundPOINode& n = CAnimData::g_SoundPOINodes[i];
    if (n.GetPoiType() != EPOIType::Sound ||
        (n.GetCharacterIndex() != -1 && n.GetCharacterIndex() != aData.GetCharacterIndex())) {
      continue;
    }

    const u16 sfx = CSfxManager::TranslateSFXID(u16(n.GetSfxId() & 0xffff));
    const bool loop = bool(n.GetSfxId() >> 31);
    if (loop) {
      continue;
    }

    CAudioSys::C3DEmitterParmData parmData;
    parmData.x0_pos = FindClosestCell(mgr.GetPlayer().GetTranslation());
    static float maxDist = n.GetMaxDist();
    static float falloff = n.GetFalloff();
    parmData.x18_maxDist = maxDist;
    parmData.x1c_distComp = falloff;
    parmData.x20_flags = 0x1;
    parmData.x24_sfxId = sfx;
    parmData.x26_maxVol = zeus::clamp(0, vol, 127) / 127.f;
    parmData.x27_minVol = 20.f / 127.f;
    parmData.x28_important = false;
    parmData.x29_prio = 0x7f;
    CSfxManager::AddEmitter(parmData, true, 0x7f, false, x4_areaId);
  }
}

zeus::CAABox CWallCrawlerSwarm::BoxForPosition(int x, int y, int z, float f) const {
  const zeus::CAABox aabb = GetBoundingBox();
  const zeus::CVector3f vec = (aabb.max - aabb.min) / 5.f;
  return zeus::CAABox(zeus::CVector3f(x, y, z) * vec + aabb.min - f,
                      zeus::CVector3f(x + 1, y + 1, z + 1) * vec + aabb.min + f);
}

void CWallCrawlerSwarm::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (x560_26_modelAssetDirty && CModelData::GetRenderingModel(mgr) != x4dc_whichModel) {
    const auto which = CModelData::GetRenderingModel(mgr);
    if (which != x4dc_whichModel) {
      AllocateSkinnedModels(mgr, which);
    }
  }

  xe4_27_notInSortedLists = true;
  x368_boidGenCooldownTimer -= dt;
  x36c_crabDamageCooldownTimer -= dt;
  ++x100_thinkCounter;
  const CGameArea* area = mgr.GetWorld()->GetAreaAlways(x4_areaId);
  const auto occState =
    area->IsPostConstructed() ? area->GetOcclusionState() : CGameArea::EOcclusionState::Occluded;
  if (occState != CGameArea::EOcclusionState::Visible) {
    if (x104_occludedTimer > 0.f) {
      x104_occludedTimer -= dt;
    }
    if (x104_occludedTimer <= 0.f) {
      return;
    }
    if (x100_thinkCounter & 0x2) {
      return;
    }
  } else {
    x104_occludedTimer = 7.f;
  }

  UpdateParticles(dt);
  x42c_lockOnIdx = SelectLockOnIdx(mgr);
  xe7_31_targetable = x42c_lockOnIdx != -1;

  if (x42c_lockOnIdx == -1) {
    RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
  } else {
    AddMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
  }

  while ((x54c_maxCreatedBoids == 0 || x550_createdBoids < x54c_maxCreatedBoids) &&
         x368_boidGenCooldownTimer <= 0.f) {
    int idx = 0;
    bool madeBoid = false;
    for (const auto& b : x108_boids) {
      if (!b.GetActive()) {
        CreateBoid(mgr, idx);
        x550_createdBoids += 1;
        x368_boidGenCooldownTimer += 1.f / x364_boidGenRate;
        madeBoid = true;
        break;
      }
      ++idx;
    }
    if (!madeBoid) {
      x368_boidGenCooldownTimer += 1.f / x364_boidGenRate;
      break;
    }
  }
  UpdatePartition();
  xe8_aabox = GetBoundingBox();

  int r21 = 0;
  for (int r26 = 0; r26 < 5; ++r26) {
    for (int r27 = 0; r27 < 5; ++r27) {
      for (int r20 = 0; r20 < 5; ++r20) {
        const int idx = r20 * 25 + r27 * 5 + r26;
        if (CBoid* boid = x168_partitionedBoidLists[idx]) {
          const zeus::CAABox aabb = BoxForPosition(r26, r27, r20, x374_boidRadius + 0.5f);
          CAreaCollisionCache ccache(aabb);
          CGameCollision::BuildAreaCollisionCache(mgr, ccache);
          while (boid != nullptr) {
            r21 += 1;
            if (boid->GetActive()) {
              if (x558_flavor == EFlavor::Scarab) {
                xe8_aabox.accumulateBounds(boid->GetTranslation() + x37c_scarabBoxMargin);
                xe8_aabox.accumulateBounds(boid->GetTranslation() - x37c_scarabBoxMargin);
              } else {
                xe8_aabox.accumulateBounds(boid->GetTranslation());
              }
            }
            if (((x100_thinkCounter & 0x1) == (r21 & 0x1) && boid->GetActive() && boid->x48_timeToDie < 0.1f) ||
                boid->x80_26_launched) {
              UpdateBoid(ccache, mgr, dt, *boid);
            }
            boid = boid->x44_next;
          }
        }
      }
    }
  }

  for (CBoid* boid = x360_outlierBoidList; boid; boid = boid->x44_next) {
    r21 += 1;
    if (boid->GetActive()) {
      xe8_aabox.accumulateBounds(boid->GetTranslation());
    }
    if (((x100_thinkCounter & 0x1) == (r21 & 0x1) && boid->GetActive() && boid->x48_timeToDie < 0.1f) ||
        boid->x80_26_launched) {
      const float margin = 1.5f + x374_boidRadius + 0.5f;
      const zeus::CAABox aabb(boid->GetTranslation() - margin, boid->GetTranslation() + margin);
      CAreaCollisionCache ccache(aabb);
      CGameCollision::BuildAreaCollisionCache(mgr, ccache);
      UpdateBoid(ccache, mgr, dt, *boid);
    }
  }

  x4b0_modelDatas[8]->GetAnimationData()->SetPlaybackRate(x160_animPlaybackSpeed);
  x4b0_modelDatas[8]->AdvanceAnimation(dt, mgr, x4_areaId, true);

  SAdvancementDeltas deltas1, deltas2;

  int r9 = 0;
  int r3 = 0;
  int r8 = 0;
  std::array<bool, 4> _38F8{};
  std::array<bool, 4> _38F4{};
  for (const auto& b : x108_boids) {
    if (b.GetActive() && !b.x80_26_launched) {
      if (b.x80_27_scarabExplodeTimerEnabled || b.x80_28_nearPlayer) {
        _38F8[r9 & 0x3] = true;
        ++r3;
      } else {
        _38F4[r9 & 0x3] = true;
        ++r8;
      }
    }
    ++r9;
  }

  for (size_t i = 0; i < _38F4.size(); ++i) {
    x4b0_modelDatas[i]->GetAnimationData()->SetPlaybackRate(x160_animPlaybackSpeed);
    deltas1 = x4b0_modelDatas[i]->AdvanceAnimation(dt, mgr, x4_areaId, true);
    x4b0_modelDatas[i + 4]->GetAnimationData()->SetPlaybackRate(x160_animPlaybackSpeed);
    deltas2 = x4b0_modelDatas[i + 4]->AdvanceAnimation(dt, mgr, x4_areaId, true);
    if (x4b0_modelDatas[i]->HasAnimData() && _38F4[i]) {
      UpdateEffects(mgr, *x4b0_modelDatas[i]->GetAnimationData(), r8 * 44 / x548_numBoids + 0x53);
    }
    if (x4b0_modelDatas[i + 4]->HasAnimData() && _38F8[i]) {
      UpdateEffects(mgr, *x4b0_modelDatas[i + 4]->GetAnimationData(), r3 * 44 / x548_numBoids + 0x53);
    }
    for (size_t r20 = i; r20 < x108_boids.size(); r20 += 4) {
      CBoid& b = x108_boids[r20];
      if (b.GetActive()) {
        if (b.x80_26_launched) {
          b.Translation() += b.x30_velocity * dt;
        } else if (b.x48_timeToDie > 0.f) {
          b.x48_timeToDie -= dt;
          if (b.x48_timeToDie < 0.7f * mgr.GetActiveRandom()->Float()) {
            KillBoid(b, mgr, 1.f, 0.05f);
          }
        } else if (b.x80_27_scarabExplodeTimerEnabled || b.x80_28_nearPlayer) {
          b.x30_velocity = b.GetTransform().rotate(deltas2.x0_posDelta) * 1.5f / dt;
          b.Translation() += b.x30_velocity * dt;
        } else {
          b.x30_velocity = b.GetTransform().rotate(deltas1.x0_posDelta) * 1.5f / dt;
          b.Translation() += b.x30_velocity * dt;
        }
      }
    }
  }

  if (x558_flavor == EFlavor::Crab) {
    const zeus::CVector3f playerPos = mgr.GetPlayer().GetTranslation();
    for (auto& b : x108_boids) {
      if (b.GetActive() && zeus::close_enough(b.x48_timeToDie, 0.f) && !b.x80_26_launched) {
        b.x80_28_nearPlayer = (playerPos - b.GetTranslation()).magnitude() < x154_attractionRadius;
      }
    }
  }

  if (x558_flavor == EFlavor::Parasite && x554_maxLaunches > 0) {
    const zeus::CVector3f _383c = mgr.GetPlayer().GetTranslation() + zeus::skUp;
    static constexpr auto filter = CMaterialFilter::MakeInclude(EMaterialTypes::Solid);
    int numLaunched = 0;
    for (const auto& b : x108_boids) {
      if (b.GetActive() && b.x80_26_launched) {
        ++numLaunched;
      }
    }

    for (auto it = x108_boids.begin(); it != x108_boids.end() && numLaunched < x554_maxLaunches; ++it) {
      CBoid& b = *it;
      if (b.GetActive() && zeus::close_enough(b.x48_timeToDie, 0.f) && !b.x80_26_launched &&
          (b.GetTranslation() - _383c).magSquared() < 18.f * 18.f && mgr.GetActiveRandom()->Float() <= 0.02f) {
        zeus::CVector3f dir = _383c - b.GetTranslation();
        const float mag = dir.magnitude();
        dir = dir / mag;
        if (mgr.RayStaticIntersection(b.GetTranslation(), dir, mag, filter).IsInvalid()) {
          LaunchBoid(b, _383c);
          ++numLaunched;
        }
      }
    }
  }
}

void CWallCrawlerSwarm::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  for (size_t i = 0; i < 5; ++i) {
    x4b0_modelDatas[i]->GetAnimationData()->PreRender();
  }
  bool activeBoid = false;
  for (auto& b : x108_boids) {
    if (b.GetActive()) {
      b.x80_25_inFrustum = frustum.sphereFrustumTest(zeus::CSphere(b.GetTranslation(), 2.f * x374_boidRadius));
      activeBoid = true;
    } else {
      b.x80_25_inFrustum = false;
    }
  }
  xe4_30_outOfFrustum = !activeBoid;
}

void CWallCrawlerSwarm::RenderParticles() const {
  for (const auto& p : x524_particleGens) {
    g_Renderer->AddParticleGen(*p);
  }
}

void CWallCrawlerSwarm::AddToRenderer(const zeus::CFrustum&, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  RenderParticles();

  if (xe4_30_outOfFrustum) {
    return;
  }

  if (CanRenderUnsorted(mgr)) {
    Render(mgr);
  } else {
    EnsureRendered(mgr);
  }
}

zeus::CColor CWallCrawlerSwarm::SoftwareLight(const CStateManager& mgr, const zeus::CAABox& aabb) const {
  CActorLights lights(8, zeus::skZero3f, 4, 4, false, false, false, 0.1f);
  lights.SetDirty();
  lights.SetCastShadows(false);
  lights.SetFindShadowLight(false);
  lights.BuildAreaLightList(mgr, *mgr.GetWorld()->GetAreaAlways(x4_areaId), aabb);
  lights.BuildDynamicLightList(mgr, aabb);
  zeus::CColor ret = lights.GetAmbientColor();
  ret.a() = 1.f;
  const zeus::CVector3f center = aabb.center();
  const u32 lightCount = lights.GetActiveLightCount();
  for (u32 i = 0; i < lightCount; ++i) {
    const CLight& light = lights.GetLight(i);
    const float dist = (light.GetPosition() - center).magnitude();
    const float att = 1.f / (dist * dist * light.GetAttenuationQuadratic() + dist * light.GetAttenuationLinear() +
                             light.GetAttenuationConstant());
    ret += zeus::CColor::lerp(zeus::skBlack, light.GetColor(), 0.8f * std::min(att, 1.f));
  }
  return ret;
}

void CWallCrawlerSwarm::HardwareLight(const CStateManager& mgr, const zeus::CAABox& aabb) const {
  CActorLights lights(8, zeus::skZero3f, 4, 4, false, false, false, 0.1f);
  lights.SetDirty();
  lights.SetCastShadows(false);
  lights.SetFindShadowLight(false);
  lights.BuildAreaLightList(mgr, *mgr.GetWorld()->GetAreaAlways(x4_areaId), aabb);
  lights.BuildDynamicLightList(mgr, aabb);
  for (const auto& m : x4b0_modelDatas) {
    lights.ActivateLights(*m->PickAnimatedModel(x4dc_whichModel).GetModelInst());
    if (const auto iceModel = m->GetAnimationData()->GetIceModel()) {
      lights.ActivateLights(*iceModel->GetModelInst());
    }
  }
}

void CWallCrawlerSwarm::RenderBoid(const CBoid* boid, u32& drawMask, bool thermalHot, const CModelFlags& flags) const {
  u32 modelIndex = 0x0;
  if (boid->x80_26_launched) {
    modelIndex = 0x8;
  } else if (boid->x48_timeToDie > 0.f) {
    modelIndex = 0x9;
  } else if (boid->x80_27_scarabExplodeTimerEnabled || boid->x80_28_nearPlayer) {
    modelIndex += 0x4;
  }

  CModelData& mData = *x4b0_modelDatas[modelIndex];
  CSkinnedModel& model = mData.PickAnimatedModel(x4dc_whichModel);
  if (!model.GetModelInst()->TryLockTextures()) {
    return;
  }

  const u32 thisDrawMask = 1u << modelIndex;
  if (drawMask & thisDrawMask) {
    drawMask &= ~thisDrawMask;
    mData.GetAnimationData()->BuildPose();
  }

  model.GetModelInst()->SetAmbientColor(boid->x40_ambientLighting);
  CGraphics::SetModelMatrix(boid->GetTransform());
  if (boid->x48_timeToDie > 0.f && !thermalHot) {
    constexpr CModelFlags useFlags(0, 0, 3, zeus::skWhite);
    mData.GetAnimationData()->Render(model, useFlags, std::nullopt, nullptr);
    if (auto iceModel = mData.GetAnimationData()->GetIceModel()) {
      if (!iceModel->GetModelInst()->TryLockTextures()) {
        return;
      }
      iceModel->GetModelInst()->SetAmbientColor(zeus::skWhite);
      const float alpha = 1.f - boid->x48_timeToDie;
      const zeus::CColor color(1.f, alpha > 0.f ? boid->x48_timeToDie : 1.f);
      const CModelFlags iceFlags(5, 0, 3, color);
      mData.GetAnimationData()->Render(*iceModel, iceFlags, std::nullopt, nullptr);
    }
  } else if (thermalHot) {
    constexpr CModelFlags thermFlags(5, 0, 3, zeus::skWhite);
    mData.RenderThermal(zeus::skWhite, zeus::CColor(0.f, 0.25f), thermFlags);
  } else {
    mData.GetAnimationData()->Render(model, flags, std::nullopt, nullptr);
  }
}

void CWallCrawlerSwarm::Render(CStateManager& mgr) {
  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(FMT_STRING("CWallCrawlerSwarm::Render {} {} {}"),
                                          x8_uid, xc_editorId, x10_name).c_str(), zeus::skOrange);
  u32 drawMask = 0xffffffff;
  const bool r24 = x560_24_enableLighting;
  const bool r23 = x560_25_useSoftwareLight;
  if (!r24) {
    // Ambient 50% grey
  }

  const bool thermalHot = mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot;
  CModelFlags flags(0, 0, 3, zeus::skWhite);
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
    flags = CModelFlags(5, 0, 3, zeus::CColor(1.f, 0.3f));
  }

  for (int r27 = 0; r27 < 5; ++r27) {
    for (int r28 = 0; r28 < 5; ++r28) {
      for (int r21 = 0; r21 < 5; ++r21) {
        const int idx = r21 * 25 + r28 * 5 + r27;
        if (CBoid* b = x168_partitionedBoidLists[idx]) {
          if (r24) {
            const zeus::CAABox aabb = BoxForPosition(r27, r28, r21, 0.f);
            if (r23) {
              if ((idx & 0x3) == (x100_thinkCounter & 0x3)) {
                const zeus::CColor color = SoftwareLight(mgr, aabb);
                for (CBoid* b2 = b; b2; b2 = b2->x44_next) {
                  if (b2->GetActive()) {
                    b2->x40_ambientLighting = zeus::CColor::lerp(b2->x40_ambientLighting, color, 0.3f);
                  }
                }
              }
            } else {
              HardwareLight(mgr, aabb);
            }
          }
          for (CBoid* b2 = b; b2; b2 = b2->x44_next) {
            if (b2->x80_25_inFrustum && b2->GetActive()) {
              RenderBoid(b2, drawMask, thermalHot, flags);
            }
          }
        }
      }
    }
  }

  CBoid* b = x360_outlierBoidList;
  for (int i = 1; b; ++i, b = b->x44_next) {
    if (b->x80_25_inFrustum && b->GetActive()) {
      if (r24) {
        const zeus::CAABox aabb(b->GetTranslation() - x374_boidRadius, b->GetTranslation() + x374_boidRadius);
        if (r23) {
          if ((i & 0x3) == (x100_thinkCounter & 0x3)) {
            zeus::CColor color = SoftwareLight(mgr, aabb);
            if (b->GetActive()) {
              b->x40_ambientLighting = zeus::CColor::lerp(b->x40_ambientLighting, color, 0.3f);
            }
          }
        } else {
          HardwareLight(mgr, aabb);
        }
      }
      RenderBoid(b, drawMask, thermalHot, flags);
    }
  }
  DrawTouchBounds();
}

bool CWallCrawlerSwarm::CanRenderUnsorted(const CStateManager&) const {
  return true;
}

void CWallCrawlerSwarm::CalculateRenderBounds() {
  x9c_renderBounds = GetBoundingBox();
}

std::optional<zeus::CAABox> CWallCrawlerSwarm::GetTouchBounds() const {
  return {xe8_aabox};
}

void CWallCrawlerSwarm::Touch(CActor& other, CStateManager& mgr) {
  CActor::Touch(other, mgr);
  if (TCastToConstPtr<CGameProjectile> proj = other) {
    if (x3c4_dVuln.WeaponHurts(proj->GetDamageInfo().GetWeaponMode(), false)) {
      if (auto projTb = proj->GetTouchBounds()) {
        const float f0 = 0.1f + x378_touchRadius;
        const float f30 = f0 * f0;
        for (auto& b : x108_boids) {
          if (b.GetActive()) {
            const zeus::CAABox aabb(b.GetTranslation() - f30, b.GetTranslation() + f30);
            if (aabb.intersects(*projTb)) {
              b.x78_health -= proj->GetDamageInfo().GetDamage(x3c4_dVuln);
              if (b.x78_health <= 0.f) {
                KillBoid(b, mgr, 1.f, 0.1f);
              }
            }
          }
        }
      }
    }
  }

  if (TCastToConstPtr<CPlayer> player = other) {
    const float radius = zeus::close_enough(x380_playerTouchRadius, 0.f) ? x378_touchRadius : x380_playerTouchRadius;
    if (auto playerTb = player->GetTouchBounds()) {
      for (auto& b : x108_boids) {
        if (b.GetActive() && b.x48_timeToDie <= 0.f) {
          if (x558_flavor == EFlavor::Scarab && b.x80_27_scarabExplodeTimerEnabled) {
            const zeus::CAABox aabb(b.GetTranslation() - x37c_scarabBoxMargin,
                                    b.GetTranslation() + x37c_scarabBoxMargin);
            if (playerTb->intersects(aabb)) {
              ExplodeBoid(b, mgr);
              SetExplodeTimers(b.GetTranslation(), 0.5f, 0.5f, 2.5f);
            }
          }
          const zeus::CAABox aabb(b.GetTranslation() - radius, b.GetTranslation() + radius);
          if (playerTb->intersects(aabb)) {
            if (b.GetActive() && x558_flavor == EFlavor::Parasite) {
              constexpr CDamageInfo dInfo(CWeaponMode(EWeaponType::AI), 2.0e-05f, 0.f, 0.f);
              mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), dInfo,
                 CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
              KillBoid(b, mgr, 0.f, 1.f);
            } else if (x558_flavor == EFlavor::Scarab) {
              ExplodeBoid(b, mgr);
            } else if (x36c_crabDamageCooldownTimer <= 0.f) {
              mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), x384_crabDamage,
                CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
              x36c_crabDamageCooldownTimer = x370_crabDamageCooldown;
              break;
            }
          }
        }
      }
    }
  }
}

zeus::CVector3f CWallCrawlerSwarm::GetOrbitPosition(const CStateManager&) const {
  if (x42c_lockOnIdx == -1) {
    return x124_lastOrbitPosition;
  }

  x124_lastOrbitPosition = x108_boids[x42c_lockOnIdx].GetTranslation();
  return x124_lastOrbitPosition;
}

zeus::CVector3f CWallCrawlerSwarm::GetAimPosition(const CStateManager&, float dt) const {
  if (x42c_lockOnIdx == -1) {
    return x124_lastOrbitPosition;
  }

  return x108_boids[x42c_lockOnIdx].x30_velocity * dt + x124_lastOrbitPosition;
}
void CWallCrawlerSwarm::ApplyRadiusDamage(const zeus::CVector3f& pos, const CDamageInfo& info, CStateManager& stateMgr) {
  for (CBoid& boid : x108_boids) {
    if (boid.GetActive() && (boid.GetTranslation() - pos).magSquared() < info.GetRadius() * info.GetRadius()) {
      boid.x78_health -= info.GetRadiusDamage();
      if (boid.x78_health <= 0.f) {
        KillBoid(boid, stateMgr, 1.f, 0.1f);
      }
    }
  }
}

void CWallCrawlerSwarm::FreezeCollision(CMarkerGrid const &grid) {
  for (CBoid &boid : x108_boids) {
    if (!boid.x80_24_active) {
      continue;
    }
    const float rad_sq_1 = x378_touchRadius * x378_touchRadius + 0.3f;
    const float rad_sq_2 = x378_touchRadius * x378_touchRadius + 0.5f;
    const zeus::CVector3f half_bounds(rad_sq_1, rad_sq_1, rad_sq_2);
    zeus::CAABox coll_bounds(boid.x0_xf.origin - half_bounds, boid.x0_xf.origin + half_bounds);
    if (grid.AABoxTouchesData(coll_bounds, 1)) {
      boid.x48_timeToDie = 1.f;
    }
  }
}
} // namespace metaforce
