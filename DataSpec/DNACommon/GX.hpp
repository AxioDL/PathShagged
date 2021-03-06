#pragma once

#include <cstdint>

#include <athena/DNA.hpp>

namespace GX {
enum AttrType { NONE, DIRECT, INDEX8, INDEX16 };

enum TevColorArg {
  CC_CPREV = 0,
  CC_APREV = 1,
  CC_C0 = 2,
  CC_A0 = 3,
  CC_C1 = 4,
  CC_A1 = 5,
  CC_C2 = 6,
  CC_A2 = 7,
  CC_TEXC = 8,
  CC_TEXA = 9,
  CC_RASC = 10,
  CC_RASA = 11,
  CC_ONE = 12,
  CC_HALF = 13,
  CC_KONST = 14,
  CC_ZERO = 15,
};

enum TevAlphaArg {
  CA_APREV = 0,
  CA_A0 = 1,
  CA_A1 = 2,
  CA_A2 = 3,
  CA_TEXA = 4,
  CA_RASA = 5,
  CA_KONST = 6,
  CA_ZERO = 7,
};

enum TevKColorSel {
  TEV_KCSEL_8_8 = 0x00,
  TEV_KCSEL_7_8 = 0x01,
  TEV_KCSEL_6_8 = 0x02,
  TEV_KCSEL_5_8 = 0x03,
  TEV_KCSEL_4_8 = 0x04,
  TEV_KCSEL_3_8 = 0x05,
  TEV_KCSEL_2_8 = 0x06,
  TEV_KCSEL_1_8 = 0x07,

  TEV_KCSEL_1 = TEV_KCSEL_8_8,
  TEV_KCSEL_3_4 = TEV_KCSEL_6_8,
  TEV_KCSEL_1_2 = TEV_KCSEL_4_8,
  TEV_KCSEL_1_4 = TEV_KCSEL_2_8,

  TEV_KCSEL_K0 = 0x0C,
  TEV_KCSEL_K1 = 0x0D,
  TEV_KCSEL_K2 = 0x0E,
  TEV_KCSEL_K3 = 0x0F,
  TEV_KCSEL_K0_R = 0x10,
  TEV_KCSEL_K1_R = 0x11,
  TEV_KCSEL_K2_R = 0x12,
  TEV_KCSEL_K3_R = 0x13,
  TEV_KCSEL_K0_G = 0x14,
  TEV_KCSEL_K1_G = 0x15,
  TEV_KCSEL_K2_G = 0x16,
  TEV_KCSEL_K3_G = 0x17,
  TEV_KCSEL_K0_B = 0x18,
  TEV_KCSEL_K1_B = 0x19,
  TEV_KCSEL_K2_B = 0x1A,
  TEV_KCSEL_K3_B = 0x1B,
  TEV_KCSEL_K0_A = 0x1C,
  TEV_KCSEL_K1_A = 0x1D,
  TEV_KCSEL_K2_A = 0x1E,
  TEV_KCSEL_K3_A = 0x1F
};

enum TevKAlphaSel {
  TEV_KASEL_8_8 = 0x00,
  TEV_KASEL_7_8 = 0x01,
  TEV_KASEL_6_8 = 0x02,
  TEV_KASEL_5_8 = 0x03,
  TEV_KASEL_4_8 = 0x04,
  TEV_KASEL_3_8 = 0x05,
  TEV_KASEL_2_8 = 0x06,
  TEV_KASEL_1_8 = 0x07,

  TEV_KASEL_1 = TEV_KASEL_8_8,
  TEV_KASEL_3_4 = TEV_KASEL_6_8,
  TEV_KASEL_1_2 = TEV_KASEL_4_8,
  TEV_KASEL_1_4 = TEV_KASEL_2_8,

