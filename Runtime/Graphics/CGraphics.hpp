#pragma once

#include <array>
#include <vector>
#include <chrono>
#include "optick.h"

#include "Runtime/RetroTypes.hpp"

#include "DataSpec/DNACommon/GX.hpp"

#include <boo/graphicsdev/IGraphicsCommandQueue.hpp>
#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

#include <hecl/CVar.hpp>
#include <hecl/Runtime.hpp>

#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector2i.hpp>
#include <zeus/CVector2f.hpp>

using frame_clock = std::chrono::high_resolution_clock;

namespace metaforce {
extern hecl::CVar* g_disableLighting;
class CLight;
class CTimeProvider;

enum class ERglLight : u8 { Zero = 0, One, Two, Three, Four, Five, Six, Seven };

enum class ERglLightBits : u8 {
  None = 0,
  Zero = 1,
  One = 1 << 1,
  Two = 1 << 2,
  Three = 1 << 3,
  Four = 1 << 4,
  Five = 1 << 5,
  Six = 1 << 6,
  Seven = 1 << 7
};
ENABLE_BITWISE_ENUM(ERglLightBits)

enum class ERglEnum { Never = 0, Less = 1, Equal = 2, LEqual = 3, Greater = 4, NEqual = 5, GEqual = 6, Always = 7 };

enum class ERglBlendMode { None = 0, Blend = 1, Logic = 2, Subtract = 3 };

enum class ERglBlendFactor {
  Zero = 0,
  One = 1,
  SrcColor = 2,
  InvSrcColor = 3,
  SrcAlpha = 4,
  InvSrcAlpha = 5,
  DstAlpha = 6,
  InvDstAlpha = 7
};

enum class ERglLogicOp {
  Clear = 0,
  And = 1,
  RevAnd = 2,
  Copy = 3,
  InvAnd = 4,
  NoOp = 5,
  Xor = 6,
  Or = 7,
  Nor = 8,
  Equiv = 9,
  Inv = 10,
  RevOr = 11,
  InvCopy = 12,
  InvOr = 13,
  NAnd = 14,
  Set = 15
};

enum class ERglCullMode { None = 0, Front = 1, Back = 2, All = 3 };

enum class ERglAlphaFunc {
  Never = 0,
  Less = 1,
  Equal = 2,
  LEqual = 3,
  Greater = 4,
  NEqual = 5,
  GEqual = 6,
  Always = 7
};

enum class ERglAlphaOp { And = 0, Or = 1, Xor = 2, XNor = 3 };

enum class ERglFogMode : uint32_t {
  None = 0x00,

  PerspLin = 0x02,
  PerspExp = 0x04,
  PerspExp2 = 0x05,
  PerspRevExp = 0x06,
  PerspRevExp2 = 0x07,

  OrthoLin = 0x0A,
  OrthoExp = 0x0C,
  OrthoExp2 = 0x0D,
  OrthoRevExp = 0x0E,
  OrthoRevExp2 = 0x0F
};

struct SViewport {
  u32 x0_left;
  u32 x4_top;
  u32 x8_width;
  u32 xc_height;
  float x10_halfWidth;
  float x14_halfHeight;
  float aspect;
};

extern SViewport g_Viewport;

struct SClipScreenRect {
  bool x0_valid = false;
  int x4_left = 0;
  int x8_top = 0;
  int xc_width = 0;
  int x10_height = 0;
  int x14_dstWidth = 0;
  float x18_uvXMin = 0.f;
  float x1c_uvXMax = 0.f;
  float x20_uvYMin = 0.f;
  float x24_uvYMax = 0.f;

  SClipScreenRect() = default;
  SClipScreenRect(bool valid, int left, int top, int width, int height, int dstWidth, float uvXMin, float uvXMax,
                  float uvYMin, float uvYMax)
  : x0_valid(valid)
  , x4_left(left)
  , x8_top(top)
  , xc_width(width)
  , x10_height(height)
  , x14_dstWidth(dstWidth)
  , x18_uvXMin(uvXMin)
  , x1c_uvXMax(uvXMax)
  , x20_uvYMin(uvYMin)
  , x24_uvYMax(uvYMax) {}

  SClipScreenRect(const boo::SWindowRect& rect) {
    x4_left = rect.location[0];
    x8_top = rect.location[1];
    xc_width = rect.size[0];
    x10_height = rect.size[1];
    x14_dstWidth = rect.size[0];
  }

