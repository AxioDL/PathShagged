#include "ImGuiConsole.hpp"

#include "../version.h"
#include "MP1/MP1.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "ImGuiEngine.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

std::array<ImGuiEntityEntry, 1024> ImGuiConsole::entities;
std::set<TUniqueId> ImGuiConsole::inspectingEntities;

void ImGuiStringViewText(std::string_view text) {
  // begin()/end() do not work on MSVC
  ImGui::TextUnformatted(text.data(), text.data() + text.size());
}

void ImGuiTextCenter(std::string_view text) {
  ImGui::NewLine();
  float fontSize = ImGui::GetFontSize() * float(text.size()) / 2;
  ImGui::SameLine(ImGui::GetWindowSize().x / 2 - fontSize + fontSize / 2);
  ImGuiStringViewText(text);
}

static std::unordered_map<CAssetId, std::unique_ptr<CDummyWorld>> dummyWorlds;
static std::unordered_map<CAssetId, TCachedToken<CStringTable>> stringTables;

static std::string ReadUtf8String(CStringTable* tbl, int idx) { return hecl::Char16ToUTF8(tbl->GetString(idx)); }

static bool ContainsCaseInsensitive(std::string_view str, std::string_view val) {
  return std::search(str.begin(), str.end(), val.begin(), val.end(),
                     [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }) != str.end();
}

static std::vector<std::pair<std::string, CAssetId>> ListWorlds() {
  std::vector<std::pair<std::string, CAssetId>> worlds;
  for (const auto& pak : g_ResFactory->GetResLoader()->GetPaks()) {
    if (!pak->IsWorldPak()) {
      continue;
    }
    CAssetId worldId = pak->GetMLVLId();
    if (!dummyWorlds.contains(worldId)) {
      dummyWorlds[worldId] = std::make_unique<CDummyWorld>(worldId, false);
    }
    auto& world = dummyWorlds[worldId];
    bool complete = world->ICheckWorldComplete();
    if (!complete) {
      continue;
    }
    CAssetId stringId = world->IGetStringTableAssetId();
    if (!stringId.IsValid()) {
      continue;
    }
    if (!stringTables.contains(stringId)) {
      stringTables[stringId] = g_SimplePool->GetObj(SObjectTag{SBIG('STRG'), stringId});
    }
    worlds.emplace_back(ReadUtf8String(stringTables[stringId].GetObj(), 0), worldId);
  }
  return worlds;
}

static std::vector<std::pair<std::string, TAreaId>> ListAreas(CAssetId worldId) {
  std::vector<std::pair<std::string, TAreaId>> areas;
  const auto& world = dummyWorlds[worldId];
  for (int i = 0; i < world->IGetAreaCount(); ++i) {
    const auto* area = world->IGetAreaAlways(i);
    if (area == nullptr) {
      continue;
    }
    CAssetId stringId = area->IGetStringTableAssetId();
    if (!stringId.IsValid()) {
      continue;
    }
    if (!stringTables.contains(stringId)) {
      stringTables[stringId] = g_SimplePool->GetObj(SObjectTag{SBIG('STRG'), stringId});
    }
    areas.emplace_back(ReadUtf8String(stringTables[stringId].GetObj(), 0), TAreaId{i});
  }
  return areas;
}

static void Warp(const CAssetId worldId, TAreaId aId) {
  g_GameState->SetCurrentWorldId(worldId);
  g_GameState->GetWorldTransitionManager()->DisableTransition();
  if (aId >= g_GameState->CurrentWorldState().GetLayerState()->GetAreaCount()) {
    aId = 0;
  }
  g_GameState->CurrentWorldState().SetAreaId(aId);
  g_Main->SetFlowState(EFlowState::None);
  if (g_StateManager != nullptr) {
    g_StateManager->SetWarping(true);
    g_StateManager->SetShouldQuitGame(true);
  } else {
    // TODO(encounter): warp from menu?
  }
}

