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
SAllocInfo CGameAllocator::GetAllocInfo() { return SAllocInfo(); }
void CGameAllocator::sub80351138() {}
void CGameAllocator::GetMetrics() {}
} // namespace metaforce
