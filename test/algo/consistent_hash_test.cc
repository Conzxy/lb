#include "lb/algo/consistent_hash.h"

#include <gtest/gtest.h>
#include <iostream>

using namespace lb;

TEST(consistent_hash, add_server) {
  ConsistentHash ch;
  ch.AddServer(0, "S1", 2);
  ch.AddServer(1, "S2", 2);
  std::cout << ch.GetHashRing() << std::endl;
}

TEST(consistent_hash, remove_server) {
  ConsistentHash ch;
  std::string node = "S";
  for (int i = 0; i < 10; ++i) {
    ch.AddServer(i, "S" + std::to_string(i), 3);
  }

  std::cout << ch.GetHashRing() << "\n";

  ch.RemoveServer("S1");
  ch.RemoveServer("S2");

  std::cout << ch.GetHashRing() << "\n";
}
