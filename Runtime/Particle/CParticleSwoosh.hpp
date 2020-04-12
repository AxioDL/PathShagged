#pragma once

#include <array>
#include <memory>
#include <vector>

#include "DataSpec/DNACommon/GX.hpp"

#include "Runtime/CRandom16.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/Shaders/CParticleSwooshShaders.hpp"
#include "Runtime/Particle/CParticleGen.hpp"
#include "Runtime/Particle/CUVElement.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CSwooshDescription;

class CParticleSwoosh : public CParticleGen {
  friend class CParticleSwooshShaders;
public:
  struct SSwooshData {
    bool x0_active;
    float x4_leftRad;
    float x8_rightRad;
    zeus::CVector3f xc_translation;   // Updated by system's velocity sources or user code
    zeus::CVector3f x18_offset;       // Updated by POFS once per system update (also resets x24_useOffset)
    zeus::CVector3f x24_useOffset;    // Combination of POFS and NPOS, once per particle instance
    float x30_irot;                   // Rotation bias once per system update
    float x34_rotm;                   // Rotation bias once per particle instance
    zeus::CTransform x38_orientation; // Updated by user code
    int x68_frame = 0;                // Frame index of evaluated data
    zeus::CColor x6c_color;           // Updated by COLR
    int x70_startFrame;
    zeus::CVector3f x74_velocity;

    SSwooshData(const zeus::CVector3f& translation, const zeus::CVector3f& offset, float irot, float rotm,
                int startFrame, bool active, const zeus::CTransform& orient, const zeus::CVector3f& vel, float leftRad,
                float rightRad, const zeus::CColor& color)
    : x0_active(active)
    , x4_leftRad(leftRad)
    , x8_rightRad(rightRad)
    , xc_translation(translation)
    , x18_offset(offset)
    , x24_useOffset(offset)
    , x30_irot(irot)
    , x34_rotm(rotm)
    , x38_orientation(orient)
    , x6c_color(color)
    , x70_startFrame(startFrame)
    , x74_velocity(vel) {}
  };

private:
  TLockedToken<CSwooshDescription> x1c_desc;
  u32 x28_curFrame = 0;
  int x2c_PSLT = 0;
  double x30_curTime = 0.0;
  zeus::CVector3f x38_translation;
  zeus::CTransform x44_orientation;
  zeus::CTransform x74_invOrientation;
  zeus::CVector3f xa4_globalTranslation;
  zeus::CTransform xb0_globalOrientation;
  zeus::CVector3f xe0_globalScale = {1.f, 1.f, 1.f};
  zeus::CTransform xec_scaleXf;
  zeus::CTransform x11c_invScaleXf;
  zeus::CVector3f x14c_localScale = {1.f, 1.f, 1.f};
  u32 x158_curParticle = 0;
  std::vector<SSwooshData> x15c_swooshes;
  std::vector<zeus::CVector3f> x16c_p0;
  std::vector<zeus::CVector3f> x17c_p1;
  std::vector<zeus::CVector3f> x18c_p2;
  std::vector<zeus::CVector3f> x19c_p3;
  u32 x1ac_particleCount = 0;
  int x1b0_SPLN = 0;
  int x1b4_LENG = 0;
  int x1b8_SIDE = 0;
  GX::Primitive x1bc_prim{};
  CRandom16 x1c0_rand;
  float x1c4_ = 0.f;
  float x1c8_ = 0.f;
  float x1cc_TSPN = 0.f;
  bool x1d0_24_emitting : 1 = true;
  bool x1d0_25_AALP : 1 = false;
  bool x1d0_26_forceOneUpdate : 1 = false;
  bool x1d0_27_renderGaps : 1 = false;
  bool x1d0_28_LLRD : 1 = false;
  bool x1d0_29_VLS1 : 1 = false;
  bool x1d0_30_VLS2 : 1 = false;
  bool x1d0_31_constantTex : 1 = false;
  bool x1d1_24_constantUv : 1 = false;

  SUVElementSet x1d4_uvs = {};
  CTexture* x1e4_tex = nullptr;
  float x1e8_uvSpan = 1.f;
  int x1ec_TSPN = 0;
  zeus::CVector3f x1f0_aabbMin;
  zeus::CVector3f x1fc_aabbMax;
  float x208_maxRadius = 0.f;
  zeus::CColor x20c_moduColor = zeus::skWhite;

  std::array<boo::ObjToken<boo::IShaderDataBinding>, 2> m_dataBind;
  boo::ObjToken<boo::IGraphicsBufferD> m_vertBuf;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniformBuf;
  std::unique_ptr<CLineRenderer> m_lineRenderer;
  std::vector<CParticleSwooshShaders::Vert> m_cachedVerts;

  static int g_ParticleSystemAliveCount;

  bool IsValid() const { return x1b4_LENG >= 2 && x1b8_SIDE >= 2; }
  void UpdateMaxRadius(float r);
  void UpdateBounds(const zeus::CVector3f& pos);
  float GetLeftRadius(size_t i) const;
  float GetRightRadius(size_t i) const;
  void UpdateSwooshTranslation(const zeus::CVector3f& translation);
  void UpdateTranslationAndOrientation();

