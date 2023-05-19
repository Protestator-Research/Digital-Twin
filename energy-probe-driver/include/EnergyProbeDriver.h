#pragma once


#ifdef _WIN32
  #define ENERGYPROBEDRIVER_EXPORT __declspec(dllexport)
#else
  #define ENERGYPROBEDRIVER_EXPORT
#endif

ENERGYPROBEDRIVER_EXPORT void EnergyProbeDriver();
