#include "Runtime/World/CIceImpact.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CCollidableOBBTreeGroup.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/Collision/CollisionUtil.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/World/CDamageVulnerability.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWallCrawlerSwarm.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

static bool PointInSphere(zeus::CSphere const& sphere, zeus::CVector3f const& point) {
  return (point - sphere.position).magSquared() <= (sphere.radius * sphere.radius);
}

CIceImpact::CIceImpact(const TLockedToken<CGenDescription>& particle, TUniqueId uid, TAreaId aid, bool active,
                       std::string_view name, const zeus::CTransform& xf, u32 flags, const zeus::CVector3f& scale,
                       const zeus::CColor& color)
: CEffect(uid, CEntityInfo(aid, CEntity::NullConnectionList), active, name, xf)
, xe8_elementGen(std::make_unique<CElementGen>(particle, CElementGen::EModelOrientationType::One,
                                               CElementGen::EOptionalSystemFlags::One))
, xf0_genAssetId(particle.GetObjectTag()->id)
, x108_sphereGenRange(x34_transform.origin, 6.4f)
, x118_grid(zeus::CAABox(xf.origin - zeus::CVector3f(x100_halfBounds), xf.origin + zeus::CVector3f(x100_halfBounds)))
, x598_24_(flags & 0x2) {

  xe6_27_thermalVisorFlags = 2;
  x540_impactSpheres.push_back(SImpactSphere(x34_transform.origin, 2.4f, 1.6f, 0.f, 0.f));
  x540_impactSpheres.push_back(SImpactSphere(x34_transform.origin, 0.f, 1.f, 1.f, 0.f));
  x540_impactSpheres.push_back(SImpactSphere(x34_transform.origin, 0.f, 1.f, 1.f, 0.f));
  // ???
  x540_impactSpheres[1].x18_d = x540_impactSpheres[2].x14_c;
  x540_impactSpheres[1].x14_c += x540_impactSpheres[2].x10_b;
  x540_impactSpheres[2].x18_d = x540_impactSpheres[2].x14_c;
  x540_impactSpheres[2].x14_c += x540_impactSpheres[2].x10_b;
  x118_grid.MarkCells(zeus::CSphere(x34_transform.origin, 2.4f), 2);
}

void CIceImpact::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CIceImpact::CalculateRenderBounds() {
  auto bounds = xe8_elementGen->GetBounds();
  if (bounds) {
    x598_25_hasRenderBounds = true;
    x9c_renderBounds = *bounds;
  } else {
    x598_25_hasRenderBounds = false;
    x9c_renderBounds = zeus::CAABox(x34_transform.origin, x34_transform.origin);
  }
}

void CIceImpact::PreRender(CStateManager& mgr, zeus::CFrustum const& planes) {
  CActor::PreRender(mgr, planes);
  bool out_of_frustum = false;
  if (!x598_25_hasRenderBounds || !planes.aabbFrustumTest(x9c_renderBounds)) {
    out_of_frustum = true;
  }

  xe4_30_outOfFrustum = out_of_frustum;
}

void CIceImpact::AddToRenderer(zeus::CFrustum const& planes, CStateManager& mgr) {
  if (xe4_30_outOfFrustum) {
    return;
  }
  if (mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot) {
    EnsureRendered(mgr);
  } else {
    g_Renderer->AddParticleGen(*xe8_elementGen);
  }
}

void CIceImpact::Render(CStateManager& mgr) {
  CElementGen::SetSubtractBlend(true);
  CBooModel::SetRenderModelBlack(true);
  xe8_elementGen->Render();
  CElementGen::SetSubtractBlend(false);
  CBooModel::SetRenderModelBlack(false);
}