  static zeus::CVector3f GetSplinePoint(const zeus::CVector3f& p0, const zeus::CVector3f& p1, const zeus::CVector3f& p2,
                                        const zeus::CVector3f& p3, float t);
  int WrapIndex(int i) const;
  void RenderNSidedSpline();
  void RenderNSidedNoSpline();
  void Render3SidedSolidSpline();
  void Render3SidedSolidNoSplineNoGaps();
  void Render2SidedSpline();
  void Render2SidedNoSplineGaps();
  void Render2SidedNoSplineNoGaps();

public:
  CParticleSwoosh(const TToken<CSwooshDescription>& desc, int);
  ~CParticleSwoosh() override;

  CSwooshDescription* GetDesc() { return x1c_desc.GetObj(); }

  bool Update(double) override;
  void Render(const CActorLights* = nullptr) override;
  void SetOrientation(const zeus::CTransform&) override;
  void SetTranslation(const zeus::CVector3f&) override;
  void SetGlobalOrientation(const zeus::CTransform&) override;
  void SetGlobalTranslation(const zeus::CVector3f&) override;
  void SetGlobalScale(const zeus::CVector3f&) override;
  void SetLocalScale(const zeus::CVector3f&) override;
  void SetParticleEmission(bool) override;
  void SetModulationColor(const zeus::CColor&) override;
  const zeus::CTransform& GetOrientation() const override;
  const zeus::CVector3f& GetTranslation() const override;
  const zeus::CTransform& GetGlobalOrientation() const override;
  const zeus::CVector3f& GetGlobalTranslation() const override;
  const zeus::CVector3f& GetGlobalScale() const override;
  const zeus::CColor& GetModulationColor() const override;
  bool IsSystemDeletable() const override;
  std::optional<zeus::CAABox> GetBounds() const override;
  u32 GetParticleCount() const override;
  bool SystemHasLight() const override;
  CLight GetLight() const override;
  bool GetParticleEmission() const override;
  void DestroyParticles() override;
  void Reset() override {}
  FourCC Get4CharId() const override { return FOURCC('SWHC'); }
  void SetRenderGaps(bool r) { x1d0_27_renderGaps = r; }
  size_t GetSwooshDataCount() const { return x15c_swooshes.size(); }
  SSwooshData& GetSwooshData(size_t idx) { return x15c_swooshes[idx]; }
  const SSwooshData& GetSwooshData(size_t idx) const { return x15c_swooshes[idx]; }
  std::vector<SSwooshData>& GetSwooshVector() { return x15c_swooshes; }
  const std::vector<SSwooshData>& GetSwooshVector() const { return x15c_swooshes; }

  void DoWarmupUpdate() {
    x1d0_26_forceOneUpdate = true;
    Update(0.0);
  }

  void DoElectricWarmup() {
    for (size_t i = 0; i < x15c_swooshes.size(); ++i) {
      x1d0_26_forceOneUpdate = true;
      Update(0.0);
    }
  }

  void DoElectricCreate(const std::vector<zeus::CVector3f>& offsets) {
    u32 curIdx = x158_curParticle;
    for (size_t i = 0; i < x15c_swooshes.size(); ++i) {
      curIdx = u32((curIdx + 1) % x15c_swooshes.size());
      x15c_swooshes[curIdx].xc_translation = offsets[i];
    }
  }

  void DoGrappleWarmup() {
    for (size_t i = 0; i < x15c_swooshes.size() - 1; ++i) {
      x1d0_26_forceOneUpdate = true;
      Update(0.0);
    }
  }

  void DoGrappleUpdate(const zeus::CVector3f& beamGunPos, const zeus::CTransform& rotation, float anglePhase,
                       float xAmplitude, float zAmplitude, const zeus::CVector3f& swooshSegDelta) {
    float rot = x15c_swooshes.back().x30_irot;
    zeus::CVector3f trans = beamGunPos;
    for (size_t i = 0; i < x15c_swooshes.size(); ++i) {
      SSwooshData& data = x15c_swooshes[i];
      zeus::CVector3f vec;
      if (i > 0)
        vec = rotation * zeus::CVector3f(std::cos(i + anglePhase) * xAmplitude, 0.f, std::sin(float(i)) * zAmplitude);
      data.xc_translation = trans + vec;
      trans += swooshSegDelta;
      std::swap(rot, data.x30_irot);
    }
  }

  void DoSpiderBallWarmup(zeus::CVector3f& translation, const zeus::CVector3f& transInc) {
    SetOrientation(zeus::lookAt(zeus::skZero3f, transInc));
    for (int i = 0; i < 6; ++i) {
      SetTranslation(translation);
      x1d0_26_forceOneUpdate = true;
      Update(0.0);
      translation += transInc;
    }
  }

  void ForceOneUpdate(float dt) {
    x1d0_26_forceOneUpdate = true;
    Update(dt);
  }
  std::vector<SSwooshData> const& GetSwooshes() const { return x15c_swooshes; }
  std::vector<SSwooshData>& GetSwooshes() { return x15c_swooshes; }
  u32 GetCurParticle() const { return x158_curParticle; }
  static u32 GetAliveParticleSystemCount() { return g_ParticleSystemAliveCount; }
};

} // namespace metaforce
