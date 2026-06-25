#pragma once
#include "../core/fvec.hpp"
#include <utility>

namespace calafite {

template <typename Container>
fvec<int> prefix_function(const Container &s) {
  int n = static_cast<int>(s.size());
  fvec<int> pi(n, 0);
  if (n == 0) return pi;

  const auto *data = s.data();
  for (int i = 1; i < n; i++) {
    int j = pi[i - 1];
    while (j > 0 && data[i] != data[j]) {
      j = pi[j - 1];
    }
    if (data[i] == data[j]) {
      j++;
    }
    pi[i] = j;
  }
  return pi;
}

template <typename T>
struct KMP {
  T pattern;
  fvec<int> pi;
  int m;
  
  int current_state = 0;

  KMP() : m(0) {}

  explicit KMP(T pat) 
      : pattern(std::move(pat)), m(static_cast<int>(pattern.size())) {
    pi = prefix_function(pattern);
  }

  template <typename U>
  fvec<int> search(const U &text) const {
    fvec<int> res;
    int n = static_cast<int>(text.size());
    if (m == 0 || n == 0 || m > n) return res;

    res.reserve(n / m + 2); 

    const auto *txt = text.data();
    const auto *pat = pattern.data();
    int j = 0;

    for (int i = 0; i < n; i++) {
      while (j > 0 && txt[i] != pat[j]) {
        j = pi[j - 1];
      }
      if (txt[i] == pat[j]) {
        j++;
      }
      if (j == m) {
        res.push_back(i - m + 1);
        j = pi[j - 1];
      }
    }
    return res;
  }

  template <typename U>
  int find_first(const U &text) const {
    int n = static_cast<int>(text.size());
    if (m == 0 || n == 0 || m > n) return -1;

    const auto *txt = text.data();
    const auto *pat = pattern.data();
    int j = 0;

    for (int i = 0; i < n; i++) {
      while (j > 0 && txt[i] != pat[j]) {
        j = pi[j - 1];
      }
      if (txt[i] == pat[j]) {
        j++;
      }
      if (j == m) {
        return i - m + 1;
      }
    }
    return -1;
  }

  void reset() {
    current_state = 0;
  }

  template <typename CharT>
  bool feed(const CharT &c) {
    if (m == 0) return false;
    const auto *pat = pattern.data();
    
    while (current_state > 0 && c != pat[current_state]) {
      current_state = pi[current_state - 1];
    }
    if (c == pat[current_state]) {
      current_state++;
    }
    if (current_state == m) {
      current_state = pi[current_state - 1];
      return true; 
    }
    return false;
  }
};

} // namespace calafite