  SClipScreenRect(const SViewport& vp) {
    x4_left = vp.x0_left;
    x8_top = vp.x4_top;
    xc_width = vp.x8_width;
    x10_height = vp.xc_height;
  }
};

enum class ETexelFormat {
  I4 = 0,
  I8 = 1,
  IA4 = 2,
  IA8 = 3,
  C4 = 4,
  C8 = 5,
  C14X2 = 6,
  RGB565 = 7,
  RGB5A3 = 8,
  RGBA8 = 9,
  CMPR = 10,
  RGBA8PC = 16,
  C8PC = 17,
  CMPRPC = 18,
  CMPRPCA = 19,
};

#define DEPTH_FAR 1.f
#define DEPTH_SKY 0.999f
#define DEPTH_TARGET_MANAGER 0.12500012f
#define DEPTH_WORLD (1.f / 8.f)
#define DEPTH_GUN (1.f / 32.f)
#define DEPTH_SCREEN_ACTORS (1.f / 64.f)
#define DEPTH_HUD (1.f / 512.f)
#define DEPTH_NEAR 0.f
#define CUBEMAP_RES 256
#define CUBEMAP_MIPS 6

static s32 sNextUniquePass = 0;
namespace CTevCombiners {
struct CTevOp {
  bool x0_clamp = true;
  GX::TevOp x4_op = GX::TevOp::TEV_ADD;
  GX::TevBias x8_bias = GX::TevBias::TB_ZERO;
  GX::TevScale xc_scale = GX::TevScale::CS_SCALE_1;
  GX::TevRegID xc_regId = GX::TevRegID::TEVPREV;
};

struct ColorPass {
  GX::TevColorArg x0_a;
  GX::TevColorArg x4_b;
  GX::TevColorArg x8_c;
  GX::TevColorArg xc_d;
};
struct AlphaPass {
  GX::TevAlphaArg x0_a;
  GX::TevAlphaArg x4_b;
  GX::TevAlphaArg x8_c;
  GX::TevAlphaArg xc_d;
};

class CTevPass {
  u32 x0_id;
  ColorPass x4_colorPass;
  AlphaPass x14_alphaPass;
  CTevOp x24_colorOp;
  CTevOp x38_alphaOp;

public:
  CTevPass(const ColorPass& colPass, const AlphaPass& alphaPass, const CTevOp& colorOp = CTevOp(),
           const CTevOp alphaOp = CTevOp())
  : x0_id(++sNextUniquePass)
  , x4_colorPass(colPass)
  , x14_alphaPass(alphaPass)
  , x24_colorOp(colorOp)
  , x38_alphaOp(alphaOp) {}
};
}; // namespace CTevCombiners

class CGraphics {
public:
  struct CProjectionState {
    bool x0_persp;
    float x4_left;
    float x8_right;
    float xc_top;
    float x10_bottom;
    float x14_near;
    float x18_far;
  };

  struct CFogState {
    zeus::CColor m_color;
    float m_A = 0.f;
    float m_B = 0.5f;
    float m_C = 0.f;
    ERglFogMode m_mode;
  };

  static CProjectionState g_Proj;
  static zeus::CVector2f g_CachedDepthRange;
  static CFogState g_Fog;
  static std::array<zeus::CColor, 3> g_ColorRegs;
  static float g_ProjAspect;
  static u32 g_NumLightsActive;
  static u32 g_NumBreakpointsWaiting;
  static u32 g_FlippingState;
  static bool g_LastFrameUsedAbove;
  static bool g_InterruptLastFrameUsedAbove;
  static ERglLightBits g_LightActive;
  static ERglLightBits g_LightsWereOn;
  static zeus::CTransform g_GXModelView;
  static zeus::CTransform g_GXModelViewInvXpose;
  static zeus::CTransform g_GXModelMatrix;
  static zeus::CTransform g_ViewMatrix;
  static zeus::CVector3f g_ViewPoint;
  static zeus::CTransform g_GXViewPointMatrix;
  static zeus::CTransform g_CameraMatrix;
  static SClipScreenRect g_CroppedViewport;
  static bool g_IsGXModelMatrixIdentity;