void ImGuiConsole::ShowMenuGame() {
  m_paused = g_Main->IsPaused();
  if (ImGui::MenuItem("Paused", nullptr, &m_paused)) {
    g_Main->SetPaused(m_paused);
  }
  if (ImGui::MenuItem("Step Frame", nullptr, &m_stepFrame, m_paused)) {
    g_Main->SetPaused(false);
  }
  if (ImGui::BeginMenu("Warp", g_StateManager != nullptr && g_ResFactory != nullptr &&
                                   g_ResFactory->GetResLoader() != nullptr)) {
    for (const auto& world : ListWorlds()) {
      if (ImGui::BeginMenu(world.first.c_str())) {
        for (const auto& area : ListAreas(world.second)) {
          if (ImGui::MenuItem(area.first.c_str())) {
            Warp(world.second, area.second);
          }
        }
        ImGui::EndMenu();
      }
    }
    ImGui::EndMenu();
  }
  if (ImGui::MenuItem("Quit", "Alt+F4")) {
    g_Main->Quit();
  }
}

void ImGuiConsole::LerpDebugColor(CActor* act) {
  if (!act->m_debugSelected && !act->m_debugHovered) {
    act->m_debugAddColorTime = 0.f;
    act->m_debugAddColor = zeus::skClear;
    return;
  }
  act->m_debugAddColorTime += 1.f / 60.f;
  float lerp = act->m_debugAddColorTime;
  if (lerp > 2.f) {
    lerp = 0.f;
    act->m_debugAddColorTime = 0.f;
  } else if (lerp > 1.f) {
    lerp = 2.f - lerp;
  }
  act->m_debugAddColor = zeus::CColor::lerp(zeus::skClear, zeus::skBlue, lerp);
}

void ImGuiConsole::UpdateEntityEntries() {
  CObjectList& list = g_StateManager->GetAllObjectList();
  s16 uid = list.GetFirstObjectIndex();
  while (uid != -1) {
    ImGuiEntityEntry& entry = ImGuiConsole::entities[uid];
    if (entry.uid == kInvalidUniqueId || entry.ent == nullptr) {
      CEntity* ent = list.GetObjectByIndex(uid);
      entry.uid = ent->GetUniqueId();
      entry.ent = ent;
      entry.type = ent->ImGuiType();
      entry.name = ent->GetName();
      entry.isActor = TCastToPtr<CActor>(ent).IsValid();
    } else {
      entry.active = entry.ent->GetActive();
    }
    if (entry.isActor) {
      LerpDebugColor(entry.AsActor());
    }
    entry.ent->m_debugHovered = false;
    uid = list.GetNextObjectIndex(uid);
  }
}

void ImGuiConsole::BeginEntityRow(const ImGuiEntityEntry& entry) {
  ImGui::PushID(entry.uid.Value());
  ImGui::TableNextRow();
  bool isActive = entry.active;

  ImVec4 textColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
  if (!isActive) {
    textColor.w = 0.5f;
  }
  ImGui::PushStyleColor(ImGuiCol_Text, textColor);

  if (ImGui::TableNextColumn()) {
    auto text = fmt::format(FMT_STRING("{:x}"), entry.uid.Value());
    ImGui::Selectable(text.c_str(), &entry.ent->m_debugSelected,
                      ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap);
    if (ImGui::IsItemHovered()) {
      entry.ent->m_debugHovered = true;
    }

    if (ImGui::BeginPopupContextItem(text.c_str())) {
      ImGui::PopStyleColor();
      if (ImGui::MenuItem(isActive ? "Deactivate" : "Activate")) {
        entry.ent->SetActive(!isActive);
      }
      if (ImGui::MenuItem("Highlight", nullptr, &entry.ent->m_debugSelected)) {
        entry.ent->SetActive(!isActive);
      }
      ImGui::Separator();
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.77f, 0.12f, 0.23f, 1.f});
      if (ImGui::MenuItem("Delete")) {
        g_StateManager->FreeScriptObject(entry.uid);
      }
      ImGui::PopStyleColor();
      ImGui::EndPopup();
      ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    }
  }
}

void ImGuiConsole::EndEntityRow(const ImGuiEntityEntry& entry) {
  if (ImGui::TableNextColumn()) {
    if (ImGui::SmallButton("View")) {
      ImGuiConsole::inspectingEntities.insert(entry.uid);
    }
  }
  ImGui::PopStyleColor();
  ImGui::PopID();
}

static void RenderEntityColumns(const ImGuiEntityEntry& entry) {
  ImGuiConsole::BeginEntityRow(entry);
  if (ImGui::TableNextColumn()) {
    ImGuiStringViewText(entry.type);
  }
  if (ImGui::TableNextColumn()) {
    ImGuiStringViewText(entry.name);
  }
  ImGuiConsole::EndEntityRow(entry);
}

