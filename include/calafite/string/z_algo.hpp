#pragma once
#include "../core/fvec.hpp"
#include <algorithm>

namespace calafite {

namespace string {
template <typename Container>
fvec<int> z_function(const Container &s) {
  int n = static_cast<int>(s.size());
  
  fvec<int> z(n, 0);
  if (n == 0) return z;

  const auto *data = s.data();
  for (int i = 1, l = 0, r = 0; i < n; i++) {
    if (i <= r) {
      z[i] = std::min(r - i + 1, z[i - l]);
    }
    while (i + z[i] < n && data[z[i]] == data[i + z[i]]) {
      z[i]++;
    }
    if (i + z[i] - 1 > r) {
      l = i;
      r = i + z[i] - 1;
    }
  }
  return z;
}
} // namespace string

struct ZF {
  fvec<int> z;
  int n;

  ZF() : n(0) {}

  template <typename Container>
  explicit ZF(const Container &s) {
    z = z_function(s);
    n = static_cast<int>(z.size());
  }

  inline const int& operator[](int i) const {
    return z[i];
  }

  inline int size() const {
    return n;
  }
};

template <typename Container1, typename Container2>
fvec<int> z_search(const Container1 &text, const Container2 &pattern) {
  fvec<int> res;
  int n = static_cast<int>(text.size());
  int m = static_cast<int>(pattern.size());
  if (m == 0 || n == 0 || m > n) return res;

  res.reserve(n / m + 2);
  
  fvec<int> z_pat = z_function(pattern);
  
  const auto *txt = text.data();
  const auto *pat = pattern.data();

  for (int i = 0, l = 0, r = -1; i < n; i++) {
    int cur_z = 0;
    
    if (i <= r) {
      cur_z = std::min(r - i + 1, z_pat[i - l]);
    }
    
    while (i + cur_z < n && cur_z < m && txt[i + cur_z] == pat[cur_z]) {
      cur_z++;
    }
    
    if (i + cur_z - 1 > r) {
      l = i;
      r = i + cur_z - 1;
    }
    
    if (cur_z == m) {
      res.push_back(i);
    }
  }
  
  return res;
}

} // namespace calafite