  static void DisableAllLights();
  static void LoadLight(ERglLight light, const CLight& info);
  static void EnableLight(ERglLight light);
  static void SetLightState(ERglLightBits lightState);
  static void SetAmbientColor(const zeus::CColor& col);
  static void SetFog(ERglFogMode mode, float startz, float endz, const zeus::CColor& color);
  static void SetDepthWriteMode(bool test, ERglEnum comp, bool write);
  static void SetBlendMode(ERglBlendMode, ERglBlendFactor, ERglBlendFactor, ERglLogicOp);
  static void SetCullMode(ERglCullMode);
  static void BeginScene();
  static void EndScene();
  static void SetAlphaCompare(ERglAlphaFunc comp0, u8 ref0, ERglAlphaOp op, ERglAlphaFunc comp1, u8 ref1);
  static void SetViewPointMatrix(const zeus::CTransform& xf);
  static void SetViewMatrix();
  static void SetModelMatrix(const zeus::CTransform& xf);
  static zeus::CMatrix4f CalculatePerspectiveMatrix(float fovy, float aspect, float znear, float zfar,
                                                    bool forRenderer);
  static zeus::CMatrix4f GetPerspectiveProjectionMatrix(bool forRenderer);
  static const CProjectionState& GetProjectionState();
  static void SetProjectionState(const CProjectionState&);
  static void SetPerspective(float fovy, float aspect, float znear, float zfar);
  static void SetOrtho(float left, float right, float top, float bottom, float znear, float zfar);
  static void FlushProjection();
  static zeus::CVector2i ProjectPoint(const zeus::CVector3f& point);
  static SClipScreenRect ClipScreenRectFromMS(const zeus::CVector3f& p1, const zeus::CVector3f& p2);
  static SClipScreenRect ClipScreenRectFromVS(const zeus::CVector3f& p1, const zeus::CVector3f& p2);
  static zeus::CVector3f ProjectModelPointToViewportSpace(const zeus::CVector3f& point);
  static zeus::CVector3f ProjectModelPointToViewportSpace(const zeus::CVector3f& point, float& wOut);

  static void SetViewportResolution(const zeus::CVector2i& res);
  static void SetViewport(int leftOff, int bottomOff, int width, int height);
  static void SetScissor(int leftOff, int bottomOff, int width, int height);
  static void SetDepthRange(float near, float far);

  static CTimeProvider* g_ExternalTimeProvider;
  static float g_DefaultSeconds;
  static u32 g_RenderTimings;
  static void SetExternalTimeProvider(CTimeProvider* provider) { g_ExternalTimeProvider = provider; }
  static float GetSecondsMod900();
  static void TickRenderTimings();
  static u32 g_FrameCounter;
  static u32 g_Framerate;
  static u32 g_FramesPast;
  static frame_clock::time_point g_FrameStartTime;
  static u32 GetFrameCounter() { return g_FrameCounter; }
  static u32 GetFPS() { return g_Framerate; }
  static void UpdateFPSCounter();

  static boo::IGraphicsDataFactory::Platform g_BooPlatform;
  static const boo::SystemChar* g_BooPlatformName;
  static boo::IGraphicsDataFactory* g_BooFactory;
  static boo::IGraphicsCommandQueue* g_BooMainCommandQueue;
  static boo::ObjToken<boo::ITextureR> g_SpareTexture;

  static const std::array<zeus::CMatrix3f, 6> skCubeBasisMats;

  static void InitializeBoo(boo::IGraphicsDataFactory* factory, boo::IGraphicsCommandQueue* cc,
                            const boo::ObjToken<boo::ITextureR>& spareTex) {
    g_BooPlatform = factory->platform();
    g_BooPlatformName = factory->platformName();
    g_BooFactory = factory;
    g_BooMainCommandQueue = cc;
    g_SpareTexture = spareTex;
  }

  static void ShutdownBoo() {
    g_BooFactory = nullptr;
    g_BooMainCommandQueue = nullptr;
    g_SpareTexture.reset();
  }

  static const boo::SystemChar* PlatformName() { return g_BooPlatformName; }


  static bool g_commitAsLazy;
  static void SetCommitResourcesAsLazy(bool newStatus) {
    if (newStatus != g_commitAsLazy) {
      g_commitAsLazy = newStatus;
      if (!newStatus && g_BooFactory) {
        g_BooFactory->commitPendingTransaction();
      }
    }
  }

  static void CommitResources(const boo::FactoryCommitFunc& commitFunc __BooTraceArgs) {
    CommitResources(commitFunc __BooTraceArgsUse, g_commitAsLazy);
  }

