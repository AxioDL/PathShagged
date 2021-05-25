#pragma once

#include <memory>
#include <string_view>
#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Collision/CCollisionSurface.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CDamageVulnerability.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CAreaCollisionCache;
class CMarkerGrid;
class CWallCrawlerSwarm : public CActor {
public:
  enum class EFlavor {
    Parasite,
    Scarab,
    Crab
  };
  class CBoid {
    friend class CWallCrawlerSwarm;
    zeus::CTransform x0_xf;
    zeus::CVector3f x30_velocity;
    TUniqueId x3c_targetWaypoint = kInvalidUniqueId;
    zeus::CColor x40_ambientLighting = zeus::CColor(0.3f, 0.3f, 0.3f, 1.f);
    CBoid* x44_next = nullptr;
    float x48_timeToDie = 0.f;
    float x4c_timeToExplode = 0.f;
    CCollisionSurface x50_surface =
      CCollisionSurface(zeus::CVector3f(0.f, 0.f, 1.f), zeus::CVector3f(0.f, 1.f, 0.f),
                        zeus::CVector3f(1.f, 0.f, 0.f), 0xffffffff);
    float x78_health = 0.f;
    int x7c_framesNotOnSurface : 8 = 0;
    int x7c_idx : 10;
    int x7c_remainingLaunchNotOnSurfaceFrames : 8 = 0;
    bool x80_24_active : 1 = false;
    bool x80_25_inFrustum : 1 = false;
    bool x80_26_launched : 1 = false;
    bool x80_27_scarabExplodeTimerEnabled : 1 = false;
    bool x80_28_nearPlayer : 1 = false;

  public:
    CBoid(const zeus::CTransform& xf, int idx) : x0_xf(xf), x7c_idx(idx) {}

    zeus::CTransform& Transform() { return x0_xf; }
    zeus::CVector3f& Translation() { return x0_xf.origin; }
    const zeus::CTransform& GetTransform() const { return x0_xf; }
    const zeus::CVector3f& GetTranslation() const { return x0_xf.origin; }
    bool GetActive() const { return x80_24_active; }
  };
  class CRepulsor {
    friend class CWallCrawlerSwarm;
    zeus::CVector3f x0_center;
    float xc_mag;
  public:
    CRepulsor(const zeus::CVector3f& center, float mag) : x0_center(center), xc_mag(mag) {}
  };

private:
  zeus::CAABox xe8_aabox = zeus::skNullBox;
  s32 x100_thinkCounter = 0;
  float x104_occludedTimer = 5.f;
  std::vector<CBoid> x108_boids;
  zeus::CVector3f x118_boundingBoxExtent;
  mutable zeus::CVector3f x124_lastOrbitPosition;
  zeus::CVector3f x130_lastKilledOffset;
  float x13c_separationRadius;
  float x140_cohesionMagnitude;
  float x144_alignmentWeight;
  float x148_separationMagnitude;
  float x14c_moveToWaypointWeight;
  float x150_attractionMagnitude;
  float x154_attractionRadius;
  float x158_scarabScatterXYVelocity;
  float x15c_scarabTimeToExplode;
  float x160_animPlaybackSpeed;
  float x164_waypointGoalRadius = 3.f;
  rstl::reserved_vector<CBoid*, 125> x168_partitionedBoidLists;
  CBoid* x360_outlierBoidList = nullptr;
  float x364_boidGenRate;
  float x368_boidGenCooldownTimer = 0.f;
  float x36c_crabDamageCooldownTimer = 0.f;
  float x370_crabDamageCooldown;
  float x374_boidRadius;
  float x378_touchRadius;
  float x37c_scarabBoxMargin;
  float x380_playerTouchRadius;
  CDamageInfo x384_crabDamage;
  CDamageInfo x3a0_scarabExplodeDamage;
  CHealthInfo x3bc_hInfo;
  CDamageVulnerability x3c4_dVuln;
  s32 x42c_lockOnIdx = -1;
  /* Used to be position and normal array pointers */
  //rstl::reserved_vector<std::unique_ptr<float[]>, 10> x430_;
  //rstl::reserved_vector<std::unique_ptr<float[]>, 10> x484_;
  rstl::reserved_vector<std::shared_ptr<CModelData>, 10> x4b0_modelDatas;
  CModelData::EWhichModel x4dc_whichModel = CModelData::EWhichModel::Normal;
  std::vector<CRepulsor> x4e0_doorRepulsors;
  rstl::reserved_vector<TLockedToken<CGenDescription>, 4> x4f0_particleDescs;
  rstl::reserved_vector<std::unique_ptr<CElementGen>, 4> x524_particleGens;
  s32 x548_numBoids;
  s32 x54c_maxCreatedBoids;
  u32 x550_createdBoids = 0;
  s32 x554_maxLaunches;
  EFlavor x558_flavor;
  u16 x55c_launchSfx;
  u16 x55e_scatterSfx;
  bool x560_24_enableLighting : 1 = true;
  bool x560_25_useSoftwareLight : 1 = true;
  bool x560_26_modelAssetDirty : 1 = false;

