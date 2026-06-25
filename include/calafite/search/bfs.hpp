#pragma once
#include "../core/fvec.hpp"
#include <cassert>

namespace calafite {

inline fvec<int> bfs(int start, const fvec<fvec<int>> &adj) {
  int n = static_cast<int>(adj.size());
  assert(start >= 0 && start < n);

  fvec<int> dist(n, -1);
  fvec<int> q;
  q.reserve(n);

  dist[start] = 0;
  q.push_back(start);

  for (size_t head = 0; head < q.size(); ++head) {
    int u = q[head];
    for (int v : adj[u]) {
      if (dist[v] == -1) {
        dist[v] = dist[u] + 1;
        q.push_back(v);
      }
    }
  }

  return dist;
}

inline fvec<int> multi_bfs(const fvec<int> &starts,
                           const fvec<fvec<int>> &adj) {
  int n = static_cast<int>(adj.size());
  fvec<int> dist(n, -1);
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
        q.push_back(v);
      }
    }
  }

  return dist;
}

struct BFSResult {
  fvec<int> dist;
  fvec<int> parent;

  fvec<int> get_path(int target) const {
    if (dist[target] == -1)
      return {};
    fvec<int> path;
    for (int curr = target; curr != -1; curr = parent[curr]) {
      path.push_back(curr);
    }
    std::reverse(path.begin(), path.end());
    return path;
  }
};

inline BFSResult bfs_path(int start, const fvec<fvec<int>> &adj) {
  int n = static_cast<int>(adj.size());
  BFSResult res{fvec<int>(n, -1), fvec<int>(n, -1)};
  fvec<int> q;
  q.reserve(n);

  res.dist[start] = 0;
  q.push_back(start);

  for (size_t head = 0; head < q.size(); ++head) {
    int u = q[head];
    for (int v : adj[u]) {
      if (res.dist[v] == -1) {
        res.dist[v] = res.dist[u] + 1;
        res.parent[v] = u;
        q.push_back(v);
      }
    }
  }
  return res;
}
} // namespace calafite