  static void CommitResources(const boo::FactoryCommitFunc& commitFunc __BooTraceArgs, bool lazy) {
    if (!g_BooFactory) {
      return;
    }
    if (lazy) {
      g_BooFactory->lazyCommitTransaction(commitFunc __BooTraceArgsUse);
    } else {
      g_BooFactory->commitTransaction(commitFunc __BooTraceArgsUse);
    }
  }

  static void SetShaderDataBinding(const boo::ObjToken<boo::IShaderDataBinding>& binding) {
    g_BooMainCommandQueue->setShaderDataBinding(binding);
  }
  static void ResolveSpareTexture(const SClipScreenRect& rect, int bindIdx = 0, bool clearDepth = false) {
    boo::SWindowRect wrect = {rect.x4_left, rect.x8_top, rect.xc_width, rect.x10_height};
    g_BooMainCommandQueue->resolveBindTexture(g_SpareTexture, wrect, true, bindIdx, true, false, clearDepth);
  }
  static void ResolveSpareDepth(const SClipScreenRect& rect, int bindIdx = 0) {
    boo::SWindowRect wrect = {rect.x4_left, rect.x8_top, rect.xc_width, rect.x10_height};
    g_BooMainCommandQueue->resolveBindTexture(g_SpareTexture, wrect, true, bindIdx, false, true);
  }
  static void DrawInstances(size_t start, size_t count, size_t instCount, size_t startInst = 0) {
    g_BooMainCommandQueue->drawInstances(start, count, instCount, startInst);
  }
  static void DrawArray(size_t start, size_t count) { g_BooMainCommandQueue->draw(start, count); }
  static void DrawArrayIndexed(size_t start, size_t count) { g_BooMainCommandQueue->drawIndexed(start, count); }

  static const CTevCombiners::CTevPass sTevPass805a564c;
  static const CTevCombiners::CTevPass sTevPass805a5698;

  static const CTevCombiners::CTevPass sTevPass805a5e70;

  static const CTevCombiners::CTevPass sTevPass805a5ebc;

  static const CTevCombiners::CTevPass sTevPass805a5f08;

  static const CTevCombiners::CTevPass sTevPass805a5f54;

  static const CTevCombiners::CTevPass sTevPass805a5fa0;

  static const CTevCombiners::CTevPass sTevPass804bfcc0;

  static const CTevCombiners::CTevPass sTevPass805a5fec;

  static const CTevCombiners::CTevPass sTevPass805a6038;

  static const CTevCombiners::CTevPass sTevPass805a6084;
};

template <class VTX>
class TriFanToStrip {
  std::vector<VTX>& m_vec;
  size_t m_start;
  size_t m_added = 0;

public:
  explicit TriFanToStrip(std::vector<VTX>& vec) : m_vec(vec), m_start(vec.size()) {}

  void AddVert(const VTX& vert) {
    ++m_added;
    if (m_added > 3 && (m_added & 1) == 0) {
      m_vec.reserve(m_vec.size() + 3);
      m_vec.push_back(m_vec.back());
      m_vec.push_back(m_vec[m_start]);
    }
    m_vec.push_back(vert);
  }

  template <class... _Args>
  void EmplaceVert(_Args&&... args) {
    ++m_added;
    if (m_added > 3 && (m_added & 1) == 0) {
      m_vec.reserve(m_vec.size() + 3);
      m_vec.push_back(m_vec.back());
      m_vec.push_back(m_vec[m_start]);
    }
    m_vec.emplace_back(std::forward<_Args>(args)...);
  }

  void Draw() const { CGraphics::DrawArray(m_start, m_vec.size() - m_start); }
};

#ifdef BOO_GRAPHICS_DEBUG_GROUPS
class GraphicsDebugGroup {
  /* Stack only */
  void* operator new(size_t);
  void operator delete(void*);
  void* operator new[](size_t);
  void operator delete[](void*);

public:
  explicit GraphicsDebugGroup(const char* name, const zeus::CColor& color = zeus::skWhite) {
    zeus::simd_floats f(color.mSimd);
    CGraphics::g_BooMainCommandQueue->pushDebugGroup(name, f.array());
  }
  ~GraphicsDebugGroup() { CGraphics::g_BooMainCommandQueue->popDebugGroup(); }
};
#define SCOPED_GRAPHICS_DEBUG_GROUP(...) GraphicsDebugGroup _GfxDbg_(__VA_ARGS__);
#else
#define SCOPED_GRAPHICS_DEBUG_GROUP(name, ...) OPTICK_EVENT_DYNAMIC(name)
#endif

} // namespace metaforce
