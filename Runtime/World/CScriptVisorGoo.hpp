#pragma once

#include <string_view>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CActor.hpp"

#include <zeus/CColor.hpp>

namespace metaforce {

class CScriptVisorGoo : public CActor {
  TToken<CGenDescription> xe8_particleDesc;
  TToken<CElectricDescription> xf0_electricDesc;
  u16 xf8_sfx;
  CAssetId xfc_particleId;
  CAssetId x100_electricId;
  float x104_minDist;
  float x108_maxDist;
  float x10c_nearProb;
  float x110_farProb;
  zeus::CColor x114_color;
  bool x118_24_angleTest : 1;

public:
  DEFINE_ENTITY
  CScriptVisorGoo(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                  CAssetId particle, CAssetId electric, float minDist, float maxDist, float nearProb, float farProb,
                  const zeus::CColor& color, int sfx, bool forceShow, bool active);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager& stateMgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override;
  void Render(CStateManager&) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;
};

} // namespace metaforce
