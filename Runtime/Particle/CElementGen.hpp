#pragma once

#include <array>
#include <vector>

#include "Runtime/CRandom16.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CLight.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"
#include "Runtime/Graphics/Shaders/CElementGenShaders.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CParticleGen.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CActorLights;
class CGenDescription;
class CLight;
class CParticleElectric;
class CParticleSwoosh;
class CWarp;
class IGenDescription;

class CElementGen : public CParticleGen {
  static u16 g_GlobalSeed;
  static bool g_subtractBlend;

public:
  static void SetGlobalSeed(u16 seed) { g_GlobalSeed = seed; }
  static void SetSubtractBlend(bool subtract) { g_subtractBlend = subtract; }
  enum class EModelOrientationType { Normal, One };
  enum class EOptionalSystemFlags { None, One, Two };
  enum class LightType { None = 0, Custom = 1, Directional = 2, Spot = 3 };
  class CParticleListItem {
    friend class CElementGen;
    s16 x0_partIdx;
    zeus::CVector3f x4_viewPoint;

  public:
    explicit CParticleListItem(s16 idx) : x0_partIdx(idx) {}
  };
  static CParticle* g_currentParticle;

private:
  friend class CElementGenShaders;
  TLockedToken<CGenDescription> x1c_genDesc;
  CGenDescription* x28_loadedGenDesc;
  EModelOrientationType x2c_orientType;
  std::vector<CParticle> x30_particles;
  std::vector<u32> x40;
  std::vector<zeus::CMatrix3f> x50_parentMatrices;
  std::vector<std::array<float, 8>> x60_advValues;

  int x70_internalStartFrame = 0;
  int x74_curFrame = 0;
  double x78_curSeconds = 0.f;
  float x80_timeDeltaScale = 0.f;
  int x84_prevFrame = -1;
  bool x88_particleEmission = true;
  float x8c_generatorRemainder = 0.f;
  int x90_MAXP = 0;
  u16 x94_randomSeed = g_GlobalSeed;
  float x98_generatorRate = 1.f;
  std::array<float, 16> x9c_externalVars{};

  zeus::CVector3f xdc_translation;
  zeus::CVector3f xe8_globalTranslation;
  zeus::CVector3f xf4_POFS;
  zeus::CVector3f x100_globalScale = {1.f, 1.f, 1.f};
  zeus::CTransform x10c_globalScaleTransform = zeus::CTransform();
  zeus::CTransform x13c_globalScaleTransformInverse = zeus::CTransform();
  zeus::CVector3f x16c_localScale = {1.f, 1.f, 1.f};
  zeus::CTransform x178_localScaleTransform = zeus::CTransform();
  zeus::CTransform x1a8_localScaleTransformInverse = zeus::CTransform();
  zeus::CTransform x1d8_orientation = zeus::CTransform();
  zeus::CMatrix3f x208_orientationInverse = zeus::CMatrix3f();
  zeus::CTransform x22c_globalOrientation = zeus::CTransform();

  u32 x25c_activeParticleCount = 0;
  u32 x260_cumulativeParticles = 0;
  u32 x264_recursiveParticleCount = 0;
  int x268_PSLT;
  bool x26c_24_translationDirty : 1 = false;
  bool x26c_25_LIT_ : 1 = false;
  bool x26c_26_AAPH : 1 = false;
  bool x26c_27_ZBUF : 1 = false;
  bool x26c_28_zTest : 1 = false;
  bool x26c_29_ORNT : 1 = false;
  bool x26c_30_MBLR : 1 = false;
  bool x26c_31_LINE : 1 = false;
  bool x26d_24_FXLL : 1 = false;
  bool x26d_25_warmedUp : 1 = false;
  bool x26d_26_modelsUseLights : 1 = false;
  bool x26d_27_enableOPTS : 1;
  bool x26d_28_enableADV : 1 = false;
  int x270_MBSP = 0;
  ERglLightBits x274_backupLightActive = ERglLightBits::None;
  std::array<bool, 4> x278_hasVMD{};
  CRandom16 x27c_randState;
  std::array<CModVectorElement*, 4> x280_VELSources{};

  std::vector<std::unique_ptr<CParticleGen>> x290_activePartChildren;
  int x2a0_CSSD = 0;
  int x2a4_SISY = 16;
  int x2a8_PISY = 16;
  int x2ac_SSSD = 0;
  zeus::CVector3f x2b0_SSPO;
  int x2bc_SESD = 0;
  zeus::CVector3f x2c0_SEPO;
  float x2cc = 0.f;
  float x2d0 = 0.f;
  zeus::CVector3f x2d4_aabbMin;
  zeus::CVector3f x2e0_aabbMax;
  float x2ec_maxSize = 0.f;
  zeus::CAABox x2f0_systemBounds = zeus::CAABox();
  LightType x308_lightType;
  zeus::CColor x30c_LCLR = zeus::skWhite;
  float x310_LINT = 1.f;
  zeus::CVector3f x314_LOFF;
  zeus::CVector3f x320_LDIR = {1.f, 0.f, 0.f};
  EFalloffType x32c_falloffType = EFalloffType::Linear;
  float x330_LFOR = 1.f;
  float x334_LSLA = 45.f;
  zeus::CColor x338_moduColor = {1.f, 1.f, 1.f, 1.f};

  std::unique_ptr<CLineRenderer> m_lineRenderer;
  CElementGenShaders::EShaderClass m_shaderClass;

  void AccumulateBounds(const zeus::CVector3f& pos, float size);