void CIceImpact::Think(float dt, CStateManager& mgr) {
  xf4_lifeTimer += dt;
  if (xf4_lifeTimer < 0.8f && xe8_elementGen->GetParticleCount() < 0x190) {
    for (SImpactSphere& sphere : x540_impactSpheres) {
      if (sphere.x14_c <= sphere.xc_a) {
        continue;
      }
      auto new_sphere = GenerateNewSphere();
      if (new_sphere) {
        sphere = *new_sphere;
      }
    }
    for (SImpactSphere& sphere : x540_impactSpheres) {
      if (sphere.x14_c > sphere.xc_a) {
        continue;
      }
      sphere.x18_d = sphere.x14_c;
      sphere.x14_c += sphere.x10_b;
      zeus::CSphere sphere_a(sphere.x0_pos, sphere.x14_c);
      zeus::CSphere sphere_b(sphere.x0_pos, sphere.x18_d);
      zeus::CAABox bbox(sphere_b.position - sphere_a.radius, sphere_b.position + sphere_a.radius);
      x104_ = 0.f;
      GenerateParticlesAgainstActors(mgr, bbox, sphere_a, sphere_b);
      CAreaCollisionCache cache(bbox);
      CGameCollision::BuildAreaCollisionCache(mgr, cache);
      for (auto const& leaf_cache : cache) {
        GenerateParticlesAgainstWorld(mgr, leaf_cache, sphere_a, sphere_b);
      }
    }
  }
  xe8_elementGen->SetOrientation(zeus::CTransform());
  xe8_elementGen->Update(dt);

  if (xec_ != kInvalidUniqueId) {
    if (TCastToPtr<CGameLight> light = mgr.ObjectById(xec_)) {
      if (x30_24_active) {
        light->SetLight(xe8_elementGen->GetLight());
      }
    }
  }
  if (x598_24_) {
    mgr.SetActorAreaId(*this, mgr.GetPlayer().GetAreaIdAlways());
  }
  if (xe8_elementGen->IsSystemDeletable()) {
    mgr.FreeScriptObject(x8_uid);
  }
}

std::optional<CIceImpact::SImpactSphere> CIceImpact::GenerateNewSphere() {
  xfc_searchDirection = (xfc_searchDirection + 1) & 7;
  bool fwd_z = (xfc_searchDirection & 1) != 0u;
  bool fwd_y = (xfc_searchDirection & 2) != 0u;
  bool fwd_x = (xfc_searchDirection & 4) != 0u;

  constexpr auto loop_cond = [](bool fwd, u32 counter) {
    if (fwd) {
      return counter < 14;
    }
    return counter != 0;
  };
  constexpr auto loop_step = [](bool fwd, u32 counter) {
    if (fwd) {
      return counter + 1;
    }
    return counter - 1;
  };

  for (u32 z = 8; loop_cond(fwd_z, z); z = loop_step(fwd_z, z)) {
    for (u32 y = 8; loop_cond(fwd_y, y); y = loop_step(fwd_y, y)) {
      for (u32 x = 8; loop_cond(fwd_x, x); x = loop_step(fwd_x, x)) {
        u32 grid_val = x118_grid.GetValue(z, y, x);
        if (grid_val != 1) {
          zeus::CVector3f pos = x118_grid.GetWorldPositionForCell(z, y, x);
          x118_grid.SetValue(z, y, x, 3);
          if (PointInSphere(x108_sphereGenRange, pos)) {
            x118_grid.MarkCells(zeus::CSphere(pos, 1.6f), 2);
            return SImpactSphere(pos, 1.6f, 1.6f, 0.f, 0.f);
          }
        }
      }
    }
  }
  return {};
}

void CIceImpact::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Deleted:
    if (xec_ != kInvalidUniqueId) {
      mgr.FreeScriptObject(xec_);
      xec_ = kInvalidUniqueId;
    }
    break;
  case EScriptObjectMessage::Registered:
    if (xe8_elementGen->SystemHasLight()) {
      xec_ = mgr.AllocateUniqueId();
      auto* new_light = new CGameLight(xec_, x4_areaId, x30_24_active, "IcePLight_"sv, x34_transform, x8_uid,
                                       xe8_elementGen->GetLight(), xf0_genAssetId.Value(), 1, 0.f);
      mgr.AddObject(new_light);
    }
    break;
  default:
    break;
  }

  CActor::AcceptScriptMsg(msg, uid, mgr);
  if (xec_ != kInvalidUniqueId) {
    mgr.SendScriptMsgAlways(xec_, uid, msg);
  }
}

