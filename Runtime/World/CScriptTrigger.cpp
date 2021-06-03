#include "Runtime/World/CScriptTrigger.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CPlayer.hpp"
#include <logvisor/logvisor.hpp>
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {
namespace {
logvisor::Module Log("CScriptTrigger");
}
CScriptTrigger::CScriptTrigger(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                               const zeus::CVector3f& pos, const zeus::CAABox& bounds, const CDamageInfo& dInfo,
                               const zeus::CVector3f& forceField, ETriggerFlags triggerFlags, bool active,
                               bool deactivateOnEntered, bool deactivateOnExited)
: CActor(uid, active, name, info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(EMaterialTypes::Trigger), CActorParameters::None(), kInvalidUniqueId)
, x100_damageInfo(dInfo)
, x11c_forceField(forceField)
, x128_forceMagnitude(forceField.magnitude())
, x12c_flags(triggerFlags)
, x130_bounds(bounds)
, x148_26_deactivateOnEntered(deactivateOnEntered)
, x148_27_deactivateOnExited(deactivateOnExited) {
  SetCallTouch(false);
#ifdef NDEBUG
  // FIXME: HACK This fixes the HotE softlock, definitely need to look into the morphball's collision codepath and
  // FIXME: determine the proper fix
  if (GetEditorId() == 0x0034004B) {
    Log.report(logvisor::Warning, FMT_STRING("BUG THIS!: Overriding forceField.x() for trigger {} in area {}"),
               GetEditorId(), GetAreaIdAlways());
    //x11c_forceField.x() = 0.f;
  }
#endif
}

void CScriptTrigger::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptTrigger::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    UpdateInhabitants(dt, mgr);
  }
}

void CScriptTrigger::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (GetActive() && (msg == EScriptObjectMessage::Deactivate || msg == EScriptObjectMessage::Deleted)) {
    if (msg == EScriptObjectMessage::Deactivate) {
      xe8_inhabitants.clear();
      x148_25_camSubmerged = false;
    }

    if (x148_28_playerTriggerProc) {
      x148_28_playerTriggerProc = false;
      if (x148_29_didPhazonDamage) {
        mgr.Player()->DecrementEnvironmentDamage();
        x148_29_didPhazonDamage = false;
      }

      if (x8_uid == mgr.GetLastTriggerId()) {
        mgr.SetLastTriggerId(kInvalidUniqueId);
      }
    }
  }

  CActor::AcceptScriptMsg(msg, uid, mgr);
}

CScriptTrigger::CObjectTracker* CScriptTrigger::FindObject(TUniqueId id) {
  auto& inhabitants = GetInhabitants();
  const auto iter = std::find(inhabitants.begin(), inhabitants.end(), CObjectTracker{id});

  if (iter == inhabitants.end()) {
    return nullptr;
  }

  return &*iter;
}

