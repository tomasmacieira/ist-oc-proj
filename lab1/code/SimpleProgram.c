#include "SimpleCache.h"

int main() {

  uint32_t value1, value2, clock;

  resetTime();
  initCache();
  value1 = -1;
  value2 = 0;

  write(1, (uint8_t *)(&value1));

  clock = getTime();
  printf("Time: %d\n", clock);

  read(1, (uint8_t *)(&value2));
  clock = getTime();
  printf("Time: %d\n", clock);

  write(512, (uint8_t *)(&value1));
  clock = getTime();
  printf("Time: %d\n", clock);

  read(512, (uint8_t *)(&value2));
  clock = getTime();
  printf("Time: %d\n", clock);

  return 0;
}