void CIceImpact::Touch(CActor& actor, CStateManager& mgr) {
  if (xf4_lifeTimer > xf8_latestDamageTime) {
    return;
  }
  auto bounds = actor.GetTouchBounds();
  if (!bounds) {
    return;
  }

  if (TCastToPtr<CPatterned> ai = actor) {
    if (x118_grid.AABoxTouchesData(
            zeus::CAABox(bounds->min - zeus::CVector3f(0, 0, 0.5f), bounds->max + zeus::CVector3f(0, 0, 0.5f)), 1)) {
      CDamageVulnerability const* vuln = ai->GetDamageVulnerability();
      if (vuln->WeaponHits(CWeaponMode(EWeaponType::Ice), false) && ai->GetKnockBackController().GetEnableFreeze() &&
          ai->GetBodyController()->GetPercentageFrozen() == 0.f && (xf8_latestDamageTime - xf4_lifeTimer) > 0.5f &&
          xf4_lifeTimer < 0.8f) {
        mgr.ApplyDamage(x8_uid, actor.GetUniqueId(), kInvalidUniqueId,
                        CDamageInfo(CWeaponMode(EWeaponType::Ice), 100.f, 0.f, 1.f),
                        CMaterialFilter::MakeIncludeExclude(CMaterialList(EMaterialTypes::Solid), CMaterialList()),
                        zeus::CVector3f());
      }
    }
  }
  if (actor.GetMaterialList().HasMaterial(EMaterialTypes::ExcludeFromLineOfSightTest) &&
      x118_grid.AABoxTouchesData(*bounds, 1)) {
    mgr.ApplyDamage(
        x8_uid, actor.GetUniqueId(), kInvalidUniqueId, CDamageInfo(CWeaponMode(EWeaponType::Ice), 100.f, 0.f, 1.f),
        CMaterialFilter::MakeIncludeExclude(CMaterialList(EMaterialTypes::Solid), CMaterialList()), zeus::CVector3f());
  }
  if (TCastToPtr<CWallCrawlerSwarm> wall_crawler = actor) {
    if (xf8_latestDamageTime - xf4_lifeTimer > 0.5f) {
      wall_crawler->FreezeCollision(x118_grid);
    }
  }
}

void CIceImpact::GenerateParticlesAgainstWorld(CStateManager& mgr,
                                               const CMetroidAreaCollider::COctreeLeafCache& leaf_cache,
                                               zeus::CSphere& a, zeus::CSphere& b) {
  CMetroidAreaCollider::ResetInternalCounters();
  auto triangle_list = CMetroidAreaCollider::GetTriangleList();
  auto filter = CMaterialFilter::MakeExclude(EMaterialTypes::ProjectilePassthrough);
  for (auto const& node : leaf_cache) {
    CAreaOctTree::TriListReference arr = node.GetTriangleArray();
    bool subdivide_result = false;
    for (int i = 0; i < arr.GetSize() && !subdivide_result; i++) {
      u16 v1 = arr.GetAt(i);
      if (triangle_list[v1] != CMetroidAreaCollider::GetPrimitiveCheckCount()) {
        triangle_list[v1] = CMetroidAreaCollider::GetPrimitiveCheckCount();
        CCollisionSurface surface = node.GetOwner().GetMasterListTriangle(arr.GetAt(i));
        if (filter.Passes(CMaterialList(surface.GetSurfaceFlags()))) {
          subdivide_result =
              SubdivideAndGenerateParticles(mgr, surface.GetVert(0), surface.GetVert(1), surface.GetVert(1), a, b);
        }
      }
    }
  }
}

void CIceImpact::GenerateParticlesAgainstActors(CStateManager& mgr, const zeus::CAABox& box, const zeus::CSphere& a,
                                                const zeus::CSphere& b) {
  CMaterialFilter filter =
      CMaterialFilter::MakeExclude({EMaterialTypes::Character, EMaterialTypes::Player, EMaterialTypes::Projectile,
                                    EMaterialTypes::ProjectilePassthrough, EMaterialTypes::AIJoint});
  rstl::reserved_vector<TUniqueId, 1024> near_list;
  mgr.BuildNearList(near_list, box, filter, this);
  for (TUniqueId uid : near_list) {
    CEntity* ent = mgr.ObjectById(uid);
    TCastToPtr<CPhysicsActor> pAct = ent;
    if (pAct && pAct->GetCollisionPrimitive()->GetPrimType() == FOURCC('OBTG')) {
      const auto* prim = static_cast<const CCollidableOBBTreeGroup*>(pAct->GetCollisionPrimitive());
      for (int i = 0; i < prim->GetContainer()->NumTrees(); ++i) {
        GenerateParticlesAgainstOBBTree(mgr, *prim->GetOBBTreeAABox(i), pAct->GetPrimitiveTransform(), a, b);
      }
    } else if (const auto* actor = static_cast<CActor*>(ent)) {
      if (!actor->GetMaterialList().HasMaterial(EMaterialTypes::Solid)) {
        if (!TCastToPtr<CScriptWater>(ent)) {
          continue;
        }
        GenerateParticlesAgainstAABox(mgr, *actor->GetTouchBounds(), a, b);
      }
    }
  }
}

