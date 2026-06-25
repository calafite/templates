#pragma once
#include "../core/fvec.hpp"
#include <utility>

namespace calafite {
template <typename T, typename F> struct SegTreeIterative {
  int n;
  fvec<T> t;
  T neutral;
  F combine;

  SegTreeIterative(int n, T neutral, F combine)
      : n(n), t(2 * n, neutral), neutral(neutral), combine(std::move(combine)) {}

  SegTreeIterative(const fvec<T> &a, T neutral, F combine)
      : n((int)a.size()), t(2 * a.size(), neutral), neutral(neutral),
        combine(std::move(combine)) {
    for (int i = 0; i < n; i++)
      t[n + i] = a[i];
    for (int i = n - 1; i > 0; i--)
      t[i] = combine(t[i << 1], t[i << 1 | 1]);
  }

  void update(int p, T val) {
    for (t[p += n] = val; p >>= 1;) {
      t[p] = combine(t[p << 1], t[p << 1 | 1]);
    }
  }

  T query(int l, int r) const {
    T resL = neutral, resR = neutral;
    for (l += n, r += n + 1; l < r; l >>= 1, r >>= 1) {
      if (l & 1)
        resL = combine(resL, t[l++]);
      if (r & 1)
        resR = combine(t[--r], resR);
    }
    return combine(resL, resR);
  }
};
} // namespace calafite
