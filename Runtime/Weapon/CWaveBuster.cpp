#include "Runtime/Weapon/CWaveBuster.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/Input/CRumbleManager.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"

#include "DataSpec/DNAMP1/SFX/Weapons.h"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

CWaveBuster::CWaveBuster(const TToken<CWeaponDescription>& desc, EWeaponType type, const zeus::CTransform& xf,
                         EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                         TUniqueId homingTarget, EProjectileAttrib attrib)
: CGameProjectile(true, desc, "WaveBuster"sv, type, xf, matType, dInfo, uid, aid, owner, homingTarget, attrib, false,
                  zeus::skOne3f, {}, -1, false)
, x2e8_originalXf(xf)
, x348_targetPoint(x2e8_originalXf.frontVector().normalized() * 25.f + x2e8_originalXf.origin)
, x354_busterSwoosh1(g_SimplePool->GetObj("BusterSwoosh1"))
, x360_busterSwoosh2(g_SimplePool->GetObj("BusterSwoosh2"))
, x36c_busterSparks(g_SimplePool->GetObj("BusterSparks"))
, x378_busterLight(g_SimplePool->GetObj("BusterLight"))
, m_lineRenderer1(CLineRenderer::EPrimitiveMode::LineStrip, 36, nullptr, true)
, m_lineRenderer2(CLineRenderer::EPrimitiveMode::LineStrip, 36, nullptr, true) {
  x354_busterSwoosh1.GetObj();
  x360_busterSwoosh2.GetObj();
  x36c_busterSparks.GetObj();
  x378_busterLight.GetObj();
  x384_busterSwoosh1Gen = std::make_unique<CParticleSwoosh>(x354_busterSwoosh1, 0);
  x388_busterSwoosh2Gen = std::make_unique<CParticleSwoosh>(x360_busterSwoosh2, 0);
  x38c_busterSparksGen = std::make_unique<CElementGen>(x36c_busterSparks);
  x390_busterLightGen = std::make_unique<CElementGen>(x378_busterLight);
}

void CWaveBuster::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CWaveBuster::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }

  if (GetAreaIdAlways() != mgr.GetWorld()->GetCurrentAreaId()) {
    mgr.SetActorAreaId(*this, mgr.GetWorld()->GetCurrentAreaId());
  }
  x3d0_27_ = false;
  x3d0_28_ = false;
  zeus::CVector3f local_160 = x2e8_originalXf.origin;
  zeus::CVector3f local_184 = x2e8_originalXf.frontVector();
  zeus::CVector3f local_16c = local_184.normalized();

  float dVar17 = 0.f;
  if (!x3d0_25_ && !x3d0_26_) {
    TUniqueId uid = kInvalidUniqueId;
    CRayCastResult res = SeekDamageTarget(uid, local_160, local_16c, mgr, dt);
    if (res.IsValid() && res.GetT() < 25.f) {
      if (TCastToPtr<CActor> act = mgr.ObjectById(uid)) {
        act->Touch(*this, mgr);
        mgr.ApplyDamage(GetUniqueId(), act->GetUniqueId(), GetOwnerId(), CDamageInfo(x12c_curDamageInfo, dt),
                        xf8_filter, GetTransform().basis[1]);
      } else {
        x3d0_28_ = true;
      }
      x3d0_27_ = true;
      dVar17 = res.GetT();
    }
  }

  if (x2c0_homingTargetId == kInvalidUniqueId || !x3d0_26_) {
    dVar17 = std::max(1.f, dVar17);
    x348_targetPoint = local_160 + (dVar17 * local_16c);
    if (!x3d0_25_) {
      const float x = mgr.GetActiveRandom()->Range(-1.f, 1.f);
      const float z = mgr.GetActiveRandom()->Range(-1.f, 1.f);
      x348_targetPoint += zeus::CVector3f{x, 0.f, z};
      SetTranslation(x348_targetPoint);
    } else {
      UpdateTargetSeek(dt, mgr);
    }
  } else {
    UpdateTargetDamage(dt, mgr);
  }

  if (UpdateBeamFrame(mgr, dt)) {
    ResetBeam(true);
  }

  zeus::CVector3f vec = x2c0_homingTargetId != kInvalidUniqueId && x3d0_26_
                        ? GetTransform() * zeus::CTransform::RotateY(x3c4_) * zeus::CVector3f{0.f, -3.f, -1.5f}
                        : (GetTranslation() + -1.5f) * GetTransform().basis[1].normalized();
  if (x3a0_ >= 0.5f || x2c0_homingTargetId == kInvalidUniqueId) {
    x330_ = x324_;
    x324_ = vec;
  } else {
    x324_ = x330_ * (1.f - x330_) + vec * (x3a0_ / 0.5f);
    x3a0_ += 0.125f * dt;
  }

  if (x2c8_projectileLight != kInvalidUniqueId) {
    x390_busterLightGen->Update(dt);
    if (TCastToPtr<CGameLight> light = mgr.ObjectById(x2c8_projectileLight)) {
      light->SetTransform(GetTransform());
      if (x390_busterLightGen && x390_busterLightGen->SystemHasLight()) {
        light->SetLight(x390_busterLightGen->GetLight());
      }
    }
  }

  x3c8_ += 20.f * dt;
  if (x3c8_ > 1.f) {
    ++x3cc_;
    if (x3cc_ > 2)
      x3cc_ = 0;
    x3c8_ = 0.f;
  }
  x38c_busterSparksGen->Update(dt);
}