  TEV_KASEL_K0_R = 0x10,
  TEV_KASEL_K1_R = 0x11,
  TEV_KASEL_K2_R = 0x12,
  TEV_KASEL_K3_R = 0x13,
  TEV_KASEL_K0_G = 0x14,
  TEV_KASEL_K1_G = 0x15,
  TEV_KASEL_K2_G = 0x16,
  TEV_KASEL_K3_G = 0x17,
  TEV_KASEL_K0_B = 0x18,
  TEV_KASEL_K1_B = 0x19,
  TEV_KASEL_K2_B = 0x1A,
  TEV_KASEL_K3_B = 0x1B,
  TEV_KASEL_K0_A = 0x1C,
  TEV_KASEL_K1_A = 0x1D,
  TEV_KASEL_K2_A = 0x1E,
  TEV_KASEL_K3_A = 0x1F
};

enum TevOp {
  TEV_ADD = 0,
  TEV_SUB = 1,
  TEV_COMP_R8_GT = 8,
  TEV_COMP_R8_EQ = 9,
  TEV_COMP_GR16_GT = 10,
  TEV_COMP_GR16_EQ = 11,
  TEV_COMP_BGR24_GT = 12,
  TEV_COMP_BGR24_EQ = 13,
  TEV_COMP_RGB8_GT = 14,
  TEV_COMP_RGB8_EQ = 15,
  TEV_COMP_A8_GT = TEV_COMP_RGB8_GT,
  TEV_COMP_A8_EQ = TEV_COMP_RGB8_EQ
};

enum TevBias {
  TB_ZERO = 0,
  TB_ADDHALF = 1,
  TB_SUBHALF = 2,
};

enum TevScale { CS_SCALE_1 = 0, CS_SCALE_2 = 1, CS_SCALE_4 = 2, CS_DIVIDE_2 = 3 };

enum TexGenType {
  TG_MTX3x4 = 0,
  TG_MTX2x4,
  TG_BUMP0,
  TG_BUMP1,
  TG_BUMP2,
  TG_BUMP3,
  TG_BUMP4,
  TG_BUMP5,
  TG_BUMP6,
  TG_BUMP7,
  TG_SRTG
};

enum TexGenSrc {
  TG_POS = 0,
  TG_NRM,
  TG_BINRM,
  TG_TANGENT,
  TG_TEX0,
  TG_TEX1,
  TG_TEX2,
  TG_TEX3,
  TG_TEX4,
  TG_TEX5,
  TG_TEX6,
  TG_TEX7,
  TG_TEXCOORD0,
  TG_TEXCOORD1,
  TG_TEXCOORD2,
  TG_TEXCOORD3,
  TG_TEXCOORD4,
  TG_TEXCOORD5,
  TG_TEXCOORD6,
  TG_COLOR0,
  TG_COLOR1
};

enum TexMtx {
  TEXMTX0 = 30,
  TEXMTX1 = 33,
  TEXMTX2 = 36,
  TEXMTX3 = 39,
  TEXMTX4 = 42,
  TEXMTX5 = 45,
  TEXMTX6 = 48,
  TEXMTX7 = 51,
  TEXMTX8 = 54,
  TEXMTX9 = 57,
  IDENTITY = 60
};

enum PTTexMtx {
  PTTEXMTX0 = 64,
  PTTEXMTX1 = 67,
  PTTEXMTX2 = 70,
  PTTEXMTX3 = 73,
  PTTEXMTX4 = 76,
  PTTEXMTX5 = 79,
  PTTEXMTX6 = 82,
  PTTEXMTX7 = 85,
  PTTEXMTX8 = 88,
  PTTEXMTX9 = 91,
  PTTEXMTX10 = 94,
  PTTEXMTX11 = 97,
  PTTEXMTX12 = 100,
  PTTEXMTX13 = 103,
  PTTEXMTX14 = 106,
  PTTEXMTX15 = 109,
  PTTEXMTX16 = 112,
  PTTEXMTX17 = 115,
  PTTEXMTX18 = 118,
  PTTEXMTX19 = 121,
  PTIDENTITY = 125
};

enum TevRegID { TEVPREV = 0, TEVREG0 = 1, TEVREG1 = 2, TEVREG2 = 3, TEVLAZY = 5 };

enum DiffuseFn { DF_NONE = 0, DF_SIGN, DF_CLAMP };

enum AttnFn { AF_SPEC = 0, AF_SPOT = 1, AF_NONE };

enum Primitive {
  POINTS = 0xb8,
  LINES = 0xa8,
  LINESTRIP = 0xb0,
  TRIANGLES = 0x90,
  TRIANGLESTRIP = 0x98,
  TRIANGLEFAN = 0xa0,
  QUADS = 0x80
};

enum ChannelID {
  GX_COLOR0,
  GX_COLOR1,
  GX_ALPHA0,
  GX_ALPHA1,
  GX_COLOR0A0,
  GX_COLOR1A1,
  GX_COLOR_ZERO,
  GX_ALPHA_BUMP,
  GX_ALPHA_BUMPN,
  GX_COLOR_NULL = 0xff
};

enum BlendFactor : uint16_t {
  BL_ZERO,
  BL_ONE,
  BL_SRCCLR,
  BL_INVSRCCLR,
  BL_SRCALPHA,
  BL_INVSRCALPHA,
  BL_DSTALPHA,
  BL_INVDSTALPHA
};

struct Color : athena::io::DNA<athena::Endian::Big> {
  union {
    uint8_t color[4];
    uint32_t num = 0;
  };
  Color() = default;
  Color& operator=(const atVec4f& vec) {
    athena::simd_floats f(vec.simd);
    color[0] = uint8_t(std::min(std::max(f[0] * 255.f, 0.f), 255.f));
    color[1] = uint8_t(std::min(std::max(f[1] * 255.f, 0.f), 255.f));
    color[2] = uint8_t(std::min(std::max(f[2] * 255.f, 0.f), 255.f));
    color[3] = uint8_t(std::min(std::max(f[3] * 255.f, 0.f), 255.f));
    return *this;
  }
  Color& operator=(const atVec3f& vec) {
    athena::simd_floats f(vec.simd);
    color[0] = uint8_t(std::min(std::max(f[0] * 255.f, 0.f), 255.f));
    color[1] = uint8_t(std::min(std::max(f[1] * 255.f, 0.f), 255.f));
    color[2] = uint8_t(std::min(std::max(f[2] * 255.f, 0.f), 255.f));
    color[3] = 0xff;
    return *this;
  }
  Color& operator=(uint8_t val) {
    color[0] = val;
    color[1] = val;
    color[2] = val;
    color[3] = val;
    return *this;
  }
  atVec4f toVec4f() const {
    atVec4f out;
    athena::simd_floats f;
    f[0] = color[0] / 255.f;
    f[1] = color[1] / 255.f;
    f[2] = color[2] / 255.f;
    f[3] = color[3] / 255.f;
    out.simd.copy_from(f);
    return out;
  }
  Color(const atVec4f& vec) { *this = vec; }
  Color(const atVec3f& vec) { *this = vec; }
  Color(uint8_t val) { *this = val; }
  bool operator==(const Color& other) const { return num == other.num; }
  bool operator!=(const Color& other) const { return num != other.num; }
  uint8_t operator[](size_t idx) const { return color[idx]; }
  uint8_t& operator[](size_t idx) { return color[idx]; }
  AT_DECL_EXPLICIT_DNA
};

} // namespace GX