void ImGuiConsole::ShowInspectWindow(bool* isOpen) {
  if (ImGui::Begin("Inspect", isOpen)) {
    CObjectList& list = g_StateManager->GetAllObjectList();
    ImGui::Text("Objects: %d / 1024", list.size());
    if (ImGui::Button("Deselect all")) {
      for (auto* const ent : list) {
        ent->m_debugSelected = false;
      }
    }
    ImGui::InputText("Filter", m_inspectFilterText.data(), m_inspectFilterText.size());
    ImGui::Checkbox("Active", &m_inspectActiveOnly);

    if (ImGui::BeginTable("Entities", 4,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ScrollY)) {
      ImGui::TableSetupColumn("ID",
                              ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_DefaultSort |
                                  ImGuiTableColumnFlags_WidthFixed,
                              0, 'id');
      ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 0, 'type');
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0, 'name');
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed |
                                      ImGuiTableColumnFlags_NoResize);
      ImGui::TableSetupScrollFreeze(0, 1);
      ImGui::TableHeadersRow();

      ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
      bool hasSortSpec = sortSpecs != nullptr && sortSpecs->SpecsCount == 1 && // no multi-sort
                         // We can skip sorting if we just want uid ascending,
                         // since that's how we iterate over CObjectList
                         (sortSpecs->Specs[0].ColumnUserID != 'id' ||
                          sortSpecs->Specs[0].SortDirection != ImGuiSortDirection_Ascending);
      std::string_view search{m_inspectFilterText.data(), strlen(m_inspectFilterText.data())};
      if (!search.empty() || m_inspectActiveOnly || hasSortSpec) {
        std::vector<s16> sortedList;
        sortedList.reserve(list.size());
        s16 uid = list.GetFirstObjectIndex();
        while (uid != -1) {
          ImGuiEntityEntry& entry = ImGuiConsole::entities[uid];
          if (m_inspectActiveOnly && !entry.active) {
            uid = list.GetNextObjectIndex(uid);
            continue;
          }
          if (!search.empty() && !ContainsCaseInsensitive(entry.type, search) &&
              !ContainsCaseInsensitive(entry.name, search)) {
            uid = list.GetNextObjectIndex(uid);
            continue;
          }
          sortedList.push_back(uid);
          uid = list.GetNextObjectIndex(uid);
        }
        if (hasSortSpec) {
          const auto& spec = sortSpecs->Specs[0];
          if (spec.ColumnUserID == 'id') {
            if (spec.SortDirection == ImGuiSortDirection_Ascending) {
              // no-op
            } else {
              std::sort(sortedList.begin(), sortedList.end(), [&](s16 a, s16 b) { return a < b; });
            }
          } else if (spec.ColumnUserID == 'name') {
            std::sort(sortedList.begin(), sortedList.end(), [&](s16 a, s16 b) {
              int compare = ImGuiConsole::entities[a].name.compare(ImGuiConsole::entities[b].name);
              return spec.SortDirection == ImGuiSortDirection_Ascending ? compare < 0 : compare > 0;
            });
          } else if (spec.ColumnUserID == 'type') {
            std::sort(sortedList.begin(), sortedList.end(), [&](s16 a, s16 b) {
              int compare = ImGuiConsole::entities[a].type.compare(ImGuiConsole::entities[b].type);
              return spec.SortDirection == ImGuiSortDirection_Ascending ? compare < 0 : compare > 0;
            });
          }
        }
        for (const auto& item : sortedList) {
          RenderEntityColumns(ImGuiConsole::entities[item]);
        }
      } else {
        // Render uid ascending
        s16 uid = list.GetFirstObjectIndex();
        while (uid != -1) {
          RenderEntityColumns(ImGuiConsole::entities[uid]);
          uid = list.GetNextObjectIndex(uid);
        }
      }

      ImGui::EndTable();
    }
  }
  ImGui::End();
}

