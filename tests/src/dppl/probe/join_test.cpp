#include "dppl/probe/join.hpp"

#include <string>

#include "experimental/net"
#include "gtest/gtest.h"

bool hardware_test_check(void) {
  char* env = std::getenv("HARDWARE_TEST");
  return env != nullptr;
}

TEST(ProbeJoinTest, check) {
  if (!hardware_test_check()) return SUCCEED();

  std::experimental::net::io_context io_context;
  dppl::probe::join join_probe(&io_context);
  //bool attempting_join = join_probe.test(std::chrono::seconds(2));
  //ASSERT_EQ(attempting_join, false);

  join_probe.async_test([](bool result) { ASSERT_EQ(result, false); }, std::chrono::seconds(2));
  io_context.run();

  std::cout << "\n\nPlease attempt to join... then hit enter" << std::endl;
  std::string input;
  std::getline(std::cin, input, '\n');
  //attempting_join = join_probe.test(std::chrono::seconds(2));
  //ASSERT_EQ(attempting_join, true);

  join_probe.async_test([](bool result) { ASSERT_EQ(result, true); }, std::chrono::seconds(2));
  io_context.run();
}
