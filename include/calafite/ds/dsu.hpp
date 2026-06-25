#pragma once
#include "../core/fvec.hpp"
#include <algorithm>
#include <cassert>

namespace calafite {
struct DSU {
  int components;
  fvec<int> data;

  explicit DSU(int n) : components(n), data(n, -1) {}

  int find(int x) {
    assert(0 <= x && x < static_cast<int>(data.size()));
    return data[x] < 0 ? x : (data[x] = find(data[x]));
  }

  bool merge(int x, int y) {
    x = find(x);
    y = find(y);
    if (x == y) return false;
    if (data[x] > data[y]) {
      std::swap(x, y);
    }
    data[x] += data[y];
    data[y] = x;
    components--;
   
    return true;
  }

  bool same(int x, int y) {
    return find(x) == find(y);
  }

  int size(int x) {
    return -data[find(x)];
  }
};
} // namespace calafite
