#pragma once

#include <chrono>
#include <cstdint>
#include <utility>
#include <functional>
#include "../core/fvec.hpp"

namespace calafite {

template <typename T> struct is_pair : std::false_type {};
template <typename T1, typename T2> struct is_pair<std::pair<T1, T2>> : std::true_type {};

template <typename T> struct CustomHash {
  static uint64_t splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    return x ^ (x >> 31);
  }

  size_t operator()(const T &x) const {
    static const uint64_t FIXED_RANDOM =
        std::chrono::steady_clock::now().time_since_epoch().count();
    
    if constexpr (is_pair<T>::value) {
      size_t seed = std::hash<typename T::first_type>{}(x.first);
      seed ^= std::hash<typename T::second_type>{}(x.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return splitmix64(seed + FIXED_RANDOM);
    } else {
      return splitmix64(std::hash<T>{}(x) + FIXED_RANDOM);
    }
  }
};

template <typename K, typename V, typename Hash = CustomHash<K>> struct hash_map {
  struct Entry {
    K key;
    V val;
    bool occupied = false;
  };

  int cap;
  fvec<Entry> table;
  int sz;
  Hash hasher;

  hash_map(int capacity = 1 << 4) {
    cap = 1;
    while (cap < capacity)
      cap <<= 1;
    table.resize(cap);
    sz = 0;
  }

  inline void check_expand() {
    if (sz * 2 >= cap) {
      int old_cap = cap;
      cap <<= 1;
      fvec<Entry> old_table = std::move(table);

      table.assign(cap, Entry());
      sz = 0;

      for (int i = 0; i < old_cap; i++) {
        if (old_table[i].occupied) {
          int pos = lookup(old_table[i].key);
          table[pos].occupied = true;
          table[pos].key = std::move(old_table[i].key);
          table[pos].val = std::move(old_table[i].val);
          sz++;
        }
      }
    }
  }

  inline int lookup(const K& key) const {
    int pos = hasher(key) & (cap - 1);
    while (table[pos].occupied && table[pos].key != key) {
      pos = (pos + 1) & (cap - 1);
    }
    return pos;
  }

  V &operator[](const K& key) {
    check_expand();
    int pos = lookup(key);
    if (!table[pos].occupied) {
      table[pos].occupied = true;
      table[pos].key = key;
      table[pos].val = V();
      sz++;
    }
    return table[pos].val;
  }

  void insert(K key, V val) {
    check_expand();
    int pos = lookup(key);
    if (!table[pos].occupied) {
      table[pos].occupied = true;
      table[pos].key = std::move(key);
      table[pos].val = std::move(val);
      sz++;
    } else {
      table[pos].val = std::move(val);
    }
  }

  bool count(const K& key) const {
    int pos = lookup(key);
    return table[pos].occupied;
  }

  void clear() {
    table.assign(cap, Entry());
    sz = 0;
  }
};
} // namespace calafite
