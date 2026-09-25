#pragma once
#include <stdint.h>
struct LightBand {
  int minute, level;
};
inline bool due(uint32_t now, uint32_t deadline) {
  return (int32_t)(now - deadline) >= 0;
}
inline int brightnessForMinute(const LightBand *bands, int count, int minute,
                               int fallback) {
  if (!count)
    return fallback;
  int best = -1, latest = -1;
  for (int i = 0; i < count; ++i) {
    if (latest < 0 || bands[i].minute > bands[latest].minute)
      latest = i;
    if (bands[i].minute <= minute &&
        (best < 0 || bands[i].minute > bands[best].minute))
      best = i;
  }
  return bands[best < 0 ? latest : best].level;
}
// NTP era 0/1, valid for the supported calendar range 2024–2099.
inline uint64_t unixFromNtp(uint32_t seconds) {
  uint64_t result = seconds;
  if (seconds < 2208988800UL)
    result += 4294967296ULL;
  return result - 2208988800ULL;
}
