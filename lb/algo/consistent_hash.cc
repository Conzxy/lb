#include "consistent_hash.h"
#include "hash_util.h"

#include <iostream>

using namespace lb;

void ConsistentHash::AddServer(Node node, Key const &key, uint32_t vnode_num)
{
  /* Register the server node */
  auto node_hash_value = Hash(key, 0);
  node_map_[node_hash_value] = node;
  hash_ring_set_.insert(node_hash_value);

  /* Register the virtual nodes */
  uint64_t vnode_hash_value = 0;
  for (uint32_t i = 1; i <= vnode_num; ++i) {
    vnode_hash_value = Hash(key, i);
    hash_ring_set_.insert(vnode_hash_value);
    vnode_cache_map_[node_hash_value].push_back(vnode_hash_value);
    vnode_map_[vnode_hash_value] = node;
  }
}

void ConsistentHash::RemoveServer(Key const &key)
{
  auto hash_value = Hash(key, 0);
  hash_ring_set_.erase(hash_value);
  node_map_.erase(hash_value);

  auto vnodes = std::move(vnode_cache_map_[hash_value]);

  for (auto vnode_hash_value : vnodes) {
    hash_ring_set_.erase(vnode_hash_value);
    vnode_map_.erase(vnode_hash_value);
  }
  
  vnode_cache_map_.erase(hash_value);
}

auto ConsistentHash::QueryServer(Key const &key) -> Node
{
  auto hash_value = Hash(key, 0);

  auto closest_node_iter = hash_ring_set_.lower_bound(hash_value);
  HashValue node_hash_value = 0;
  if (closest_node_iter == hash_ring_set_.end()) {
    node_hash_value = *hash_ring_set_.begin();
  }

  node_hash_value = *closest_node_iter;

  /* Search if this is a virtual node */
  auto vnode_iter = vnode_map_.find(node_hash_value);
  if (vnode_iter != vnode_map_.end()) {
    return vnode_iter->second;
  }
  
  auto node_iter = node_map_.find(node_hash_value);
  if (node_iter != node_map_.end()) {
    return node_iter->second;
  }
  
  return -1;
}

std::string ConsistentHash::GetHashRing() const noexcept
{
  std::string ret;
  for (auto const hash_value : hash_ring_set_) {
    // ret += std::to_string(hash_value);
    auto node_iter = node_map_.find(hash_value);
    if (node_iter != node_map_.end()) {
      ret += "(Node: ";
      ret += std::to_string(node_iter->second);
      ret += ")";
    }

    auto vnode_iter = vnode_map_.find(hash_value);
    if (vnode_iter != vnode_map_.end()) {
      ret += "(VNode: ";
      ret += std::to_string(vnode_iter->second);
      ret += ")";
    }

    ret += "-> ";
  }

  return ret;
}
