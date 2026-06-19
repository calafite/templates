#pragma once
#include <bits/stdc++.h>

namespace calafite {
template <typename T, typename F> struct SparseTable {
  int n;
  std::vector<std::vector<T>> st;
  F combine;

  SparseTable(const std::vector<T> &a, F combine)
      : n(a.size()), combine(std::move(combine)) {
    int k = std::__lg(n) + 1;
    st.assign(k, std::vector<T>(n));
    st[0] = a;
    for (int i = 1; i < k; i++) {
      for (int j = 0; j + (1 << i) <= n; j++) {
        st[i][j] = this->combine(st[i - 1][j], st[i - 1][j + (1 << (i - 1))]);
      }
    }
  }

  T query(int l, int r) const {
    int i = std::__lg(r - l + 1);
    return combine(st[i][l], st[i][r - (1 << i) + 1]);
  }
};
} // namespace calafite