void CWaveBuster::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  EnsureRendered(mgr, x2e8_originalXf.origin, GetSortingBounds(mgr));
}

void CWaveBuster::Render(CStateManager& mgr) {
  RenderParticles();
  RenderBeam();
  CWeapon::Render(mgr);
}

void CWaveBuster::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId senderId, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Deleted) {
    DeleteProjectileLight(mgr);
  } else if (msg == EScriptObjectMessage::Registered) {
    if (x390_busterLightGen != nullptr && x390_busterLightGen->SystemHasLight()) {
      const CLight light = x390_busterLightGen->GetLight();
      CreateProjectileLight("WaveBuster_Light", light, mgr);
    }

    // Thermal hot
    xe6_27_thermalVisorFlags = 2;

    x318_ = x2e8_originalXf.origin;
    x324_ = x34_transform.origin;
    x330_ = x34_transform.origin;
  }

  CGameProjectile::AcceptScriptMsg(msg, senderId, mgr);
}

std::optional<zeus::CAABox> CWaveBuster::GetTouchBounds() const {
  if (x3d0_28_) {
    return std::nullopt;
  }

  return GetProjectileBounds();
}

void CWaveBuster::UpdateFx(const zeus::CTransform& xf, float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  x2e8_originalXf = xf;
  x398_ -= std::max(0.f, x398_ - (60.f * dt));
  x170_projectile.SetVelocity(zeus::CVector3f{0.f, x3d0_25_ ? 1.6f : 0.f, 0.f});
}

void CWaveBuster::ResetBeam(bool deactivate) {
  if (!deactivate) {
    x38c_busterSparksGen->SetParticleEmission(false);
    x3d0_24_firing = false;
  } else {
    SetActive(false);
    x3d0_24_firing = false;
    x38c_busterSparksGen->SetParticleEmission(false);
    x398_ = 2.f * M_PIF;
  }
}

void CWaveBuster::SetNewTarget(TUniqueId id, CStateManager& mgr) {
  x2c0_homingTargetId = id;
  if (id == kInvalidUniqueId) {
    x3d0_26_ = false;
  } else {
    x3d0_26_ = true;
    CSfxManager::AddEmitter(SFXsfx06FF, GetTranslation(), zeus::skZero3f, true, false, 255, kInvalidAreaId);
    mgr.GetRumbleManager().Rumble(mgr, ERumbleFxId::PlayerBump, 0.5f, ERumblePriority::Three);
  }
}