void CScriptTrigger::UpdateInhabitants(float dt, CStateManager& mgr) {
  bool sendInside = false;
  bool sendExited = false;
  std::list<CObjectTracker>::iterator nextIt;
  for (auto it = xe8_inhabitants.begin(); it != xe8_inhabitants.end(); it = nextIt) {
    nextIt = it;
    ++nextIt;
    if (const TCastToPtr<CActor> act = mgr.ObjectById(it->GetObjectId())) {
      bool playerValid = true;
      if (it->GetObjectId() == mgr.GetPlayer().GetUniqueId()) {
        if (False(x12c_flags & ETriggerFlags::DetectPlayer) &&
            ((mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed &&
              True(x12c_flags & ETriggerFlags::DetectUnmorphedPlayer)) ||
             (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed &&
              True(x12c_flags & ETriggerFlags::DetectMorphedPlayer)))) {
          playerValid = false;
        }
        if (!playerValid) {
          xe8_inhabitants.erase(it);
          sendExited = true;
          if (x148_28_playerTriggerProc) {
            x148_28_playerTriggerProc = false;
            if (x148_29_didPhazonDamage) {
              mgr.GetPlayer().DecrementEnvironmentDamage();
              x148_29_didPhazonDamage = false;
            }

            if (mgr.GetLastTriggerId() == GetUniqueId()) {
              mgr.SetLastTriggerId(kInvalidUniqueId);
            }
          }

          InhabitantExited(*act, mgr);
          continue;
        }
      }

      const auto touchBounds = GetTouchBounds();
      const auto actTouchBounds = act->GetTouchBounds();
      if (touchBounds && actTouchBounds && touchBounds->intersects(*actTouchBounds)) {
        sendInside = true;
        InhabitantIdle(*act, mgr);
        if (act->HealthInfo(mgr) && x100_damageInfo.GetDamage() > 0.f) {
          mgr.ApplyDamage(GetUniqueId(), act->GetUniqueId(), GetUniqueId(), {x100_damageInfo, dt},
                          CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
        }

        if (x128_forceMagnitude > 0.f) {
          if (TCastToPtr<CPhysicsActor> pact = act.GetPtr()) {
            float forceMult = 1.f;
            if (True(x12c_flags & ETriggerFlags::UseBooleanIntersection)) {
              forceMult = touchBounds->booleanIntersection(*actTouchBounds).volume() / actTouchBounds->volume();
            }

            const zeus::CVector3f force = forceMult * x11c_forceField;
            if (True(x12c_flags & ETriggerFlags::UseCollisionImpulses)) {
              pact->ApplyImpulseWR(force, zeus::CAxisAngle());
              pact->UseCollisionImpulses();
            } else {
              pact->ApplyForceWR(force, zeus::CAxisAngle());
            }
          }
        }
      } else {
        const TUniqueId tmpId = it->GetObjectId();
        xe8_inhabitants.erase(it);
        sendExited = true;
        if (mgr.GetPlayer().GetUniqueId() == tmpId && x148_28_playerTriggerProc) {
          x148_28_playerTriggerProc = false;
          if (x148_29_didPhazonDamage) {
            mgr.Player()->DecrementEnvironmentDamage();
            x148_29_didPhazonDamage = false;
          }

          if (mgr.GetLastTriggerId() == GetUniqueId())
            mgr.SetLastTriggerId(kInvalidUniqueId);
        }

        InhabitantExited(*act, mgr);
      }
    } else {
      const TUniqueId tmpId = it->GetObjectId();
      xe8_inhabitants.erase(it);
      if (mgr.GetPlayer().GetUniqueId() == tmpId && x148_28_playerTriggerProc) {
        x148_28_playerTriggerProc = false;
        if (x148_29_didPhazonDamage) {
          mgr.Player()->DecrementEnvironmentDamage();
          x148_29_didPhazonDamage = false;
        }

        if (mgr.GetLastTriggerId() == GetUniqueId()) {
          mgr.SetLastTriggerId(kInvalidUniqueId);
        }
      }
    }
  }

  if (True(x12c_flags & ETriggerFlags::DetectCamera) || x148_24_detectCamera) {
    CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    const bool camInTrigger = GetTriggerBoundsWR().pointInside(cam->GetTranslation());
    if (x148_25_camSubmerged) {
      if (!camInTrigger) {
        x148_25_camSubmerged = false;
        if (True(x12c_flags & ETriggerFlags::DetectCamera)) {
          sendExited = true;
          InhabitantExited(*cam, mgr);
        }
      } else {
        if (True(x12c_flags & ETriggerFlags::DetectCamera)) {
          InhabitantIdle(*cam, mgr);
          sendInside = true;
        }
      }
    } else {
      if (camInTrigger) {
        x148_25_camSubmerged = true;
        if (True(x12c_flags & ETriggerFlags::DetectCamera)) {
          InhabitantAdded(*cam, mgr);
          SendScriptMsgs(EScriptObjectState::Entered, mgr, EScriptObjectMessage::None);
        }
      }
    }
  }

  if (sendInside) {
    SendScriptMsgs(EScriptObjectState::Inside, mgr, EScriptObjectMessage::None);
  }

  if (sendExited) {
    SendScriptMsgs(EScriptObjectState::Exited, mgr, EScriptObjectMessage::None);
    if (x148_27_deactivateOnExited) {
      mgr.SendScriptMsg(GetUniqueId(), mgr.GetEditorIdForUniqueId(GetUniqueId()), EScriptObjectMessage::Deactivate,
                        EScriptObjectState::Exited);
    }
  }
}

std::list<CScriptTrigger::CObjectTracker>& CScriptTrigger::GetInhabitants() { return xe8_inhabitants; }

std::optional<zeus::CAABox> CScriptTrigger::GetTouchBounds() const {
  if (x30_24_active) {
    return GetTriggerBoundsWR();
  }
  return std::nullopt;
}
constexpr auto sktonOHurtWeaponMode = CWeaponMode(EWeaponType::Power, false, false, true);

void CScriptTrigger::Touch(CActor& act, CStateManager& mgr) {
  if (!act.GetActive() || act.GetMaterialList().HasMaterial(EMaterialTypes::Trigger)) {
    return;
  }

  if (FindObject(act.GetUniqueId()) == nullptr) {
    auto testFlags = ETriggerFlags::None;
    const TCastToPtr<CPlayer> pl(act);
    if (pl) {
      if (x128_forceMagnitude > 0.f && True(x12c_flags & ETriggerFlags::DetectPlayer) &&
          mgr.GetLastTriggerId() == kInvalidUniqueId) {
        mgr.SetLastTriggerId(x8_uid);
      }

      testFlags |= ETriggerFlags::DetectPlayer;
      if (pl->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed) {
        testFlags |= ETriggerFlags::DetectUnmorphedPlayer;
      } else if (pl->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
        testFlags |= ETriggerFlags::DetectMorphedPlayer;
      }
    } else if (TCastToPtr<CAi>(act)) {
      testFlags |= ETriggerFlags::DetectAI;
    } else if (TCastToPtr<CGameProjectile>(act)) {
      testFlags |= ETriggerFlags::DetectProjectiles1 | ETriggerFlags::DetectProjectiles2 |
                   ETriggerFlags::DetectProjectiles3 | ETriggerFlags::DetectProjectiles4 |
                   ETriggerFlags::DetectProjectiles5 | ETriggerFlags::DetectProjectiles6 |
                   ETriggerFlags::DetectProjectiles7;
    } else if (const TCastToConstPtr<CWeapon> weap = act) {
      if ((weap->GetAttribField() & EProjectileAttrib::Bombs) == EProjectileAttrib::Bombs) {
        testFlags |= ETriggerFlags::DetectBombs;
      } else if ((weap->GetAttribField() & EProjectileAttrib::PowerBombs) == EProjectileAttrib::PowerBombs) {
        testFlags |= ETriggerFlags::DetectPowerBombs;
      }
    }

    if (True(testFlags & x12c_flags)) {
      xe8_inhabitants.emplace_back(act.GetUniqueId());
      InhabitantAdded(act, mgr);

      if (pl) {
        if (!x148_28_playerTriggerProc) {
          x148_28_playerTriggerProc = true;
          if (x148_29_didPhazonDamage) {
            mgr.Player()->DecrementEnvironmentDamage();
            x148_29_didPhazonDamage = false;
          } else if (x100_damageInfo.GetDamage() > 0.f) {
            const CDamageVulnerability* dVuln = mgr.Player()->GetDamageVulnerability();
            if (dVuln->WeaponHurts(x100_damageInfo.GetWeaponMode(), false) &&
                x100_damageInfo.GetWeaponMode().GetType() == EWeaponType::Phazon &&
                !mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PhazonSuit)) {
              pl->IncrementEnvironmentDamage();
              x148_29_didPhazonDamage = true;
            }
          }
        }
      }

      SendScriptMsgs(EScriptObjectState::Entered, mgr, EScriptObjectMessage::None);

      if (x148_26_deactivateOnEntered) {
        mgr.SendScriptMsg(x8_uid, mgr.GetEditorIdForUniqueId(x8_uid), EScriptObjectMessage::Deactivate,
                          EScriptObjectState::Entered);
        if (act.HealthInfo(mgr) && x100_damageInfo.GetDamage() > 0.f) {
          mgr.ApplyDamage(x8_uid, act.GetUniqueId(), x8_uid, x100_damageInfo,
                          CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {0ull}), zeus::skZero3f);
        }
      }

      if (True(x12c_flags & ETriggerFlags::KillOnEnter) && act.HealthInfo(mgr)) {
        CHealthInfo* hInfo = act.HealthInfo(mgr);
        mgr.ApplyDamage(x8_uid, act.GetUniqueId(), x8_uid, {sktonOHurtWeaponMode, 10.f * hInfo->GetHP(), 0.f, 0.f},
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {0ull}), zeus::skZero3f);
      }
    } else {
      InhabitantRejected(act, mgr);
    }
  }
}

zeus::CAABox CScriptTrigger::GetTriggerBoundsWR() const {
  return {x130_bounds.min + x34_transform.origin, x130_bounds.max + x34_transform.origin};
}

void CScriptTrigger::DebugDraw() {
  if (m_debugSelected || m_debugHovered) {
    m_debugBox.setAABB(GetTriggerBoundsWR());
    m_debugBox.draw(m_debugAddColor);
  }
}
} // namespace metaforce
