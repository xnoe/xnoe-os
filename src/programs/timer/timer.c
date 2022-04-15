#include "common/common.h"

int main() {
  uint32_t crashBin = fopen("/crash.bin");
  while (1) {
    printf("Time Elapsed: %dms\n", getMillisecondsElapsed());
    printf("Init. Pages: %d\nRemaining Pages: %d\n", getInitPages(), getRemainingPages());
    exec(crashBin);
    sleep(1000);
  }
}