#pragma once

#include <zeus/CAABox.hpp>
#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CMarkerGrid {
private:
  zeus::CAABox x0_bounds;
  zeus::CVector3f x18_gridUnits;
  rstl::reserved_vector<u8, 0x400> x24_gridState;

public:
  CMarkerGrid(const zeus::CAABox& bounds);
  void MarkCells(const zeus::CSphere& area, u32 val);
  bool GetCoords(const zeus::CVector3f& vec, u32& x, u32& y, u32& z) const;
  u32 GetValue(u32 x, u32 y, u32 z) const;
  void SetValue(u32 x, u32 y, u32 z, u32 val);

  bool AABoxTouchesData(const zeus::CAABox& box, u32 val) const;
  zeus::CVector3f GetWorldPositionForCell(u32 x, u32 y, u32 z) const;
  const zeus::CAABox& GetBounds() const { return x0_bounds; }
};
} // namespace urde