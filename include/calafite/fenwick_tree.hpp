#pragma once
#include <bits/stdc++.h>

namespace calafite {
template <typename T> struct FenwickTree {
  int sz;
  std::vector<T> bits;

  FenwickTree(int n) : sz(n), bits(n + 1, T(0)) {}

  FenwickTree(const std::vector<T> &a)
      : sz((int)a.size()), bits(a.size() + 1, T(0)) {
    for (int i = 1; i <= sz; i++) {
      bits[i] += a[i - 1];
      int j = i + (i & -i);
      if (j <= sz)
        bits[j] += bits[i];
    }
  }

  T query(int i) const {
    T ans = T(0);
    for (i++; i > 0; i -= i & -i)
      ans += bits[i];
    return ans;
  }

  T query(int l, int r) const { return query(r) - query(l - 1); }

  void update(int i, T add) {
    for (i++; i <= sz; i += i & -i)
      bits[i] += add;
  }

  void set(int i, T val) { update(i, val - query(i, i)); }

  int kth(T k) const {
    if (sz == 0)
      return 0;
    int pos = 0;
    for (int pw = 1 << std::__lg(sz); pw > 0; pw >>= 1) {
      if (pos + pw <= sz && bits[pos + pw] < k) {
        pos += pw;
        k -= bits[pos];
      }
    }
    return pos;
  }
};
} // namespace calafite
