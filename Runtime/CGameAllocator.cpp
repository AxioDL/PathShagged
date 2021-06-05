#include "Runtime/CGameAllocator.hpp"
#include <new>

namespace metaforce {
logvisor::Module AllocLog("metaforce::CGameAllocator");

#pragma GCC diagnostic ignored "-Wclass-memaccess"

u32 CGameAllocator::GetFreeBinEntryForSize(size_t len) {
  size_t bin = 0;
  size_t binSize = 32;
  while (true) {
    if (binSize > 0x200000) {
      return 15;
    }
    if (len < binSize) {
      break;
    }
    binSize <<= 1;
    ++bin;
  }
  return bin;
}

bool CGameAllocator::Initialize() { return true; }

void* CGameAllocator::Alloc() { return nullptr; }
s32 CGameAllocator::Free(void* ptr) { return 0; }
void CGameAllocator::ReleaseAll() {}
void CGameAllocator::AllocSecondary() {}
void CGameAllocator::FreeSecondary() {}
void CGameAllocator::ReleaseAllSecondary() {}
void CGameAllocator::SetOutOfMemoryCallback() {}
void CGameAllocator::EnumAllocations() {}
IAllocator::SAllocInfo CGameAllocator::GetAllocInfo(void* ptr) {
  SGameMemInfo* info = GetMemInfoFromBlockPtr(ptr);
  return {.x0_infoPtr = info,
          .x4_len = info->x4_len,
          .x8_hasPrevious = info->x10_prev != nullptr,
          .x9_ = false,
          .xc_fileAndLne = info->x8_line,
          .x10_type = info->xc_type};
}
void CGameAllocator::OffsetFakeStatics(s32 offset) { xb8_fakeStatics += offset; }
void CGameAllocator::GetMetrics() {}
} // namespace metaforce
