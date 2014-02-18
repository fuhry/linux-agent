#include "dattod/signal_handler.h"

#include <chrono>
#include <thread>
#include <vector>

#include <sys/mman.h>

#include <gtest/gtest.h>
#include <glog/logging.h>

namespace {

using ::datto_linux_client::SignalHandler;

TEST(SignalHandlerTest, DefaultConstructor) {
  std::vector<int> v { };

  SignalHandler signal_handler(v);
}

TEST(SignalHandlerTest, DoesBlock) {
  int *did_return = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *did_return = 0;

  pid_t fork_ret = fork();
  if (fork_ret == -1) {
    // error
    FAIL() << "Fork failed";
  } else if (fork_ret == 0) {
    // child
    std::vector<int> v { SIGTERM };
    SignalHandler signal_handler(v);
    signal_handler.BlockSignals();
    // pause blocks until a signal handler is run
    pause();
    *did_return = 1;
    _exit(0);
  } else {
    // parent

    // using sleep is really lazy but it works
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(0, *did_return);
    kill(fork_ret, SIGTERM);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(1, *did_return);

    // Just in case :)
    kill(fork_ret, SIGKILL);
  }
}

// same idea as above but use WaitForSignal instead of pausing
TEST(SignalHandlerTest, DoesWait) {
  int *wait_finished = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  *wait_finished = 0;

  pid_t fork_ret = fork();
  if (fork_ret == -1) {
    // error
    FAIL() << "Fork failed";
  } else if (fork_ret == 0) {
    // child
    std::vector<int> v { SIGTERM };
    SignalHandler signal_handler(v);
    signal_handler.WaitForSignal([&](int) { *wait_finished = 1; });
    _exit(0);
  } else {
    // parent

    // using sleep is really lazy but it works
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(0, *wait_finished);
    kill(fork_ret, SIGTERM);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(1, *wait_finished);

    // Just in case :)
    kill(fork_ret, SIGKILL);
  }
}

} // namespace
