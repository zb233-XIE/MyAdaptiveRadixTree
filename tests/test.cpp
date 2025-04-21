#include "art.hpp"
#include "art/art_inner_node.hpp"
#include "art/art_node.hpp"
#include "art_printer.hpp"
#include "gtest/gtest.h"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <unordered_map>

// TEST(NodeTest, DISABLED_GrowShrink) {
//   constexpr int KEYRANGE = 256;

//   std::pair<art::LeafNode<int> *, int> nls[KEYRANGE] = {};
//   int cnt = 0;
//   std::random_device rd;
//   std::mt19937 gen(rd());
//   std::uniform_int_distribution<uint8_t> kdis(33, 122);

//   // Initialization（start with a Node4)
//   char prefix[10] = "prefix";
//   int prefixLen = std::strlen(prefix);
//   art::Node<int> *nd = new art::Node4<int>{prefix};
//   EXPECT_TRUE(static_cast<art::InnerNode<int> *>(nd)->isLack());
//   EXPECT_FALSE(static_cast<art::InnerNode<int> *>(nd)->isFull());
//   EXPECT_EQ(nd->type(), art::NodeType::Node4);

//   int bound[5] = {0, 4, 16, 48};
//   // Insert test, grow the Node from art::Node4 to art::Node256
//   for (int i = 1; i < 4; i++) {
//     int MAX = bound[i];
//     // Insert until full
//     while (cnt < MAX) {
//       uint8_t key = kdis(gen);
//       while (nls[key].first != nullptr) {
//         key = kdis(gen);
//       }
//       int val = gen() % 100;
//       art::LeafNode<int> *leaf =
//           new art::LeafNode<int>{reinterpret_cast<char *>(&key), val};
//       EXPECT_EQ(leaf->type(), art::NodeType::LeafNode);
//       nls[key] = {leaf, val};
//       nd->addChild(key, leaf);
//       cnt++;
//     }
//     EXPECT_TRUE(nd->isFull());
//     // Grow, check prefix, type and all children
//     nd = nd->grow();
//     switch (i) {
//     case 1:
//       EXPECT_EQ(nd->type(), art::NodeType::Node16);
//       break;
//     case 2:
//       EXPECT_EQ(nd->type(), art::NodeType::Node48);
//       break;
//     case 3:
//       EXPECT_EQ(nd->type(), art::NodeType::Node256);
//       break;
//     default:;
//     }
//     // Check prefix
//     EXPECT_EQ(nd->checkPrefix(prefix, prefixLen, 0), prefixLen);
//     // Check children
//     for (int i = 0; i < KEYRANGE; ++i) {
//       auto &[cptr, val] = nls[i];
//       if (cptr != nullptr) {
//         uint8_t ckey = static_cast<uint8_t>(i);
//         EXPECT_EQ(cptr->checkPrefix(reinterpret_cast<char *>(&ckey), 1, 0), 1);
//         EXPECT_EQ(cptr->getValue(), val);
//       }
//     }
//   }

//   // Shrink test, shrink the Node from art::Node256 to art::Node4
//   for (int i = 3; i >= 0; i--) {
//     int MIN = bound[i];
//     while (cnt > MIN) {
//       uint8_t key = kdis(gen);
//       while (nls[key].first == nullptr || nls[key].second == -1) {
//         key = kdis(gen);
//       }
//       auto lptr = dynamic_cast<art::LeafNode<int> *>(nd->findChild(key));
//       ASSERT_TRUE(lptr != nullptr);
//       EXPECT_EQ(lptr->getValue(), nls[key].second);
//       nd->deleteChild(key);
//       nls[key].second = -1;
//       cnt--;
//     }
//     EXPECT_TRUE(nd->isLack());

//     if (i == 0) {
//       EXPECT_THROW(nd->shrink(), std::runtime_error);
//       break;
//     }
//     nd = nd->shrink();