void CWaveBuster::RenderParticles() {
  static constexpr std::array<zeus::CColor, 4> skCols = {{
      zeus::skWhite,
      {1.f, 0.f, 1.f, 1.f},
      {1.f, 0.f, 0.f, 1.f},
      {0.f, 0.f, 1.f, 1.f},
  }};
  zeus::CTransform CStack152 = x2e8_originalXf.getRotation();
  zeus::CVector3f local_d4 = x2e8_originalXf.origin;
  x38c_busterSparksGen->SetParticleEmission(true);
  zeus::CColor col = zeus::CColor::lerp(skCols[x3cc_], skCols[x3cc_ + 1], x3c8_);
  float dVar8 = x388_busterSwoosh2Gen->GetSwooshData(x388_busterSwoosh2Gen->GetSwooshDataCount()).x30_irot;
  float dVar9 = x384_busterSwoosh1Gen->GetSwooshData(x384_busterSwoosh1Gen->GetSwooshDataCount()).x30_irot;

  float fVar1 = 0.f;
  int iVar3 = 0;
  while (iVar3 < x384_busterSwoosh1Gen->GetSwooshDataCount()) {
    float dVar10 = fVar1;
    auto& swoosh1 = x384_busterSwoosh1Gen->GetSwooshData(iVar3);
    auto& swoosh2 = x388_busterSwoosh2Gen->GetSwooshData(iVar3);
    zeus::CVector3f point = zeus::getBezierPoint(GetTranslation(), x324_, x318_, local_d4, fVar1);
    swoosh1.xc_translation = point;
    swoosh2.xc_translation = point;
    swoosh1.x38_orientation = CStack152;
    swoosh2.x38_orientation = CStack152;
    swoosh2.x6c_color = col;
    float dVar6 = swoosh1.x30_irot;
    float dVar7 = swoosh2.x30_irot;
    swoosh1.x30_irot = dVar9;
    swoosh2.x30_irot = dVar8;
    x38c_busterSparksGen->SetTranslation(point);
    fVar1 = dVar10 + 0.4f;
    ++iVar3;
    dVar8 = dVar7;
    dVar9 = dVar6;
  }
  x38c_busterSparksGen->SetParticleEmission(false);
  x384_busterSwoosh1Gen->Render(GetActorLights());
  x388_busterSwoosh2Gen->Render(GetActorLights());
  x38c_busterSparksGen->Render(GetActorLights());
}

void CWaveBuster::RenderBeam() {
  const zeus::CTransform inv = x2e8_originalXf.inverse();
  const zeus::CVector3f vecA = inv * x2e8_originalXf.origin;
  const zeus::CVector3f vecB = inv * x318_;
  const zeus::CVector3f vecC = inv * x324_;
  const zeus::CVector3f vecD = inv * GetTranslation();
  float variation = 0.f;
  rstl::reserved_vector<zeus::CVector3f, 36> linePoints; // Used to be L2Cache access
  linePoints.resize(36);
  float t = 0.16;
  zeus::CVector3f lastPoint = vecA;
  while (t <= 1.f) {
    const zeus::CVector3f point = zeus::getBezierPoint(vecA, vecB, vecC, vecD, t);
    float angle = 0.f;
    for (size_t i = 0; i < 36; ++i) {
      const float randX = x394_rand.Range(-0.041667f, 0.041667f);
      const float randZ = x394_rand.Range(-0.041667f, 0.041667f);
      const float offX = variation * std::cos(angle + x398_) + randX;
      const float offZ = variation * std::sin(angle + x398_) + randZ;
      const float d = angle / (2.f * M_PIF);
      linePoints[i] = lastPoint * (1.f - d) + point * d + zeus::CVector3f{offX, 0.f, offZ};
      angle += zeus::degToRad(10.f);
    }
    variation = 0.25f;
    t += 0.16;
    lastPoint = point;
  }
  g_Renderer->SetModelMatrix(x2e8_originalXf);
  m_lineRenderer1.Reset();
  for (const zeus::CVector3f& vec : linePoints) {
    m_lineRenderer1.AddVertex(vec, zeus::skWhite, 12.f / 6.f);
  }
  m_lineRenderer1.Render();

  m_lineRenderer2.Reset();
  for (const zeus::CVector3f& vec : linePoints) {
    m_lineRenderer2.AddVertex(vec, zeus::CColor{1.f, 0.f, 1.f, 0.5f}, 48.f / 6);
  }
  m_lineRenderer2.Render();
}

CRayCastResult CWaveBuster::SeekDamageTarget(TUniqueId& uid, const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                             CStateManager& mgr, float dt) {
  CRayCastResult res = mgr.RayStaticIntersection(pos, dir, 25.f, xf8_filter);
  TUniqueId physId = kInvalidUniqueId;
  TUniqueId actId = kInvalidUniqueId;
  CRayCastResult physRes;
  CRayCastResult actRes;
  RayCastTarget(mgr, physId, actId, pos, dir, 25.f, physRes, actRes);
  if (actRes.IsValid() && ApplyDamageToTarget(physId, actRes, physRes, res, mgr, dt)) {
    uid = actId;
    return actRes;
  }

  if (physRes.IsValid() && ApplyDamageToTarget(actId, actRes, physRes, res, mgr, dt)) {
    uid = physId;
    return physRes;
  }

  return res;
}