  void _RecreatePipelines();
public:
  explicit CElementGen(TToken<CGenDescription> gen, EModelOrientationType orientType = EModelOrientationType::Normal,
                       EOptionalSystemFlags flags = EOptionalSystemFlags::One);
  ~CElementGen() override;

  std::array<boo::ObjToken<boo::IShaderDataBinding>, 2> m_normalDataBind;
  std::array<boo::ObjToken<boo::IShaderDataBinding>, 2> m_normalSubDataBind;
  std::array<boo::ObjToken<boo::IShaderDataBinding>, 2> m_redToAlphaDataBind;
  std::array<boo::ObjToken<boo::IShaderDataBinding>, 2> m_redToAlphaSubDataBind;
  boo::ObjToken<boo::IGraphicsBufferD> m_instBuf;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniformBuf;

  std::array<boo::ObjToken<boo::IShaderDataBinding>, 2> m_normalDataBindPmus;
  std::array<boo::ObjToken<boo::IShaderDataBinding>, 2> m_redToAlphaDataBindPmus;
  boo::ObjToken<boo::IGraphicsBufferD> m_instBufPmus;
  boo::ObjToken<boo::IGraphicsBufferD> m_uniformBufPmus;

  CGenDescription* GetDesc() { return x1c_genDesc.GetObj(); }
  const SObjectTag* GetDescTag() const { return x1c_genDesc.GetObjectTag(); }
  CGenDescription* GetLoadedDesc() { return x28_loadedGenDesc; }

  static bool g_ParticleSystemInitialized;
  static int g_ParticleAliveCount;
  static int g_ParticleSystemAliveCount;
  static bool sMoveRedToAlphaBuffer;
  static void Initialize();
  static void Shutdown();

  void UpdateAdvanceAccessParameters(u32 activeParticleCount, s32 particleFrame);
  bool UpdateVelocitySource(size_t idx, s32 particleFrame, CParticle& particle);
  void UpdateExistingParticles();
  void CreateNewParticles(int count);
  void UpdatePSTranslationAndOrientation();
  void UpdateChildParticleSystems(double dt);
  std::unique_ptr<CParticleGen> ConstructChildParticleSystem(const TToken<CGenDescription>& desc) const;
  void UpdateLightParameters();
  void BuildParticleSystemBounds();
  u32 GetEmitterTime() const { return x74_curFrame; }
  u32 GetSystemCount() const;
  u32 GetCumulativeParticleCount() const { return x260_cumulativeParticles; }
  u32 GetParticleCountAllInternal() const;
  u32 GetParticleCountAll() const { return x264_recursiveParticleCount; }
  void EndLifetime();
  void ForceParticleCreation(int amount);
  float GetExternalVar(int index) const { return x9c_externalVars[index]; }
  void SetExternalVar(int index, float var) { x9c_externalVars[index] = var; }

  bool InternalUpdate(double dt);
  void RenderModels(const CActorLights* actLights);
  void RenderLines();
  void RenderParticles();
  void RenderParticlesIndirectTexture();

  bool Update(double t) override;
  void Render(const CActorLights* actorLights = nullptr) override;
  void SetOrientation(const zeus::CTransform& orientation) override;
  void SetTranslation(const zeus::CVector3f& translation) override;
  void SetGlobalOrientation(const zeus::CTransform& orientation) override;
  void SetGlobalTranslation(const zeus::CVector3f& translation) override;
  void SetGlobalScale(const zeus::CVector3f& scale) override;
  void SetLocalScale(const zeus::CVector3f& scale) override;
  void SetGlobalOrientAndTrans(const zeus::CTransform& xf);
  void SetParticleEmission(bool enabled) override;
  void SetModulationColor(const zeus::CColor& color) override;
  void SetGeneratorRate(float rate) override;
  const zeus::CTransform& GetOrientation() const override;
  const zeus::CVector3f& GetTranslation() const override;
  const zeus::CTransform& GetGlobalOrientation() const override;
  const zeus::CVector3f& GetGlobalTranslation() const override;
  const zeus::CVector3f& GetGlobalScale() const override;
  const zeus::CColor& GetModulationColor() const override;
  float GetGeneratorRate() const override { return x98_generatorRate; }
  bool IsSystemDeletable() const override;
  std::optional<zeus::CAABox> GetBounds() const override;
  u32 GetParticleCount() const override;
  bool SystemHasLight() const override;
  CLight GetLight() const override;
  bool GetParticleEmission() const override;
  void DestroyParticles() override;
  void Reset() override;
  FourCC Get4CharId() const override { return FOURCC('PART'); }
  size_t GetNumActiveChildParticles() const { return x290_activePartChildren.size(); }
  CParticleGen& GetActiveChildParticle(size_t idx) const { return *x290_activePartChildren[idx]; }
  bool IsIndirectTextured() const { return x28_loadedGenDesc->x54_x40_TEXR && x28_loadedGenDesc->x58_x44_TIND; }
  void SetModelsUseLights(bool useLights) { x26d_26_modelsUseLights = useLights; }
  void SetZTest(bool z) { x26c_28_zTest = z; }
  static void SetMoveRedToAlphaBuffer(bool move);

  s32 GetMaxParticles() const { return x90_MAXP; }

  std::vector<CParticle> const& GetParticles() const { return x30_particles; }
  std::vector<CParticle> &GetParticles() { return x30_particles; }
};
ENABLE_BITWISE_ENUM(CElementGen::EOptionalSystemFlags)

} // namespace metaforce
