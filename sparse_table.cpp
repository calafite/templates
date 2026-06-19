#pragma once
#include <bits/stdc++.h>
using namespace std;

template <typename T, typename F> struct SparseTable {
  vector<vector<T>> table;
  int length, max_log;
  F operation;

  SparseTable(const vector<T> &vec, F operation)
      : length(static_cast<int>(vec.size())),
        max_log(length > 0 ? __lg(length) : 0), operation(move(operation)) {
   if (length == 0)
      return;
    table.assign(max_log + 1, vector<T>(length));
    for (int j = 0; j < length; j++)
      table[0][j] = vec[j];
    for (int i = 1; i <= max_log; i++)
      for (int j = 0; j + (1 << i) <= length; j++)
        table[i][j] =
            this->operation(table[i - 1][j], table[i - 1][j + (1 << (i - 1))]);
  }

  T query(int left, int right) const {
    int i = __lg(right - left + 1);
    return operation(table[i][left], table[i][right - (1 << i) + 1]);
  }
};