bool ImGuiConsole::ShowEntityInfoWindow(TUniqueId uid) {
  bool open = true;
  ImGuiEntityEntry& entry = ImGuiConsole::entities[uid.Value()];
  auto name = fmt::format(FMT_STRING("{}##{:x}"), !entry.name.empty() ? entry.name : entry.type, uid.Value());
  if (ImGui::Begin(name.c_str(), &open, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::PushID(uid.Value());
    entry.ent->ImGuiInspect();
    ImGui::PopID();
  }
  ImGui::End();
  return open;
}

void ImGuiConsole::ShowAboutWindow() {
  // Center window
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  ImVec4& windowBg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
  ImGui::PushStyleColor(ImGuiCol_TitleBg, windowBg);
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive, windowBg);

  if (ImGui::Begin("About", &m_showAboutWindow,
                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav |
                       ImGuiWindowFlags_NoSavedSettings)) {
    float iconSize = 128.f * ImGui::GetIO().DisplayFramebufferScale.x;
    ImGui::SameLine(ImGui::GetWindowSize().x / 2 - iconSize + (iconSize / 2));
    ImGui::Image(ImGuiUserTextureID_MetaforceIcon, ImVec2{iconSize, iconSize});
    ImGui::PushFont(ImGuiEngine::fontLarge);
    ImGuiTextCenter("Metaforce");
    ImGui::PopFont();
    ImGuiTextCenter(METAFORCE_WC_DESCRIBE);
    const ImVec2& padding = ImGui::GetStyle().WindowPadding;
    ImGui::Dummy(padding);
    ImGuiTextCenter("2015-2021");
    ImGuiTextCenter("Phillip Stephens (Antidote)");
    ImGuiTextCenter("Jack Andersen (jackoalan)");
    ImGuiTextCenter("Luke Street (encounter)");
    ImGuiTextCenter("Metaforce contributors");
    ImGui::Dummy(padding);
    ImGui::Separator();
    if (ImGui::BeginTable("Version Info", 2, ImGuiTableFlags_BordersInnerV)) {
      ImGui::TableNextRow();
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText("Branch");
      }
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText(METAFORCE_WC_BRANCH);
      }
      ImGui::TableNextRow();
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText("Revision");
      }
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText(METAFORCE_WC_REVISION);
      }
      ImGui::TableNextRow();
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText("Build");
      }
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText(METAFORCE_DLPACKAGE);
      }
      ImGui::TableNextRow();
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText("Date");
      }
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText(METAFORCE_WC_DATE);
      }
      ImGui::TableNextRow();
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText("Type");
      }
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText(METAFORCE_BUILD_TYPE);
      }
      ImGui::TableNextRow();
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText("Game");
      }
      if (ImGui::TableNextColumn()) {
        ImGuiStringViewText(g_Main->GetVersionString());
      }
      ImGui::EndTable();
    }
  }
  ImGui::End();
  ImGui::PopStyleColor(2);
}

