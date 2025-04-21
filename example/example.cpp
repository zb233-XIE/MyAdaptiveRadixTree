#include "art.hpp"
#include "art/art.hpp"
#include "art_printer.hpp"
#include <iostream>
#include <sstream>
#include <string>

int main() {
  art::AdaptiveRadixTree<float> tree;
  std::string key1{"abcas"};
  std::string key2{"ababdw"};

  art::RC ret = art::RC::INTERNAL_FAILURE;

  // Insert
  ret = tree.insert(key1.c_str(), 1.2);
  if (ret == art::RC::SUCCESS) {
    std::cout << "\nInsert Operation:\n";
    std::cout << "Successfully insert <" << key1 << ", " << 1.2 << ">"
              << std::endl;
  }
  ret = tree.insert(key2.c_str(), 2.1);
  if (ret == art::RC::SUCCESS) {
    std::cout << "\nInsert Operation:\n";
    std::cout << "Successfully insert <" << key2 << ", " << 2.1 << ">"
              << std::endl;
  }

  std::ostringstream oss;
  art::AdaptiveRadixTreePrinter<float> printer;
  printer.draw(&tree, oss);
  std::cout << "\nPrint Tree: \n" << oss.str() << std::endl;

  // Search
  float value;
  ret = tree.search(key1.c_str(), value);
  if (ret == art::RC::SUCCESS) {
    std::cout << "Search Operation:\n";
    std::cout << key1 << " -> " << value << std::endl;
  }

  // Delete
  ret = tree.remove(key1.c_str(), value);
  if (ret == art::RC::SUCCESS) {
    std::cout << "Remove Operation:\n";
    std::cout << "Successfully remove <" << key1 << ", " << value << ">"
              << std::endl;
  }

  return 0;
}