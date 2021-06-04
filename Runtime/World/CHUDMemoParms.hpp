#pragma once

#include "Runtime/IOStreams.hpp"

namespace metaforce {

class CHUDMemoParms {
  float x0_dispTime = 0.f;
  bool x4_clearMemoWindow = false;
  bool x5_fadeOutOnly = false;
  bool x6_hintMemo = false;
#ifdef PRIME2
  // Echoes: x7
  bool fadeInText = false;
  // Echoes: x8
  int playerMask = 0;
#endif

public:
  CHUDMemoParms() = default;
  CHUDMemoParms(float dispTime, bool clearMemoWindow, bool fadeOutOnly, bool hintMemo)
  : x0_dispTime(dispTime), x4_clearMemoWindow(clearMemoWindow), x5_fadeOutOnly(fadeOutOnly), x6_hintMemo(hintMemo) {}
#ifdef PRIME2
  CHUDMemoParms(float dispTime, bool clearMemoWindow, bool fadeOutOnly, bool hintMemo, int playerMask, bool fadeInText)
  : x0_dispTime(dispTime), x4_clearMemoWindow(clearMemoWindow), x5_fadeOutOnly(fadeOutOnly), x6_hintMemo(hintMemo), fadeInText(fadeInText), playerMask(playerMask) {}
#endif

  explicit CHUDMemoParms(CInputStream& in) {
    x0_dispTime = in.readFloatBig();
    x4_clearMemoWindow = in.readBool();
  }

  float GetDisplayTime() const { return x0_dispTime; }
  bool IsClearMemoWindow() const { return x4_clearMemoWindow; }
  bool IsFadeOutOnly() const { return x5_fadeOutOnly; }
  bool IsHintMemo() const { return x6_hintMemo; }
#ifdef PRIME2
  bool IsFadeInOnly() const { return fadeInText; }
  int GetPlayerMask() const { return playerMask; }
#endif
};

} // namespace metaforce