void ImGuiConsole::ShowDebugOverlay() {
  if (!m_frameCounter && !m_frameRate && !m_inGameTime && !m_roomTimer && !m_playerInfo && !m_areaInfo &&
      !m_worldInfo && !m_randomStats && !m_resourceStats) {
    return;
  }
  ImGuiIO& io = ImGui::GetIO();
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                 ImGuiWindowFlags_NoNav;
  if (m_debugOverlayCorner != -1) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 workPos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
    ImVec2 workSize = viewport->WorkSize;
    ImVec2 windowPos;
    ImVec2 windowPosPivot;
    constexpr float padding = 10.0f;
    windowPos.x = (m_debugOverlayCorner & 1) != 0 ? (workPos.x + workSize.x - padding) : (workPos.x + padding);
    windowPos.y = (m_debugOverlayCorner & 2) != 0 ? (workPos.y + workSize.y - padding) : (workPos.y + padding);
    windowPosPivot.x = (m_debugOverlayCorner & 1) != 0 ? 1.0f : 0.0f;
    windowPosPivot.y = (m_debugOverlayCorner & 2) != 0 ? 1.0f : 0.0f;
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);
    windowFlags |= ImGuiWindowFlags_NoMove;
  }
  ImGui::SetNextWindowBgAlpha(0.65f);
  if (ImGui::Begin("Debug Overlay", nullptr, windowFlags)) {
    bool hasPrevious = false;
    if (m_frameCounter && g_StateManager != nullptr) {
      ImGuiStringViewText(fmt::format(FMT_STRING("Frame: {}\n"), g_StateManager->GetUpdateFrameIndex()));
      hasPrevious = true;
    }
    if (m_frameRate) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      ImGuiStringViewText(fmt::format(FMT_STRING("FPS: {}\n"), metaforce::CGraphics::GetFPS()));
      hasPrevious = true;
    }
    if (m_inGameTime && g_GameState != nullptr) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      double igt = g_GameState->GetTotalPlayTime();
      u32 ms = u64(igt * 1000) % 1000;
      auto pt = std::div(int(igt), 3600);
      ImGuiStringViewText(
          fmt::format(FMT_STRING("Play Time: {:02d}:{:02d}:{:02d}.{:03d}\n"), pt.quot, pt.rem / 60, pt.rem % 60, ms));
      hasPrevious = true;
    }
    if (m_roomTimer && g_StateManager != nullptr) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      double igt = g_GameState->GetTotalPlayTime();
      if (m_currentRoom != g_StateManager->GetCurrentArea()) {
        m_currentRoom = static_cast<const void*>(g_StateManager->GetCurrentArea());
        m_lastRoomTime = igt - m_currentRoomStart;
        m_currentRoomStart = igt;
      }
      double currentRoomTime = igt - m_currentRoomStart;
      u32 curFrames = u32(std::round(u32(currentRoomTime * 60)));
      u32 lastFrames = u32(std::round(u32(m_lastRoomTime * 60)));
      ImGuiStringViewText(fmt::format(FMT_STRING("Room Time: {:7.3f} / {:5d} | Last Room:{:7.3f} / {:5d}\n"),
                                      currentRoomTime, curFrames, m_lastRoomTime, lastFrames));
      hasPrevious = true;
    }
    if (m_playerInfo && g_StateManager != nullptr && g_StateManager->Player() != nullptr) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      const CPlayer& pl = g_StateManager->GetPlayer();
      const zeus::CQuaternion plQ = zeus::CQuaternion(pl.GetTransform().getRotation().buildMatrix3f());
      const zeus::CTransform camXf = g_StateManager->GetCameraManager()->GetCurrentCameraTransform(*g_StateManager);
      const zeus::CQuaternion camQ = zeus::CQuaternion(camXf.getRotation().buildMatrix3f());
      ImGuiStringViewText(
          fmt::format(FMT_STRING("Player Position x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
                                 "       Roll: {: .2f}, Pitch: {: .2f}, Yaw: {: .2f}\n"
                                 "       Momentum x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
                                 "       Velocity x: {: .2f}, y: {: .2f}, z: {: .2f}\n"
                                 "Camera Position x: {: .2f}, y: {: .2f}, z {: .2f}\n"
                                 "       Roll: {: .2f}, Pitch: {: .2f}, Yaw: {: .2f}\n"),
                      pl.GetTranslation().x(), pl.GetTranslation().y(), pl.GetTranslation().z(),
                      zeus::radToDeg(plQ.roll()), zeus::radToDeg(plQ.pitch()), zeus::radToDeg(plQ.yaw()),
                      pl.GetMomentum().x(), pl.GetMomentum().y(), pl.GetMomentum().z(), pl.GetVelocity().x(),
                      pl.GetVelocity().y(), pl.GetVelocity().z(), camXf.origin.x(), camXf.origin.y(), camXf.origin.z(),
                      zeus::radToDeg(camQ.roll()), zeus::radToDeg(camQ.pitch()), zeus::radToDeg(camQ.yaw())));
      hasPrevious = true;
    }
    if (m_worldInfo && g_StateManager != nullptr) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      TLockedToken<CStringTable> tbl =
          g_SimplePool->GetObj({FOURCC('STRG'), g_StateManager->GetWorld()->IGetStringTableAssetId()});
      const metaforce::TAreaId aId = g_GameState->CurrentWorldState().GetCurrentAreaId();
      ImGuiStringViewText(fmt::format(FMT_STRING("World: 0x{}{}, Area: {}\n"), g_GameState->CurrentWorldAssetId(),
                                      (tbl.IsLoaded() ? (" " + hecl::Char16ToUTF8(tbl->GetString(0))).c_str() : ""),
                                      aId));
      hasPrevious = true;
    }
    if (m_areaInfo && g_StateManager != nullptr) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      const metaforce::TAreaId aId = g_GameState->CurrentWorldState().GetCurrentAreaId();
      if (g_StateManager->GetWorld() != nullptr && g_StateManager->GetWorld()->DoesAreaExist(aId)) {
        const auto& layerStates = g_GameState->CurrentWorldState().GetLayerState();
        std::string layerBits;
        u32 totalActive = 0;
        for (int i = 0; i < layerStates->GetAreaLayerCount(aId); ++i) {
          if (layerStates->IsLayerActive(aId, i)) {
            ++totalActive;
            layerBits += "1";
          } else {
            layerBits += "0";
          }
        }
        ImGuiStringViewText(fmt::format(FMT_STRING("Area AssetId: 0x{}, Total Objects: {}\n"
                                                   "Active Layer bits: {}\n"),
                                        g_StateManager->GetWorld()->GetArea(aId)->GetAreaAssetId(),
                                        g_StateManager->GetAllObjectList().size(), layerBits));
        hasPrevious = true;
      }
    }
    if (m_randomStats) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      ImGuiStringViewText(
          fmt::format(FMT_STRING("CRandom16::Next calls: {}\n"), metaforce::CRandom16::GetNumNextCalls()));
      hasPrevious = true;
    }
    if (m_resourceStats) {
      if (hasPrevious) {
        ImGui::Separator();
      }
      ImGuiStringViewText(fmt::format(FMT_STRING("Resource Objects: {}\n"), g_SimplePool->GetLiveObjects()));
    }
    if (ImGui::BeginPopupContextWindow()) {
      if (ImGui::MenuItem("Custom", nullptr, m_debugOverlayCorner == -1)) {
        m_debugOverlayCorner = -1;
      }
      if (ImGui::MenuItem("Top-left", nullptr, m_debugOverlayCorner == 0)) {
        m_debugOverlayCorner = 0;
      }
      if (ImGui::MenuItem("Top-right", nullptr, m_debugOverlayCorner == 1)) {
        m_debugOverlayCorner = 1;
      }
      if (ImGui::MenuItem("Bottom-left", nullptr, m_debugOverlayCorner == 2)) {
        m_debugOverlayCorner = 2;
      }
      if (ImGui::MenuItem("Bottom-right", nullptr, m_debugOverlayCorner == 3)) {
        m_debugOverlayCorner = 3;
      }
      ImGui::EndPopup();
    }
  }
  ImGui::End();
}

