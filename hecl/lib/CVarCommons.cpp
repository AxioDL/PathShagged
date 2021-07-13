#include "hecl/CVarCommons.hpp"

namespace hecl {

namespace {
CVarCommons* m_instance = nullptr;
}

CVarCommons::CVarCommons(CVarManager& manager) : m_mgr(manager) {
  m_fullscreen = m_mgr.findOrMakeCVar("fullscreen"sv, "Start in fullscreen"sv, false,
                                      hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive);
  m_graphicsApi = m_mgr.findOrMakeCVar("graphicsApi"sv, "API to use for rendering graphics"sv, DEFAULT_GRAPHICS_API,
                                       hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive |
                                           hecl::CVar::EFlags::ModifyRestart);
  m_drawSamples = m_mgr.findOrMakeCVar("drawSamples"sv, "Number of MSAA samples to use for render targets"sv, 1,
                                       hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive |
                                           hecl::CVar::EFlags::ModifyRestart);
  m_texAnisotropy = m_mgr.findOrMakeCVar(
      "texAnisotropy"sv, "Number of anisotropic samples to use for sampling textures"sv, 1,
      hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ModifyRestart);
  m_deepColor = m_mgr.findOrMakeCVar("deepColor"sv, "Allow framebuffer with color depth greater-then 24-bits"sv, false,
                                     hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive |
                                         hecl::CVar::EFlags::ModifyRestart);
  m_variableDt = m_mgr.findOrMakeCVar(
      "variableDt", "Enable variable delta time (experimental)", false,
      (hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ModifyRestart));
  m_lazyCommitResources = m_mgr.findOrMakeCVar(
      "lazyCommitResources"sv, "Enable lazy commiting resources to GPU", true,
      (hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive));

  m_debugOverlayPlayerInfo = m_mgr.findOrMakeCVar(
      "debugOverlay.playerInfo"sv, "Displays information about the player, such as location and orientation"sv, false,
      hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_debugOverlayWorldInfo = m_mgr.findOrMakeCVar(
      "debugOverlay.worldInfo"sv, "Displays information about the current world, such as world asset ID, and areaId"sv,
      false, hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_debugOverlayAreaInfo = m_mgr.findOrMakeCVar(
      "debugOverlay.areaInfo"sv,
      "Displays information about the current area, such as asset ID, object/layer counts, and active layer bits"sv,
      false, hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_debugOverlayLayerInfo = m_mgr.findOrMakeCVar(
      "debugOverlay.layerInfo"sv, "Displays information about the currently active area layers"sv, false,
      hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_debugOverlayShowFrameCounter =
      m_mgr.findOrMakeCVar("debugOverlay.showFrameCounter"sv, "Displays the current frame index"sv, false,
                           hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_debugOverlayShowFramerate =
      m_mgr.findOrMakeCVar("debugOverlay.showFramerate"sv, "Displays the current framerate"sv, false,
                           hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_debugOverlayShowInGameTime =
      m_mgr.findOrMakeCVar("debugOverlay.showInGameTime"sv, "Displays the current in game time"sv, false,
                           hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_debugOverlayShowRoomTimer = m_mgr.findOrMakeCVar(
      "debugOverlay.showRoomTimer", "Displays the current/last room timers in seconds and frames"sv, false,
      hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_debugOverlayShowResourceStats = m_mgr.findOrMakeCVar(
      "debugOverlay.showResourceStats"sv, "Displays the current live resource object and token counts"sv, false,
      hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_debugOverlayShowRandomStats = m_mgr.findOrMakeCVar(
      "debugOverlay.showRandomStats", "Displays the current number of random calls per frame"sv, false,
      hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_debugOverlayShowInput =
      m_mgr.findOrMakeCVar("debugOverlay.showInput"sv, "Displays user input"sv, false,
                           hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_debugToolDrawAiPath =
      m_mgr.findOrMakeCVar("debugTool.drawAiPath", "Draws the selected paths of any AI in the room"sv, false,
                           hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_debugToolDrawLighting = m_mgr.findOrMakeCVar("debugTool.drawLighting", "Draws the lighting setup in a room"sv,
                                                 false, hecl::CVar::EFlags::Game | hecl::CVar::EFlags::ReadOnly);
  m_debugToolDrawCollisionActors =
      m_mgr.findOrMakeCVar("debugTool.drawCollisionActors", "Draws the collision actors for enemies and objects"sv,
                           false, hecl::CVar::EFlags::Game | hecl::CVar::EFlags::ReadOnly);
  m_debugToolDrawMazePath =
      m_mgr.findOrMakeCVar("debugTool.drawMazePath", "Draws the maze path in Dynamo"sv, false,
                           hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_debugToolDrawPlatformCollision =
      m_mgr.findOrMakeCVar("debugTool.drawPlatformCollision", "Draws the bounding boxes of platforms"sv, false,
                           hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ReadOnly);
  m_logFile = m_mgr.findOrMakeCVar("logFile"sv, "Any log prints will be stored to this file upon exit"sv, "app.log"sv,
                                   hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive |
                                       hecl::CVar::EFlags::ModifyRestart);

  m_instance = this;
}

CVarCommons* CVarCommons::instance() { return m_instance; }
} // namespace hecl