void CIceImpact::GenerateParticlesAgainstOBBTree(CStateManager& mgr, const COBBTree& tree, const zeus::CTransform& xf,
                                                 const zeus::CSphere& a, const zeus::CSphere& b) {
  auto filter = CMaterialFilter::MakeExclude(EMaterialTypes::Solid);

  for (int i = 0; i < tree.NumSurfaceMaterials(); i++) {
    CCollisionSurface surface = tree.GetTransformedSurface(i, xf);
    if (filter.Passes(static_cast<u64>(surface.GetSurfaceFlags()))) {
      if (SubdivideAndGenerateParticles(mgr, surface.GetVert(0), surface.GetVert(1), surface.GetVert(2), a, b)) {
        break;
      }
    }
  }
}

void CIceImpact::GenerateParticlesAgainstAABox(CStateManager& mgr, const zeus::CAABox& box,
                                               const zeus::CSphere& a, const zeus::CSphere& b) {
  for (int i = 0; i < 12; ++i) {
    auto tri = box.getTri(zeus::CAABox::EBoxFaceId(i / 2), (i & 1) * 2);
    if (SubdivideAndGenerateParticles(mgr, tri.x10_verts[0], tri.x10_verts[2], tri.x10_verts[1], a, b)) {
      return;
    }
  }
}

bool CIceImpact::SubdivideAndGenerateParticles(CStateManager& mgr, zeus::CVector3f const& v1, zeus::CVector3f const& v2,
                                               zeus::CVector3f const& v3, zeus::CSphere const& a,
                                               zeus::CSphere const& b) {
  if (!CollisionUtil::TriSphereOverlap(a, v1, v2, v3)) {
    return false;
  }
  if (!PointInSphere(b, v1) && !PointInSphere(b, v2) && !PointInSphere(b, v3)) {
    return false;
  }

  zeus::CVector3f xprod = (v2 - v1).cross(v3 - v1);
  float mag = xprod.magnitude();
  if (mag <= 1.f) {
    x104_ += mag;
    int particle_create_count = static_cast<int>(x104_);
    x104_ -= particle_create_count;
    for (int i = 0; i < particle_create_count; i++) {
      float rx = mgr.GetActiveRandom()->Float();
      float ry = mgr.GetActiveRandom()->Float();
      float rz = mgr.GetActiveRandom()->Float();
      float random_inv = 1.f / (rx + ry + rz);
      zeus::CVector3f point =
          zeus::baryToWorld(v1, v2, v3, zeus::CVector3f(rx * random_inv, ry * random_inv, rz * random_inv));
      u32 cx, cy, cz;
      if (!PointInSphere(b, point) && PointInSphere(a, point) && x118_grid.GetCoords(point, cx, cy, cz) &&
          ((x118_grid.GetValue(cx, cy, cz) & 1) == 0u)) {
        x118_grid.SetValue(cx, cy, cz, 1);
        zeus::CVector3f vec;
        switch (mgr.GetActiveRandom()->Range(0, 2)) {
        case 0:
          vec = v1 - point;
          if (!vec.canBeNormalized()) {
            vec = v2 - point;
          }
          break;
        case 1:
          vec = v2 - point;
          if (!vec.canBeNormalized()) {
            vec = v3 - point;
          }
          break;
        case 2:
          vec = v3 - point;
          if (!vec.canBeNormalized()) {
            vec = v1 - point;
          }
          break;
        }
        vec = vec.normalized();
        zeus::CVector3f norm_xprod = xprod.normalized();
        norm_xprod.x() = (mgr.GetActiveRandom()->Float() - 0.5f) * 0.4f + norm_xprod.x();
        norm_xprod.y() = (mgr.GetActiveRandom()->Float() - 0.5f) * 0.4f + norm_xprod.y();
        norm_xprod.z() = (mgr.GetActiveRandom()->Float() - 0.5f) * 0.4f + norm_xprod.z();
        xe8_elementGen->SetOrientation(zeus::lookAt(zeus::CVector3f(), norm_xprod.normalized(), vec));
        xe8_elementGen->SetTranslation(point);
        xe8_elementGen->ForceParticleCreation(1);
        if (xe8_elementGen->GetParticleCount() == xe8_elementGen->GetMaxParticles()) {
          return true;
        }
      }
    }
  } else {
    zeus::CVector3f point = zeus::baryToWorld(v1, v2, v3, zeus::CVector3f(1.f / 3.f));
    SubdivideAndGenerateParticles(mgr, v1, v2, point, a, b);
    SubdivideAndGenerateParticles(mgr, v2, v3, point, a, b);
    SubdivideAndGenerateParticles(mgr, v3, v1, point, a, b);
  }

  return false;
}

} // namespace metaforce
