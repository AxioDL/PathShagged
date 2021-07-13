#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

#include "hecl/CVarManager.hpp"

#undef min
#undef max

namespace hecl {

using namespace std::literals;

#if defined(__APPLE__)
#define DEFAULT_GRAPHICS_API "Metal"sv
#else
#define DEFAULT_GRAPHICS_API "Vulkan"sv
#endif

struct CVarCommons {
  CVarManager& m_mgr;
  CVar* m_fullscreen = nullptr;
  CVar* m_graphicsApi = nullptr;
  CVar* m_drawSamples = nullptr;
  CVar* m_texAnisotropy = nullptr;
  CVar* m_deepColor = nullptr;
  CVar* m_variableDt = nullptr;
  CVar* m_lazyCommitResources = nullptr;

  CVar* m_debugOverlayPlayerInfo = nullptr;
  CVar* m_debugOverlayWorldInfo = nullptr;
  CVar* m_debugOverlayAreaInfo = nullptr;
  CVar* m_debugOverlayLayerInfo = nullptr;
  CVar* m_debugOverlayShowFrameCounter = nullptr;
  CVar* m_debugOverlayShowFramerate = nullptr;
  CVar* m_debugOverlayShowInGameTime = nullptr;
  CVar* m_debugOverlayShowResourceStats = nullptr;
  CVar* m_debugOverlayShowRandomStats = nullptr;
  CVar* m_debugOverlayShowRoomTimer = nullptr;
  CVar* m_debugOverlayShowInput = nullptr;
  CVar* m_debugToolDrawAiPath = nullptr;
  CVar* m_debugToolDrawLighting = nullptr;
  CVar* m_debugToolDrawCollisionActors = nullptr;
  CVar* m_debugToolDrawMazePath = nullptr;
  CVar* m_debugToolDrawPlatformCollision = nullptr;
  CVar* m_logFile = nullptr;

  CVarCommons(CVarManager& manager);

  bool getFullscreen() const { return m_fullscreen->toBoolean(); }

  void setFullscreen(bool b) { m_fullscreen->fromBoolean(b); }

  std::string getGraphicsApi() const { return m_graphicsApi->toLiteral(); }

  void setGraphicsApi(std::string_view api) { m_graphicsApi->fromLiteral(api); }

  uint32_t getSamples() const { return std::max(1u, m_drawSamples->toUnsigned()); }

  void setSamples(uint32_t v) { m_drawSamples->fromInteger(std::max(uint32_t(1), v)); }

  uint32_t getAnisotropy() const { return std::max(1u, uint32_t(m_texAnisotropy->toUnsigned())); }

  void setAnisotropy(uint32_t v) { m_texAnisotropy->fromInteger(std::max(1u, v)); }

  bool getDeepColor() const { return m_deepColor->toBoolean(); }

  void setDeepColor(bool b) { m_deepColor->fromBoolean(b); }

  bool getVariableFrameTime() const { return m_variableDt->toBoolean(); }

  void setVariableFrameTime(bool b) { m_variableDt->fromBoolean(b); }

  std::string getLogFile() const { return m_logFile->toLiteral(); };

  void setLogFile(std::string_view log) { m_logFile->fromLiteral(log); }

  void serialize() { m_mgr.serialize(); }

  static CVarCommons* instance();
};

} // namespace hecl