//     switch (i) {
//     case 1:
//       EXPECT_EQ(nd->type(), art::NodeType::Node4);
//       break;
//     case 2:
//       EXPECT_EQ(nd->type(), art::NodeType::Node16);
//       break;
//     case 3:
//       EXPECT_EQ(nd->type(), art::NodeType::Node48);
//       break;
//     default:;
//     }

//     // Check prefix
//     EXPECT_EQ(nd->checkPrefix(prefix, prefixLen, 0), prefixLen);
//     // Check children
//     for (int i = 0; i < KEYRANGE; ++i) {
//       auto &[cptr, val] = nls[i];
//       if (cptr != nullptr && val != -1) {
//         uint8_t ckey = static_cast<uint8_t>(i);
//         EXPECT_EQ(cptr->checkPrefix(reinterpret_cast<char *>(&ckey), 1, 0), 1);
//         EXPECT_EQ(cptr->getValue(), val);
//       }
//     }
//   }

//   // finish up
//   for (auto &[ptr, _] : nls) {
//     if (ptr != nullptr) {
//       delete ptr;
//     }
//   }
//   delete nd;
// }

TEST(TreeTest, DISABLED_InsertTest) {
  art::AdaptiveRadixTree<int> tree;
  std::unordered_map<std::string, int> kvs;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(0, 100);

  std::ifstream infile{"/home/zb233/project/art/tests/words.txt"};
  std::string line;

  if (!infile) {
    std::cerr << "Failed to open file.\n";
  }

  while (std::getline(infile, line)) {
    line = line.substr(1, line.size() - 3);
    int v = dis(gen);
    kvs[line] = v;
    // std::cout << "Insert <" << line << ", " << v << std::endl;
    // tree.insert(line.c_str(), v);
  }
  std::cout << kvs.size() << '\n';

  std::unordered_map<std::string, int> kvs_;
  // auto start = std::chrono::high_resolution_clock::now();
  // for (auto &[k, v] : kvs) {
  //   kvs_[k] = v;
  // }
  // auto end = std::chrono::high_resolution_clock::now();
  // auto duration =
  // std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  // std::cout << "Map Time taken: " << duration.count() << " ms\n";

  auto start = std::chrono::high_resolution_clock::now();
  for (auto &[k, v] : kvs) {
    tree.insert(k.c_str(), v);
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  std::cout << "ART Time taken: " << duration.count() << " ms\n";

  art::RC ret;
  int value;
  for (auto &[k, v] : kvs) {
    ret = tree.search(k.c_str(), value);
    EXPECT_EQ(ret, art::RC::SUCCESS);
    EXPECT_EQ(value, v);
  }
  // std::ostringstream oss;
  // art::AdaptiveRadixTreePrinter<int> printer;
  // printer.draw(&tree, oss);
  // std::cout << oss.str() << std::endl;
  // std::cout << std::endl;
}

TEST(TreeTest, DeleteTest) {
  art::AdaptiveRadixTree<int> tree;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::unordered_map<std::string, int> kvs;
  std::vector<std::string> keys;
  
  std::ifstream infile{"./words.txt"};
  std::string line;

  if (!infile) {
    std::cerr << "Failed to open file.\n";
  }

  while (std::getline(infile, line)) {
    line = line.substr(1, line.size() - 3);
    int v = gen() % 100;
    kvs[line] = v;
    tree.insert(line.c_str(), v);
    keys.push_back(line);
  }

  int val = 0;
  art::RC ret = art::RC::INTERNAL_FAILURE;
  int threshold = 20;
  while (kvs.size() > threshold) {
    int index = gen() % keys.size();
    ret = tree.remove(keys[index].c_str(), val);
    auto iter = kvs.find(keys[index]);
    if (iter != kvs.end()) {
      EXPECT_EQ(ret, art::RC::SUCCESS);
      EXPECT_EQ(val, iter->second);
      kvs.erase(iter);
    } else {
      EXPECT_EQ(ret, art::RC::KEY_NOT_EXIST);
    }
  }

  for (auto &[k, v] : kvs) {
    ret = tree.search(k.c_str(), val);
    EXPECT_EQ(ret, art::RC::SUCCESS);
    EXPECT_EQ(v, val);
  }
}