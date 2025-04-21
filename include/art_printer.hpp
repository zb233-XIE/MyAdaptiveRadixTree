#ifndef ART_PRINTER_HPP
#define ART_PRINTER_HPP

#include "art/art_inner_node.hpp"
#include "art/art_leaf_node.hpp"
#include <iomanip>
#include <ios>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <system_error>

namespace art {

template <class T> class AdaptiveRadixTree;
template <class T> class Node4;
template <class T> class Node16;
template <class T> class Node48;
template <class T> class Node256;

template <class T> class AdaptiveRadixTreePrinter {
public:
  /**
   * @brief do a dfs of the tree and print it
   * @param[out] os holds the printable format of the tree
   */
  void draw(const AdaptiveRadixTree<T> *tree, std::ostream &os) {
    if (tree == nullptr || tree->root_ == nullptr) {
      os << "Empty Tree\n";
      return;
    }
    printNode(os, tree->root_, 0);
  }

  void printKey(std::ostream &os, uint8_t key) {
    std::ostringstream tos;
    tos << "-[" << static_cast<int>(key);
    // for printable charaters
    if (key >= 32 && key <= 126) {
      tos << "(" << key << ")";
    }
    tos << "]";
    os << std::left << std::setw(10) << tos.str();
  }

  void printLeaf(std::ostream &os, const LeafNode<T> *node, int level) {
    os << "@LeafNode ";
    os << "<" << std::string(node->getPrefix()) << ", " << node->getValue()
       << ">\n";
  }

  void printNode4(std::ostream &os, const Node4<T> *node, int level) {
    os << "#Node4 {";
    if (node->getPrefixLen() > 0) {
      os << std::string(node->getPrefix());
    }
    os << "}\n";
    uint8_t sz = node->size_;
    for (int i = 0; i < sz; ++i) {
      for (int j = 0; j < level; ++j) {
        os << "  ";
      }
      int val = static_cast<int>(node->key_[i]);
      printKey(os, val);
      printNode(os, node->child_[i], level + 1);
    }
  }

  void printNode16(std::ostream &os, const Node16<T> *node, int level) {
    os << "$Node16 {";
    if (node->getPrefixLen() > 0) {
      os << std::string(node->getPrefix());
    }
    os << "}\n";
    uint8_t sz = node->size_;
    for (int i = 0; i < sz; ++i) {
      for (int j = 0; j < level; ++j) {
        os << "  ";
      }
      int val = static_cast<int>(node->key_[i]);
      printKey(os, val);
      printNode(os, node->child_[i], level + 1);
    }
  }

  void printNode48(std::ostream &os, const Node48<T> *node, int level) {
    os << "%Node48 {";
    if (node->getPrefixLen() > 0) {
      os << std::string(node->getPrefix());
    }
    os << "}\n";
    for (int i = 0; i < node->CIMAX; ++i) {
      int cindex = node->childIndex_[i];
      if (cindex != -1) {
        for (int j = 0; j < level; ++j) {
          os << "  ";
        }
        printKey(os, static_cast<uint8_t>(i));
        printNode(os, node->child_[cindex], level + 1);
      }
    }
  }

  void printNode256(std::ostream &os, const Node256<T> *node, int level) {
    os << "^Node256 {";
    if (node->getPrefixLen() > 0) {
      os << std::string(node->getPrefix());
    }
    os << "}\n";
    for (int i = 0; i < node->MAX; ++i) {
      if (node->child_[i] != nullptr) {
        for (int j = 0; j < level; ++j) {
          os << "  ";
        }
        printKey(os, static_cast<uint8_t>(i));
        printNode(os, node->child_[i], level + 1);
      }
    }
  }

  void printNode(std::ostream &os, const Node<T> *node, int level) {
    switch (node->type()) {
    case NodeType::LeafNode: {
      printLeaf(os, static_cast<const LeafNode<T> *>(node), level);
    } break;
    case NodeType::Node4: {
      printNode4(os, static_cast<const Node4<T> *>(node), level);
    } break;
    case NodeType::Node16: {
      printNode16(os, static_cast<const Node16<T> *>(node), level);
    } break;
    case NodeType::Node48: {
      printNode48(os, static_cast<const Node48<T> *>(node), level);
    } break;
    case NodeType::Node256: {
      printNode256(os, static_cast<const Node256<T> *>(node), level);
    } break;
    default:
      throw std::runtime_error("invalid node type");
    }
  }
};
} // namespace art

#endif