#ifndef LB_ALGO_CONSISTENT_HASH_H__
#define LB_ALGO_CONSISTENT_HASH_H__

#include <set>
#include <map>
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace lb {

class ConsistentHash {
  using HashValue = uint64_t;
  using VNodeHashValue = HashValue;
  using NodeHashValue = HashValue;
  using Node = uint32_t;
 public:
  using Key = std::string;
  ConsistentHash() = default;
  ~ConsistentHash() = default;
  
  /**
   * Add a server to the hash ring 
   * If specify the number of virtual nodes, also allocate them to the ring 
   * \param node The index of the node
   * \param key Used for hashing
   * \param vnode_num The number of virtual nodes
   */
  void AddServer(Node node, Key const &key, uint32_t vnode_num);
  
  /**
   * Remove a server from the hash ring, including
   * virtual nodes
   */
  void RemoveServer(Key const &key);
  
  /**
   * Query the target server of the key
   */
  Node QueryServer(Key const &key);
  
  // For debugging
  std::string GetHashRing() const noexcept;
 private:
  std::set<HashValue> hash_ring_set_;
  std::unordered_map<NodeHashValue, Node> node_map_;
  std::unordered_map<NodeHashValue, std::vector<VNodeHashValue>> vnode_cache_map_;
  std::unordered_map<VNodeHashValue, Node> vnode_map_;

};
}  // namespace lb

#endif  // LB_ALGO_CONSISTENT_HASH_H__
