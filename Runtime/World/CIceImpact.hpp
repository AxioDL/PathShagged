#pragma once

#include "Runtime/Collision/CMetroidAreaCollider.hpp"
#include "Runtime/World/CEffect.hpp"
#include "Runtime/World/CMarkerGrid.hpp"

namespace metaforce {
class COBBTree;
class CElementGen;
class CIceImpact : public CEffect {
private:
  struct SImpactSphere {
    zeus::CVector3f x0_pos;
    float xc_a;
    float x10_b;
    float x14_c;
    float x18_d;

    SImpactSphere(zeus::CVector3f const& pos, float a, float b, float c, float d)
    : x0_pos(pos), xc_a(a), x10_b(b), x14_c(c), x18_d(d) {}
  };
  std::unique_ptr<CElementGen> xe8_elementGen;
  TUniqueId xec_ = kInvalidUniqueId;
  CAssetId xf0_genAssetId;
  float xf4_lifeTimer = 0.f;
  float xf8_latestDamageTime = 4.f;
  u32 xfc_searchDirection = 0;
  float x100_halfBounds = 8.f;
  float x104_ = 0;
  zeus::CSphere x108_sphereGenRange;
  CMarkerGrid x118_grid;
  rstl::reserved_vector<SImpactSphere, 3> x540_impactSpheres;
  bool x598_24_ : 1;
  bool x598_25_hasRenderBounds : 1 = false;

  std::optional<CIceImpact::SImpactSphere> GenerateNewSphere();
  void GenerateParticlesAgainstWorld(CStateManager& mgr, const CMetroidAreaCollider::COctreeLeafCache& leaf_cache,
                                     zeus::CSphere& a, zeus::CSphere& b);
  void GenerateParticlesAgainstActors(CStateManager& mgr, const zeus::CAABox& box, const zeus::CSphere& a,
                                      const zeus::CSphere& b);
  void GenerateParticlesAgainstOBBTree(CStateManager& mgr, const COBBTree& tree, const zeus::CTransform& xf,
                                       const zeus::CSphere& a, const zeus::CSphere& b);
  void GenerateParticlesAgainstAABox(CStateManager& mgr, const zeus::CAABox& box, const zeus::CSphere& a,
                                     const zeus::CSphere& b);
  bool SubdivideAndGenerateParticles(CStateManager& mgr, zeus::CVector3f const& v1, zeus::CVector3f const& v2,
                                     zeus::CVector3f const& v3, zeus::CSphere const& a, zeus::CSphere const& b);

public:
  CIceImpact(const TLockedToken<CGenDescription>& particle, TUniqueId uid, TAreaId aid, bool active,
             std::string_view name, const zeus::CTransform& xf, u32 flags, const zeus::CVector3f& scale,
             const zeus::CColor& color);
  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void Render(CStateManager& frustum) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override { return x118_grid.GetBounds(); }
  void Touch(CActor& actor, CStateManager& mgr) override;
  void CalculateRenderBounds() override;
};

} // namespace metaforce
