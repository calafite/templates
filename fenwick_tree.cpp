#pragma once
#include <bits/stdc++.h>
using namespace std;

template <typename T> struct FenwickTree {
  vector<T> bits;
  int sz, highbit;

  FenwickTree(int n) : sz(n), highbit(1), bits(n + 1, T(0)) {
    while (highbit < sz)
      highbit <<= 1;
  }

  FenwickTree(const vector<T> &a)
      : sz((int)a.size()), highbit(1), bits(a.size() + 1, T(0)) {
    for (int i = 1; i <= sz; i++) {
      bits[i] += a[i - 1];
      int j = i + (i & -i);
      if (j <= sz)
        bits[j] += bits[i];
    }
    while (highbit < sz)
      highbit <<= 1;
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
    int pos = 0;
    for (int pw = highbit; pw > 0; pw >>= 1) {
      if (pos + pw <= sz && bits[pos + pw] < k) {
        pos += pw;
        k -= bits[pos];
      }
    }
    return pos;
  }
};