void ImGuiConsole::ShowAppMainMenuBar(bool canInspect) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Game")) {
      ShowMenuGame();
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Tools")) {
      ImGui::MenuItem("Inspect", nullptr, &m_showInspectWindow, canInspect);
      ImGui::MenuItem("Items", nullptr, &m_showItemsWindow, canInspect);
      ImGui::MenuItem("Layers", nullptr, &m_showLayersWindow, canInspect);
      ImGui::Separator();
      ImGui::MenuItem("Demo", nullptr, &m_showDemoWindow);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Debug")) {
      if (ImGui::MenuItem("Frame Counter", nullptr, &m_frameCounter)) {
        m_cvarCommons.m_debugOverlayShowFrameCounter->fromBoolean(m_frameCounter);
      }
      if (ImGui::MenuItem("Frame Rate", nullptr, &m_frameRate)) {
        m_cvarCommons.m_debugOverlayShowFramerate->fromBoolean(m_frameRate);
      }
      if (ImGui::MenuItem("In-Game Time", nullptr, &m_inGameTime)) {
        m_cvarCommons.m_debugOverlayShowInGameTime->fromBoolean(m_inGameTime);
      }
      if (ImGui::MenuItem("Room Timer", nullptr, &m_roomTimer)) {
        m_cvarCommons.m_debugOverlayShowRoomTimer->fromBoolean(m_roomTimer);
      }
      if (ImGui::MenuItem("Player Info", nullptr, &m_playerInfo)) {
        m_cvarCommons.m_debugOverlayPlayerInfo->fromBoolean(m_playerInfo);
      }
      if (ImGui::MenuItem("World Info", nullptr, &m_worldInfo)) {
        m_cvarCommons.m_debugOverlayWorldInfo->fromBoolean(m_worldInfo);
      }
      if (ImGui::MenuItem("Area Info", nullptr, &m_areaInfo)) {
        m_cvarCommons.m_debugOverlayAreaInfo->fromBoolean(m_areaInfo);
      }
      if (ImGui::MenuItem("Random Stats", nullptr, &m_randomStats)) {
        m_cvarCommons.m_debugOverlayShowRandomStats->fromBoolean(m_randomStats);
      }
      if (ImGui::MenuItem("Resource Stats", nullptr, &m_resourceStats)) {
        m_cvarCommons.m_debugOverlayShowResourceStats->fromBoolean(m_resourceStats);
      }
      ImGui::EndMenu();
    }
    ImGui::Spacing();
    if (ImGui::BeginMenu("Help")) {
      ImGui::MenuItem("About", nullptr, &m_showAboutWindow);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}