bool CWaveBuster::ApplyDamageToTarget(TUniqueId damagee, const CRayCastResult& actRes, const CRayCastResult& physRes,
                                      const CRayCastResult& selfRes, CStateManager& mgr, float dt) {

  if (selfRes.IsInvalid() || physRes.GetT() <= selfRes.GetT()) {
    if (actRes.IsValid()) {
      if (TCastToPtr<CActor> act = mgr.ObjectById(damagee)) {
        act->Touch(*this, mgr);
        mgr.ApplyDamage(GetUniqueId(), damagee, GetOwnerId(), CDamageInfo(x12c_curDamageInfo, dt), xf8_filter,
                        GetTransform().basis[1]);
      }
    }
    return true;
  }
  return false;
}

void CWaveBuster::UpdateTargetSeek(float dt, CStateManager& mgr) {
  TUniqueId uid = kInvalidUniqueId;
  SeekTarget(dt, uid, mgr);
  if (x2c0_homingTargetId == kInvalidUniqueId && !x3d0_26_ &&
      (GetTranslation() - x2e8_originalXf.origin).magSquared() > 625.f) {
    x3d0_25_ = false;
  } else if (const TCastToConstPtr<CActor> act = mgr.GetObjectById(x2c0_homingTargetId)) {
    zeus::CVector3f vec = zeus::skForward;
    if (GetViewAngleToTarget(vec, *act) <= zeus::degToRad(90.f) && x3d0_26_) {
      x3d0_25_ = false;
    } else {
      x2c0_homingTargetId = kInvalidUniqueId;
      x3d0_26_ = false;
    }
  }
}

void CWaveBuster::UpdateTargetDamage(float dt, CStateManager& mgr) {
  if (const TCastToConstPtr<CActor> act = mgr.GetObjectById(x2c0_homingTargetId)) {
    const CHealthInfo* hInfo = act->GetHealthInfo(mgr);
    if (hInfo != nullptr && hInfo->GetHP() > 0.f) {
      x33c_ = act->GetAimPosition(mgr, 0.f);
      SetTranslation(x33c_);
      mgr.ApplyDamage(GetUniqueId(), x2c0_homingTargetId, GetOwnerId(), CDamageInfo(x12c_curDamageInfo, dt), xf8_filter,
                      zeus::skZero3f);
      return;
    }
  }

  SetTransform(zeus::lookAt(GetTranslation(), x348_targetPoint + (0.001f * x2e8_originalXf.basis[1].normalized())));
  x2c0_homingTargetId = kInvalidUniqueId;
  x39c_ = 0.f;
  x3a0_ = 0.f;
}

bool CWaveBuster::UpdateBeamFrame(CStateManager& mgr, float dt) {
  zeus::CVector3f local_ac = zeus::skForward;
  float viewAngle = 0.f;
  if (const TCastToConstPtr<CActor> act = mgr.GetObjectById(x2c0_homingTargetId)) {
    const CHealthInfo* hInfo = act->GetHealthInfo(mgr);
    if (hInfo != nullptr && hInfo->GetHP() > 0.f &&
        (act->GetTranslation() - x2e8_originalXf.origin).magSquared() > 10000.f) {
      return true;
    }
    viewAngle = GetViewAngleToTarget(local_ac, *act);
  }
  x3a8_ -= dt;
  if (x3a8_ <= 0.f) {
    const float dVar10 = mgr.GetActiveRandom()->Range(0.f, 2 * M_PIF);
    const float dVar9 = mgr.GetActiveRandom()->Range(0.05f, 0.25f);
    x3a4_ = (1.f / dVar9) * (dVar10 - x3ac_);
    x3a8_ = dVar9;
  }
  x3b8_ -= dt;
  if (x3b8_ <= 0.f) {
    const float dVar10 = mgr.GetActiveRandom()->Range(0.f, 0.5f);
    const float dVar9 = mgr.GetActiveRandom()->Range(0.1f, 0.5f);
    x3b4_ = (1.f / dVar9) * (dVar10 - x3b0_);
    x3b8_ = dVar9;
  }

  x3c0_ -= dt;
  if (x3c0_ <= 0.f) {
    const float dVar10 = mgr.GetActiveRandom()->Range(0.f, 2 * M_PIF);
    const float dVar9 = mgr.GetActiveRandom()->Range(0.05f, 0.25f);
    x3bc_ = (1.f / dVar9) * (dVar10 - x3ac_);
    x3c0_ = dVar9;
  }
  x3ac_ += x3a4_ * dt;
  x3b0_ += x3b4_ * dt;
  x3c4_ += x3bc_ * dt;
  x318_ = x2e8_originalXf * zeus::CTransform::RotateY(x3ac_) *
          zeus::CVector3f(0.f, 2.f, 1.5f * ((x2c0_homingTargetId == kInvalidUniqueId ? 1.f : 1.25f) - x3b0_ * x3b0_));
  return viewAngle > zeus::degToRad(90.f);
}

