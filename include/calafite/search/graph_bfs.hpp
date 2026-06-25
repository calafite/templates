#pragma once
#include "../core/fvec.hpp"
#include <algorithm>
#include <cassert>

namespace calafite {

struct GraphBFS {
  fvec<int> dist;
  fvec<int> parent;

  BFS(int start, const fvec<fvec<int>> &adj, bool record_paths = false) {
    int n = static_cast<int>(adj.size());
    assert(start >= 0 && start < n);

    dist.assign(n, -1);
    if (record_paths) {
      parent.assign(n, -1);
    }

    fvec<int> q;
    q.reserve(n);

    dist[start] = 0;
    q.push_back(start);

    for (size_t head = 0; head < q.size(); ++head) {
      int u = q[head];
      for (int v : adj[u]) {
        if (dist[v] == -1) {
          dist[v] = dist[u] + 1;
          if (record_paths) {
            parent[v] = u;
          }
          q.push_back(v);
        }
      }
    }
  }

  BFS(const fvec<int> &starts, const fvec<fvec<int>> &adj, bool record_paths = false) {
    int n = static_cast<int>(adj.size());
    dist.assign(n, -1);
    if (record_paths) {
      parent.assign(n, -1);
    }

    fvec<int> q;
    q.reserve(n);

    for (int start : starts) {
      assert(start >= 0 && start < n);
      if (dist[start] == -1) {
        dist[start] = 0;
        q.push_back(start);
      }
    }

    for (size_t head = 0; head < q.size(); ++head) {
      int u = q[head];
      for (int v : adj[u]) {
        if (dist[v] == -1) {
          dist[v] = dist[u] + 1;
          if (record_paths) {
            parent[v] = u;
          }
          q.push_back(v);
        }
      }
    }
  }

  fvec<int> get_path(int target) const {
    assert(!parent.empty() && "Paths were not tracked. Initialize the BFS struct with record_paths = true.");
    assert(target >= 0 && target < static_cast<int>(dist.size()));

    if (dist[target] == -1) {
      return {};
    }

    fvec<int> path;
    for (int curr = target; curr != -1; curr = parent[curr]) {
      path.push_back(curr);
    }
    std::reverse(path.begin(), path.end());
    return path;
  }
};

} // namespace calafite
