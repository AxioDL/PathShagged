#include "Runtime/World/CMarkerGrid.hpp"

namespace metaforce {
CMarkerGrid::CMarkerGrid(const zeus::CAABox& bounds) : x0_bounds(bounds) {
  x18_gridUnits = zeus::CVector3f((bounds.max - bounds.min) * 0.0625f);
  x24_gridState.resize(0x400);
}

void CMarkerGrid::MarkCells(const zeus::CSphere& area, u32 val) {
  int width_units = static_cast<int>((area.radius - x18_gridUnits.x()) / x18_gridUnits.x());
  int length_units = static_cast<int>((area.radius - x18_gridUnits.y()) / x18_gridUnits.y());
  int height_units = static_cast<int>((area.radius - x18_gridUnits.z()) / x18_gridUnits.z());
  u32 x_coord, y_coord, z_coord;
  if (!GetCoords(area.position, x_coord, y_coord, z_coord)) {
    return;
  }
  for (int i = width_units - z_coord; i < (z_coord + width_units); i++) {
    for (int j = length_units - y_coord; j < (y_coord + length_units); j++) {
      for (int k = height_units - x_coord; k < (z_coord + height_units); k++) {
        u32 new_cell_val = val | GetValue(x_coord, y_coord, z_coord);
        SetValue(k, j, i, new_cell_val);
      }
    }
  }
}

bool CMarkerGrid::GetCoords(zeus::CVector3f const& vec, u32& x, u32& y, u32& z) const {
  if (x0_bounds.pointInside(vec)) {
    x = static_cast<u32>((vec.x() - x0_bounds.min.x()) / x);
    y = static_cast<u32>((vec.y() - x0_bounds.min.y()) / y);
    z = static_cast<u32>((vec.z() - x0_bounds.min.z()) / z);
    return true;
  }
  return false;
}

u32 CMarkerGrid::GetValue(u32 x, u32 y, u32 z) const {
  const u32 bit_offset = (x & 3) << 1;
  u8 marker_byte = x24_gridState[(z << 6) + (y << 2) + (x >> 2)];
  return static_cast<u32>((marker_byte & (3 << bit_offset)) >> bit_offset);
}

void CMarkerGrid::SetValue(u32 x, u32 y, u32 z, u32 val) {
  const u32 bit_offset = (x & 3) << 1;
  const u32 grid_offset = (z << 6) + (y << 2) + (x >> 2);
  u8 marker_byte = x24_gridState[grid_offset];
  marker_byte |= (marker_byte & ~(3 << bit_offset)) | (val << bit_offset);
  x24_gridState[grid_offset] = marker_byte;
}

bool CMarkerGrid::AABoxTouchesData(const zeus::CAABox& box, u32 val) const {
  if (!x0_bounds.intersects(box)) {
    return false;
  }
  zeus::CAABox in_box = box;
  if (!box.inside(x0_bounds)) {
    zeus::CVector3f max_of_min(x0_bounds.min.x() > box.min.x() ? x0_bounds.min.x() : box.min.x(),
                               x0_bounds.min.y() > box.min.y() ? x0_bounds.min.y() : box.min.y(),
                               x0_bounds.min.z() > box.min.z() ? x0_bounds.min.z() : box.min.z());
    zeus::CVector3f min_of_max(x0_bounds.max.x() < box.max.x() ? x0_bounds.max.x() : box.max.x(),
                               x0_bounds.max.y() < box.max.y() ? x0_bounds.max.y() : box.max.y(),
                               x0_bounds.max.z() < box.max.z() ? x0_bounds.max.z() : box.max.z());
    in_box = zeus::CAABox(max_of_min, min_of_max);
  }
  u32 c1x, c1y, c1z, c2x, c2y, c2z;
  GetCoords(in_box.min, c1x, c1y, c1z);
  GetCoords(in_box.max, c2x, c2y, c2z);

  for (int i = c1z; i < c2z; i++) {
    for (int j = c1y; j < c2y; j++) {
      for (int k = c1x; k < c2x; k++) {
        if ((GetValue(k, j, i) & val) != 0u) {
          return true;
        }
      }
    }
  }
  return false;
}

zeus::CVector3f CMarkerGrid::GetWorldPositionForCell(u32 x, u32 y, u32 z) const {
  // returns the center of a given cell
  return zeus::CVector3f(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)) * x18_gridUnits +
         x0_bounds.min + (x18_gridUnits / 2.f);
}
} // namespace urde