  void AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which);
  void AddDoorRepulsors(CStateManager& mgr);
  void UpdateParticles(float dt);
  int SelectLockOnIdx(CStateManager& mgr) const;
  zeus::CAABox GetBoundingBox() const;
  TUniqueId GetWaypointForState(EScriptObjectState state, CStateManager& mgr) const;
  static zeus::CVector3f ProjectVectorToPlane(const zeus::CVector3f& pt, const zeus::CVector3f& plane) {
    return pt - plane * pt.dot(plane);
  }
  static zeus::CVector3f ProjectPointToPlane(const zeus::CVector3f& p0, const zeus::CVector3f& p1,
                                             const zeus::CVector3f& plane) {
    return p0 - (p0 - p1).dot(plane) * plane;
  }
  bool PointOnSurface(const CCollisionSurface& surf, const zeus::CVector3f& pos, const zeus::CPlane& plane) const;
  bool FindBestSurface(const CAreaCollisionCache& ccache, const zeus::CVector3f& pos, float radius,
                       CCollisionSurface& out) const;
  CCollisionSurface FindBestCollisionInBox(CStateManager& mgr, const zeus::CVector3f& wpPos) const;
  void CreateBoid(CStateManager& mgr, int idx);
  void ExplodeBoid(CBoid& boid, CStateManager& mgr);
  void SetExplodeTimers(const zeus::CVector3f& pos, float radius, float minTime, float maxTime);
  CBoid* GetListAt(const zeus::CVector3f& pos);
  void BuildBoidNearList(const CBoid& boid, float radius, rstl::reserved_vector<CBoid*, 50>& nearList);
  void ApplySeparation(const CBoid& boid, const rstl::reserved_vector<CBoid*, 50>& nearList,
                       zeus::CVector3f& aheadVec) const;
  void ApplySeparation(const CBoid& boid, const zeus::CVector3f& separateFrom, float separationRadius, float separationMagnitude,
                       zeus::CVector3f& aheadVec) const;
  void ScatterScarabBoid(CBoid& boid, CStateManager& mgr) const;
  void MoveToWayPoint(CBoid& boid, CStateManager& mgr, zeus::CVector3f& aheadVec) const;
  void ApplyCohesion(const CBoid& boid, const rstl::reserved_vector<CBoid*, 50>& nearList,
                     zeus::CVector3f& aheadVec) const;
  void ApplyCohesion(const CBoid& boid, const zeus::CVector3f& cohesionFrom, float cohesionRadius, float cohesionMagnitude,
                     zeus::CVector3f& aheadVec) const;
  void ApplyAlignment(const CBoid& boid, const rstl::reserved_vector<CBoid*, 50>& nearList,
                      zeus::CVector3f& aheadVec) const;
  void ApplyAttraction(const CBoid& boid, const zeus::CVector3f& attractTo, float attractionRadius, float attractionMagnitude,
                       zeus::CVector3f& aheadVec) const;
  void UpdateBoid(const CAreaCollisionCache& ccache, CStateManager& mgr, float dt, CBoid& boid);
  void LaunchBoid(CBoid& boid, const zeus::CVector3f& dir);
  void AddParticle(const zeus::CTransform& xf);
  void KillBoid(CBoid& boid, CStateManager& mgr, float deathRattleChance, float deadChance);
  void UpdatePartition();
  zeus::CVector3f FindClosestCell(const zeus::CVector3f& pos) const;
  void UpdateEffects(CStateManager& mgr, CAnimData& aData, int vol);
  zeus::CAABox BoxForPosition(int x, int y, int z, float f) const;
  void RenderParticles() const;
  zeus::CColor SoftwareLight(const CStateManager& mgr, const zeus::CAABox& aabb) const;
  void HardwareLight(const CStateManager& mgr, const zeus::CAABox& aabb) const;
  void RenderBoid(const CBoid* boid, u32& drawMask, bool thermalHot, const CModelFlags& flags) const;

public:
  CWallCrawlerSwarm(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
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
                    s32 scatterSfx, const CActorParameters& aParams);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override;
  void Render(CStateManager&) override;
  bool CanRenderUnsorted(const CStateManager&) const override;
  void CalculateRenderBounds() override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Touch(CActor& other, CStateManager&) override;
  zeus::CVector3f GetOrbitPosition(const CStateManager&) const override;
  zeus::CVector3f GetAimPosition(const CStateManager&, float) const override;
  const zeus::CVector3f& GetLastKilledOffset() const { return x130_lastKilledOffset; }
  void ApplyRadiusDamage(const zeus::CVector3f& pos, const CDamageInfo& info, CStateManager& stateMgr);
  const std::vector<CBoid>& GetBoids() const { return x108_boids; }
  int GetCurrentLockOnId() const { return x42c_lockOnIdx; }
  bool GetLockOnLocationValid(int id) const { return id >= 0 && id < x108_boids.size() && x108_boids[id].GetActive(); }
  const zeus::CVector3f& GetLockOnLocation(int id) const { return x108_boids[id].GetTranslation(); }
  void FreezeCollision(const CMarkerGrid& grid);
};
} // namespace metaforce
