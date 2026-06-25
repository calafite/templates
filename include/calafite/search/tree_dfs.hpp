#pragma once
#include "../core/fvec.hpp"
#include "../utils/fix_point.hpp"
#include <algorithm>

namespace calafite {

struct TreeDFS {
  fvec<int> parent;
  fvec<int> depth;
  fvec<int> sub_size;
  fvec<int> tin;
  fvec<int> tout;
  int timer;

  TreeDFS(int root, const fvec<fvec<int>> &adj) {
    int n = static_cast<int>(adj.size());
    parent.assign(n, -1);
    depth.assign(n, 0);
    sub_size.assign(n, 1);
    tin.assign(n, -1);
    tout.assign(n, -1);
    timer = 0;

    auto dfs = FixPoint([&](auto &self, int u, int p) -> void {
      parent[u] = p;
      tin[u] = timer++;

      for (int v : adj[u]) {
        if (v != p) {
          depth[v] = depth[u] + 1;
          self(v, u);
          sub_size[u] += sub_size[v];
        }
      }
      tout[u] = timer;
    });

    dfs(root, -1);
  }

  bool is_ancestor(int u, int v) const {
    return tin[u] <= tin[v] && tin[v] < tout[u];
  }
};

inline fvec<int> topo_sort(const fvec<fvec<int>> &adj) {
  int n = static_cast<int>(adj.size());
  fvec<int> order;
  order.reserve(n);

  fvec<int> state(n, 0);
  bool has_cycle = false;

  auto dfs = FixPoint([&](auto &self, int u) -> void {
    state[u] = 1;
    for (int v : adj[u]) {
      if (state[v] == 0) {
        self(v);
      } else if (state[v] == 1) {
        has_cycle = true;
      }
    }
    state[u] = 2;
    order.push_back(u);
  });

  for (int i = 0; i < n; ++i) {
    if (state[i] == 0 && !has_cycle) {
      dfs(i);
    }
  }

  if (has_cycle)
    return {};

  std::reverse(order.begin(), order.end());
  return order;
}
} // namespace calafite
