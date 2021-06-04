#include "Runtime/World/CScriptHUDMemo.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"
#include "Runtime/MP1/CSamusHud.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

CScriptHUDMemo::CScriptHUDMemo(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                               const CHUDMemoParms& parms, EDisplayType disp, CAssetId msg, bool active)
: CEntity(uid, info, active, name)
, x34_parms(parms)
#ifdef PRIME2
, useOriginator(false)
#endif
, x3c_dispType(disp)
, x40_stringTableId(msg) {
  if (msg.IsValid()) {
    x44_stringTable.emplace(g_SimplePool->GetObj({FOURCC('STRG'), msg}));
  }
}

void CScriptHUDMemo::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptHUDMemo::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CHUDMemoParms parms = x34_parms;
#ifdef PRIME2
  if (useOriginator) {
    auto originator = ScriptObjectMessage_GetOriginator(msg);
    if (TCastToPtr<CPlayer> player = mgr.ObjectById(originator)) {
      parms = CHUDMemoParms(x34_parms.GetDisplayTime(), x34_parms.IsClearMemoWindow(), x34_parms.IsFadeOutOnly(),
                            x34_parms.IsHintMemo(), 1 << mgr.MaskUIdNumPlayers(originator), x34_parms.IsFadeInOnly());
    }
  }
#endif

  if (msg == EScriptObjectMessage::SetToZero) {
    if (GetActive()) {
      if (x3c_dispType == EDisplayType::MessageBox) {
        mgr.ShowPausedHUDMemo(x40_stringTableId, parms.GetDisplayTime());
      } else if (x3c_dispType == EDisplayType::StatusMessage) {
        if (x44_stringTable) {
          MP1::CSamusHud::DisplayHudMemo((*x44_stringTable)->GetString(0), parms);
        } else {
          MP1::CSamusHud::DisplayHudMemo(u"", parms);
        }
      }
    }
  } else if (msg == EScriptObjectMessage::Deactivate && GetActive() && x3c_dispType == EDisplayType::StatusMessage) {
    MP1::CSamusHud::DisplayHudMemo(u"", CHUDMemoParms(0.f, false, true, false));
  }

  CEntity::AcceptScriptMsg(msg, uid, mgr);
}

} // namespace metaforce
