#pragma once

#include <optional>
#include <string_view>

#include "Runtime/CToken.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"
#include "Runtime/World/CEntity.hpp"
#include "Runtime/World/CHUDMemoParms.hpp"

namespace metaforce {

class CScriptHUDMemo : public CEntity {
public:
  enum class EDisplayType {
    StatusMessage,
    MessageBox,
  };
  // Prime: x34; Echoes: x24
  CHUDMemoParms x34_parms;
#ifdef PRIME2
  // Echoes: x30
  bool useOriginator;
#endif
  // Prime: x3c; Echoes: x34
  EDisplayType x3c_dispType;
  // Prime: x40; Echoes: x38
  CAssetId x40_stringTableId;
  // Prime: x44; Echoes: x3c
  std::optional<TLockedToken<CStringTable>> x44_stringTable;

private:
public:
  DEFINE_ENTITY
  CScriptHUDMemo(TUniqueId, std::string_view, const CEntityInfo&, const CHUDMemoParms&, CScriptHUDMemo::EDisplayType,
                 CAssetId, bool);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
};
} // namespace metaforce
