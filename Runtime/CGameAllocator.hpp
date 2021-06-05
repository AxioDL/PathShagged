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
  struct SAllocInfo {
    void* x0_infoPtr;
    size_t x4_len;
    bool x8_hasPrevious;
    bool x9_;
    const char* xc_fileAndLne;
    const char* x10_type;
  };

  virtual bool Initialize() = 0; // const COSContext& ctx) = 0;

  virtual void* Alloc(size_t size) = 0;
  virtual bool Free(void* ptr) = 0;
  virtual void ReleaseAll() = 0;
  virtual void* AllocSecondary(size_t size) = 0;
  virtual bool FreeSecondary(void* ptr) = 0;
  virtual void ReleaseAllSecondary() = 0;
  virtual void SetOutOfMemoryCallback() = 0;
  virtual void EnumAllocations() = 0;
  virtual SAllocInfo GetAllocInfo(void* ptr) = 0;
  virtual void OffsetFakeStatics(s32 offset) = 0;
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
  SGameMemInfo* GetMemInfoFromBlockPtr(void* ptr) {
    return reinterpret_cast<SGameMemInfo*>(reinterpret_cast<void*>(intptr_t(ptr) - sizeof(SGameMemInfo)));
  }
  std::array<SGameMemInfo*, 16> x14_bins;

  s32 xb8_fakeStatics = 0;

public:
  bool Initialize() override; // const COsContext& ctx);
  void* Alloc(size_t size) override;
  bool Free(void* ptr) override;
  void ReleaseAll() override;
  void* AllocSecondary(size_t size) override;
  bool FreeSecondary(void* ptr) override;
  void ReleaseAllSecondary() override;
  void SetOutOfMemoryCallback() override;
  void EnumAllocations() override;
  SAllocInfo GetAllocInfo(void* ptr) override;
  void OffsetFakeStatics(s32 offset) override;
  void GetMetrics() override;
};
} // namespace metaforce