float CWaveBuster::GetViewAngleToTarget(zeus::CVector3f& p1, const CActor& act) {
  p1 = act.GetTranslation() - x2e8_originalXf.origin;
  if (!p1.canBeNormalized()) {
    p1 = GetTransform().basis[1];
  } else {
    p1.normalize();
  }

  return zeus::CVector2f::getAngleDiff(x2e8_originalXf.frontVector().toVec2f(), p1.toVec2f());
}

void CWaveBuster::RayCastTarget(CStateManager& mgr, TUniqueId& physId, TUniqueId& actId, const zeus::CVector3f& start,
                                const zeus::CVector3f& end, float length, CRayCastResult& physRes,
                                CRayCastResult& actorRes) {
  const zeus::CAABox box = zeus::CAABox(-0.5f, 0.f, -0.5f, 0.5f, 25.f, 0.5f).getTransformedAABox(x2e8_originalXf);
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, box,
                    CMaterialFilter::MakeExclude({EMaterialTypes::ProjectilePassthrough, EMaterialTypes::Player}),
                    this);
  if (length <= 0.f) {
    length = 100000.f;
  }

  float prevT = length;
  for (TUniqueId uid : nearList) {
    if (const TCastToConstPtr<CPhysicsActor> pAct = mgr.GetObjectById(uid)) {
      CRayCastResult res =
          pAct->GetCollisionPrimitive()->CastRay(start, end, length, xf8_filter, pAct->GetPrimitiveTransform());
      if (!res.IsValid() || res.GetT() >= length) {
        continue;
      }
      actorRes = res;
      actId = pAct->GetUniqueId();
      prevT = res.GetT();
    } else if (const TCastToConstPtr<CActor> act = mgr.GetObjectById(uid)) {
      if (auto bounds = act->GetTouchBounds()) {
        CCollidableAABox collidableBox{*bounds, GetMaterialList()};
        CRayCastResult res = collidableBox.CastRay(start, end, length, xf8_filter, zeus::CTransform{});
        if (!res.IsValid() || res.GetT() >= prevT) {
          continue;
        }
        actorRes = res;
        actId = act->GetUniqueId();
        prevT = res.GetT();
      }
    }
  }
}

CRayCastResult CWaveBuster::SeekTarget(float dt, TUniqueId& uid, CStateManager& mgr) {
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, GetProjectileBounds(),
                    CMaterialFilter::MakeIncludeExclude(
                        {EMaterialTypes::Solid}, {EMaterialTypes::ProjectilePassthrough, EMaterialTypes::Player}),
                    this);
  CRayCastResult res = RayCollisionCheckWithWorld(uid, x298_previousPos, GetTranslation(),
                                                  (GetTranslation() - x298_previousPos).magnitude(), nearList, mgr);
  if (res.IsInvalid()) {
    UpdateProjectileMovement(dt, mgr);
  } else {
    x3d0_25_ = false;
    if (uid == kInvalidUniqueId || uid != x2c0_homingTargetId) {
      x2c0_homingTargetId = kInvalidUniqueId;
    } else {
      x3d0_26_ = true;
      CSfxManager::AddEmitter(SFXsfx06FF, res.GetPoint(), zeus::skZero3f, true, false, 255, kInvalidAreaId);
    }
  }

  return res;
}

} // namespace metaforce