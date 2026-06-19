#pragma once
#include <bits/stdc++.h>
using namespace std;

template <typename T, typename F> struct SegTree {
  int n;
  vector<T> t;
  T neutral;
  mutable F combine;

  SegTree(int n, T neutral, F combine)
      : n(n), neutral(neutral), combine(move(combine)) {
    t.assign(4 * n, neutral);
  }

  SegTree(const vector<T> &a, T neutral, F combine)
      : n((int)a.size()), neutral(neutral), combine(move(combine)) {
    t.assign(4 * n, neutral);
    build(1, 0, n - 1, a);
  }

  void build(int pos, int tl, int tr, const vector<T> &a) {
    if (tl == tr) {
      t[pos] = a[tl];
      return;
    }
    int tm = tl + (tr - tl) / 2;
    build(2 * pos, tl, tm, a);
    build(2 * pos + 1, tm + 1, tr, a);
    t[pos] = combine(t[2 * pos], t[2 * pos + 1]);
  }

  void update(int i, T val, int pos, int tl, int tr) {
    if (tl == tr) {
      t[pos] = val;
      return;
    }
    int tm = tl + (tr - tl) / 2;
    if (i <= tm)
      update(i, val, 2 * pos, tl, tm);
    else
      update(i, val, 2 * pos + 1, tm + 1, tr);
    t[pos] = combine(t[2 * pos], t[2 * pos + 1]);
  }

  void update(int i, T val) { update(i, val, 1, 0, n - 1); }

  T query(int l, int r, int pos, int tl, int tr) const {
    if (r < tl || tr < l)
      return neutral;
    if (l <= tl && tr <= r)
      return t[pos];
    int tm = tl + (tr - tl) / 2;
    return combine(query(l, r, 2 * pos, tl, tm),
                   query(l, r, 2 * pos + 1, tm + 1, tr));
  }

  T query(int l, int r) const { return query(l, r, 1, 0, n - 1); }
};