void ImGuiConsole::PreUpdate() {
  if (m_stepFrame) {
    g_Main->SetPaused(true);
    m_stepFrame = false;
  }
  bool canInspect = g_StateManager != nullptr && g_StateManager->GetObjectList();
  ShowAppMainMenuBar(canInspect);
  if (canInspect && (m_showInspectWindow || !inspectingEntities.empty())) {
    UpdateEntityEntries();
    if (m_showInspectWindow) {
      ShowInspectWindow(&m_showInspectWindow);
    }
    auto iter = inspectingEntities.begin();
    while (iter != inspectingEntities.end()) {
      if (!ShowEntityInfoWindow(*iter)) {
        iter = inspectingEntities.erase(iter);
      } else {
        iter++;
      }
    }
  }
  if (canInspect && m_showItemsWindow) {
    ShowItemsWindow();
  }
  if (canInspect && m_showLayersWindow) {
    ShowLayersWindow();
  }
  if (m_showAboutWindow) {
    ShowAboutWindow();
  }
  if (m_showDemoWindow) {
    ImGui::ShowDemoWindow(&m_showDemoWindow);
  }
  ShowDebugOverlay();
}

void ImGuiConsole::PostUpdate() {
  if (g_StateManager != nullptr && g_StateManager->GetObjectList()) {
    // Clear deleted objects
    CObjectList& list = g_StateManager->GetAllObjectList();
    for (s16 uid = 0; uid < s16(entities.size()); uid++) {
      ImGuiEntityEntry& item = entities[uid];
      if (item.uid == kInvalidUniqueId) {
        continue; // already cleared
      }
      CEntity* ent = list.GetObjectByIndex(uid);
      if (ent == nullptr || ent != item.ent) {
        // Remove inspect windows for deleted entities
        inspectingEntities.erase(item.uid);
        item.uid = kInvalidUniqueId;
        item.ent = nullptr; // for safety
      }
    }
  } else {
    entities.fill(ImGuiEntityEntry{});
    inspectingEntities.clear();
  }
}

ImGuiConsole::~ImGuiConsole() {
  dummyWorlds.clear();
  stringTables.clear();
}

static constexpr std::array ItemOrder{
    CPlayerState::EItemType::PowerBeam,
    CPlayerState::EItemType::ChargeBeam,
    CPlayerState::EItemType::IceBeam,
    CPlayerState::EItemType::WaveBeam,
    CPlayerState::EItemType::PlasmaBeam,
    CPlayerState::EItemType::EnergyTanks,
    CPlayerState::EItemType::Missiles,
    CPlayerState::EItemType::SuperMissile,
    CPlayerState::EItemType::Flamethrower,
    CPlayerState::EItemType::IceSpreader,
    CPlayerState::EItemType::Wavebuster,
    CPlayerState::EItemType::CombatVisor,
    CPlayerState::EItemType::ScanVisor,
    CPlayerState::EItemType::ThermalVisor,
    CPlayerState::EItemType::XRayVisor,
    CPlayerState::EItemType::MorphBall,
    CPlayerState::EItemType::MorphBallBombs,
    CPlayerState::EItemType::PowerBombs,
    CPlayerState::EItemType::BoostBall,
    CPlayerState::EItemType::SpiderBall,
    CPlayerState::EItemType::GrappleBeam,
    CPlayerState::EItemType::SpaceJumpBoots,
    CPlayerState::EItemType::PowerSuit,
    CPlayerState::EItemType::VariaSuit,
    CPlayerState::EItemType::GravitySuit,
    CPlayerState::EItemType::PhazonSuit,
    CPlayerState::EItemType::Truth,
    CPlayerState::EItemType::Strength,
    CPlayerState::EItemType::Elder,
    CPlayerState::EItemType::Wild,
    CPlayerState::EItemType::Lifegiver,
    CPlayerState::EItemType::Warrior,
    CPlayerState::EItemType::Chozo,
    CPlayerState::EItemType::Nature,
    CPlayerState::EItemType::Sun,
    CPlayerState::EItemType::World,
    CPlayerState::EItemType::Spirit,
    CPlayerState::EItemType::Newborn,
};

