#include "aos/libc/aos_strsignal.h"

#include <csignal>

#include "aos/thread_local.h"
#include "glog/logging.h"

const char *aos_strsignal(int signal) {
  AOS_THREAD_LOCAL char buffer[512];

  if (signal >= SIGRTMIN && signal <= SIGRTMAX) {
    CHECK_GT(snprintf(buffer, sizeof(buffer), "Real-time signal %d",
                      signal - SIGRTMIN),
             0);
    return buffer;
  }

  if (signal > 0 && signal < NSIG && sys_siglist[signal] != nullptr) {
    return sys_siglist[signal];
  }

  CHECK_GT(snprintf(buffer, sizeof(buffer), "Unknown signal %d", signal), 0);
  return buffer;
}
