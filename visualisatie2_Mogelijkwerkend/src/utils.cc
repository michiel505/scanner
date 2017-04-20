#include "libs.h"
#include "utils.h"
#include "timer.h"

static Timer timer;

namespace Utils
{
    /*
        Check for matching of a prefix
    */
  bool PrefixMatched(const char* prefix, const char* str)
  {
    int m = strlen(prefix);
    int n = strlen(str);
    if (m>n) {
      return false;
    }
    for (int i=0; i<m; ++i) {
      if (prefix[i] != str[i]) {
        return false;
      }
    }
    return true;
  }
  /*
      Delay function
  */
  void Delay(const int64_t micros)
  {
    timer.Reset();
    timer.Wait(micros);
//    DEBUG_PRINT("Waiting %ld microseconds...", micros);
  }
}
