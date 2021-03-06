#pragma once

#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class CEnergyDrainSource {
  TUniqueId x0_source;
  float x4_intensity;

public:
  CEnergyDrainSource(TUniqueId src, float intensity);
  TUniqueId GetEnergyDrainSourceId() const { return x0_source; }
  void SetEnergyDrainIntensity(float in) { x4_intensity = in; }
  float GetEnergyDrainIntensity() const { return x4_intensity; }
};
} // namespace metaforce