void ImGuiConsole::ShowItemsWindow() {
  CPlayerState& pState = *g_StateManager->GetPlayerState();
  if (ImGui::Begin("Items", &m_showItemsWindow)) {
    if (ImGui::Button("Refill")) {
      for (const auto itemType : ItemOrder) {
        u32 maxValue = CPlayerState::GetPowerUpMaxValue(itemType);
        pState.ResetAndIncrPickUp(itemType, maxValue);
      }
    }
    auto& mapWorldInfo = *g_GameState->CurrentWorldState().MapWorldInfo();
    ImGui::SameLine();
    bool mapStationUsed = mapWorldInfo.GetMapStationUsed();
    if (ImGui::Checkbox("Area map", &mapStationUsed)) {
      mapWorldInfo.SetMapStationUsed(mapStationUsed);
    }
    if (ImGui::Button("All")) {
      for (const auto itemType : ItemOrder) {
        u32 maxValue = CPlayerState::GetPowerUpMaxValue(itemType);
        pState.ReInitializePowerUp(itemType, maxValue);
        pState.ResetAndIncrPickUp(itemType, maxValue);
      }
      mapWorldInfo.SetMapStationUsed(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("None")) {
      for (const auto itemType : ItemOrder) {
        pState.ReInitializePowerUp(itemType, 0);
      }
      mapWorldInfo.SetMapStationUsed(false);
    }
    for (const auto itemType : ItemOrder) {
      u32 maxValue = CPlayerState::GetPowerUpMaxValue(itemType);
      std::string name{CPlayerState::ItemTypeToName(itemType)};
      if (maxValue == 1) {
        bool enabled = pState.GetItemCapacity(itemType) == 1;
        if (ImGui::Checkbox(name.c_str(), &enabled)) {
          if (enabled) {
            pState.ReInitializePowerUp(itemType, 1);
            pState.ResetAndIncrPickUp(itemType, 1);
          } else {
            pState.ReInitializePowerUp(itemType, 0);
          }
          if (itemType == CPlayerState::EItemType::VariaSuit || itemType == CPlayerState::EItemType::PowerSuit ||
              itemType == CPlayerState::EItemType::GravitySuit || itemType == CPlayerState::EItemType::PhazonSuit) {
            g_StateManager->Player()->AsyncLoadSuit(*g_StateManager);
          }
        }
      } else if (maxValue > 1) {
        int capacity = int(pState.GetItemCapacity(itemType));
        if (ImGui::SliderInt(name.c_str(), &capacity, 0, int(maxValue), "%d", ImGuiSliderFlags_AlwaysClamp)) {
          pState.ReInitializePowerUp(itemType, u32(capacity));
          pState.ResetAndIncrPickUp(itemType, u32(capacity));
        }
      }
    }
  }
  ImGui::End();
}

void ImGuiConsole::ShowLayersWindow() {
  if (ImGui::Begin("Layers", &m_showLayersWindow)) {
    for (const auto& world : ListWorlds()) {
      const auto& layers = dummyWorlds[world.second]->GetWorldLayers();
      if (!layers) {
        continue;
      }
      
      if (ImGui::TreeNode(world.first.c_str())) {
        auto worldLayerState = g_GameState->StateForWorld(world.second).GetLayerState();
        u32 startNameIdx = 0;

        for (const auto& area : ListAreas(world.second)) {
          u32 layerCount = worldLayerState->GetAreaLayerCount(area.second);
          if (layerCount == 0) {
            continue;
          }
          if (ImGui::TreeNode(area.first.c_str())) {
            // TODO: m_startNameIdx have incorrect values in the data due to a Metaforce bug
            // u32 startNameIdx = layers->m_areas[area.second].m_startNameIdx;

            for (u32 layer = 0; layer < layerCount; ++layer) {
              bool active = worldLayerState->IsLayerActive(area.second, layer);
              if (ImGui::Checkbox(layers->m_names[startNameIdx + layer].c_str(), &active)) {
                worldLayerState->SetLayerActive(area.second, layer, active);
              }
            }
            ImGui::TreePop();
          }
          startNameIdx += layerCount;
        }
        ImGui::TreePop();
      }
    }
  }
  ImGui::End();
}

} // namespace metaforce
