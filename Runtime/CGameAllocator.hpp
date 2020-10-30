#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CCallStack {
  const char* x0_line;
  const char* x4_type;

public:
  CCallStack(int lineNum, const char* lineStr, const char* type) : x0_line(lineStr), x4_type(type) {}
};

struct SAllocInfo {};

enum class EHint {
  Unk = (1 << 0),
  RoundUpLen = (1 << 1),
};

ENABLE_BITWISE_ENUM(EHint);

enum class EScope {

};

enum class EType {

};

class IAllocator {
public:
  virtual bool Initialize() = 0; // const COSContext& ctx) = 0;

  virtual void* Alloc() = 0;
  virtual s32 Free(void* ptr) = 0;
  virtual void ReleaseAll() = 0;
  virtual void AllocSecondary() = 0;
  virtual void FreeSecondary() = 0;
  virtual void ReleaseAllSecondary() = 0;
  virtual void SetOutOfMemoryCallback() = 0;
  virtual void EnumAllocations() = 0;
  virtual SAllocInfo GetAllocInfo() = 0;
  virtual void sub80351138() = 0;
  virtual void GetMetrics() = 0;
};

class CGameAllocator : public IAllocator {
  struct SGameMemInfo {
    u32 x0_sentinel = 0xefefefef;
    size_t x4_len = 0;
    const char* x8_line;
    const char* xc_type;
    SGameMemInfo* x10_prev = nullptr;
    void* x14_ = nullptr;
    void* x18_ = nullptr;
    u32 x1c_canary = 0xeaeaeaea;
  };
  static u32 GetFreeBinEntryForSize(size_t len);
  std::array<SGameMemInfo*, 16> x14_bins;

public:
  bool Initialize(); // const COsContext& ctx);
};
} // namespace metaforce

