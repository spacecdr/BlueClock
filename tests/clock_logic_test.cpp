#include "../src/clock_logic.h"
#include <cassert>
#include <cstdio>
int main() {
  LightBand bands[] = {{1320, 20}, {420, 100}, {1140, 60}};
  assert(brightnessForMinute(bands, 3, 0, 50) == 20);
  assert(brightnessForMinute(bands, 3, 419, 50) == 20);
  assert(brightnessForMinute(bands, 3, 420, 50) == 100);
  assert(brightnessForMinute(bands, 3, 1139, 50) == 100);
  assert(brightnessForMinute(bands, 3, 1140, 50) == 60);
  assert(brightnessForMinute(bands, 3, 1320, 50) == 20);
  assert(brightnessForMinute(bands, 3, 1439, 50) == 20);
  assert(brightnessForMinute(bands, 0, 100, 75) == 75);
  LightBand single[] = {{900, 0}};
  assert(brightnessForMinute(single, 1, 0, 100) == 0);
  assert(brightnessForMinute(single, 1, 1000, 100) == 0);
  assert(!due(0xfffffff0u, 0x20u));
  assert(due(0x20u, 0x20u));
  assert(due(0x30u, 0x20u));
  assert(unixFromNtp(3913056000UL) == 1704067200ULL);
  assert(unixFromNtp(0xffffffffUL) == 2085978495ULL);
  assert(unixFromNtp(0) == 2085978496ULL);
  puts("PASS: brightness boundaries, midnight wrap, timer wrap, NTP era "
       "rollover");
}
